$ErrorActionPreference = 'Stop'
$source = Get-Content (Join-Path $PSScriptRoot 'composite.cpp') -Raw
function Extract([string]$name) {
    $match = [regex]::Match($source, "(?m)^[ \t]*(?:void|bool) $name\([^;]*?\) \{")
    if (!$match.Success) { throw "Missing function: $name" }
    $opening = $source.IndexOf('{', $match.Index)
    $depth = 1
    $end = $opening + 1
    while ($depth -and $end -lt $source.Length) {
        if ($source[$end] -eq '{') { $depth++ }
        if ($source[$end] -eq '}') { $depth-- }
        $end++
    }
    return $source.Substring($match.Index, $end - $match.Index)
}
$globals = $source.Substring($source.IndexOf('constexpr uint32_t sample_rate'),
    $source.IndexOf('void reply') - $source.IndexOf('constexpr uint32_t sample_rate'))
$mock = @'
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <string>
#include <vector>
#define CFG_TUD_AUDIO_FUNC_2_EP_IN_SW_BUF_SZ 384
uint32_t clock_ms = 0;
uint32_t millis() { return clock_ms; }
void yield() {}
void panic(const char *) { assert(false); }
struct {
  std::deque<char> input;
  std::string output;
  int space = 64;
  operator bool() { return true; }
  int availableForWrite() { return space; }
  int available() { return int(input.size()); }
  int read() { const char c = input.front(); input.pop_front(); return c; }
  size_t write(const uint8_t *data,size_t size) { output.append((const char *)data,size); return size; }
} Serial;
struct { int mutex; } USB;
struct CoreMutex {
  CoreMutex(int *,bool) {}
  operator bool() { return true; }
};
bool mounted = true, cdc_open = true, midi_writable = true;
std::string cdc_output;
std::vector<std::array<uint8_t,4>> packets;
bool tud_mounted() { return mounted; }
bool tud_cdc_n_connected(int) { return cdc_open; }
unsigned tud_cdc_n_available(int) { return 0; }
unsigned tud_cdc_n_write_available(int) { return 8; }
int tud_cdc_n_read_char(int) { return -1; }
size_t tud_cdc_n_write(int,const void *data,size_t size) { cdc_output.append((const char *)data,size); return size; }
void tud_cdc_n_write_flush(int) {}
unsigned tud_midi_available() { return 0; }
bool tud_midi_packet_read(uint8_t *) { return false; }
bool tud_midi_packet_write(const uint8_t *p) {
  if (!midi_writable) return false;
  packets.push_back({p[0],p[1],p[2],p[3]}); return true;
}
unsigned tu_fifo_count(int *) { return 0; }
int *tud_audio_n_get_ep_in_ff(int) { static int fifo; return &fifo; }
std::vector<int16_t> pcm;
uint16_t tud_audio_n_write(int,const void *data,uint16_t size) {
  const auto *samples = static_cast<const int16_t *>(data);
  pcm.assign(samples,samples+size/2); return size;
}
'@
$checks = @'
void drain() { for (int i=0;i<100 && response[0];++i) service_cli(); assert(!response[0]); }
void input(const char *line) {
  drain(); for (const char *p=line;*p;++p) Serial.input.push_back(*p);
  while (!Serial.input.empty()) { service_cli(); drain(); }
}
int main() {
  ready = true;
  input("\\?\r\n");
  for (auto text : {"\\text","\\note","\\cc","\\tone","\\stop","\\status","PC input monitoring"})
    assert(Serial.output.find(text) != std::string::npos);
  input("\\bogus\n");
  assert(Serial.output.find("unknown command") != std::string::npos);
  input("\\text\n");
  for (int i=0;i<10;++i) composite_service();
  assert(cdc_output == sample_text && cdc_bytes == cdc_output.size());
  drain();
  input("\\note\n");
  composite_service();
  assert((packets.back() == std::array<uint8_t,4>{9,0x90,60,100}));
  clock_ms += 499; composite_service(); assert(packets.size() == 1);
  midi_writable = false;
  ++clock_ms; composite_service(); assert(packets.size() == 1);
  clock_ms += 6000; composite_service();
  drain(); assert(Serial.output.find("Note Off blocked") != std::string::npos);
  midi_writable = true; composite_service();
  assert((packets.back() == std::array<uint8_t,4>{8,0x80,60,0}));
  input("\\cc\n"); composite_service();
  assert((packets.back() == std::array<uint8_t,4>{11,0xB0,21,64}));
  input("\\tone\n");
  tud_audio_tx_done_pre_load_cb(0,1,0,1);
  assert(pcm.size() == 96 && pcm[0] == 4096 && pcm[24] == -4096);
  clock_ms += 2999; service_cli(); assert(tone_enabled);
  ++clock_ms; service_cli(); assert(!tone_enabled);
  tud_audio_tx_done_pre_load_cb(0,1,0,1);
  for (auto sample : pcm) assert(sample == 0);
  input("\\tone\n"); input("\\stop\n"); assert(!tone_enabled);
  cdc_open = false;
  input("\\text\n"); clock_ms += 5000; composite_service(); drain();
  assert(Serial.output.find("CDC2 text test timed out") != std::string::npos);
  midi_writable = false;
  input("\\cc\n"); clock_ms += 5000; composite_service(); drain();
  assert(Serial.output.find("MIDI test timed out") != std::string::npos);
  input("\\note\n"); mounted = false; composite_service(); drain();
  assert(midi_test == MidiTest::Idle && Serial.output.find("USB disconnected") != std::string::npos);
  input("\\this-command-is-much-too-long-to-fit-in-the-buffer\n");
  assert(Serial.output.find("command too long") != std::string::npos);
  puts("PASS: help/parser, CDC text, exact Note/CC packets, backpressure, timeout, disconnect, timed tone PCM");
}
'@
$temp = Join-Path ([IO.Path]::GetTempPath()) ("composite-cli-" + [guid]::NewGuid())
$null = New-Item -ItemType Directory -Path $temp
try {
    $cpp = Join-Path $temp 'test.cpp'
    $exe = Join-Path $temp 'test.exe'
    $functions = @('reply','status','command','service_cli','composite_service',
        'tud_audio_tx_done_pre_load_cb') | ForEach-Object { Extract $_ }
    Set-Content -LiteralPath $cpp -Encoding ASCII -Value ($mock + $globals +
        ($functions -join "`n") + $checks)
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual Studio C++ tools required' }
    $vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
    & $env:ComSpec /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /std:c++17 /EHsc /D_CRT_SECURE_NO_WARNINGS /Fe`"$exe`" `"$cpp`" && `"$exe`""
    if ($LASTEXITCODE -ne 0) { throw 'CLI regression failed' }
} finally {
    Remove-Item -LiteralPath $temp -Recurse -Force
}
