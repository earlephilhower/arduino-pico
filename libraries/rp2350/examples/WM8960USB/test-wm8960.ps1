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
#include <cassert>
#include <cstdint>
#include <cstring>
#include <deque>
#include <vector>
#define CFG_TUD_AUDIO_FUNC_2_EP_IN_SW_BUF_SZ 384
struct repeating_timer {};
struct spin_lock_t {};
uint32_t spin_lock_blocking(spin_lock_t *) { return 0; }
void spin_unlock(spin_lock_t *,uint32_t) {}
bool mutex_available = true;
struct { int mutex; } USB;
struct CoreMutex {
  CoreMutex(int *,bool) {}
  operator bool() { return mutex_available; }
};
std::deque<int16_t> usb_pcm, adc_pcm;
std::vector<int16_t> dac_output, capture_output;
unsigned fifo_count = 0, dma_lead = 208, write_frames = 48;
uint32_t feedback_value = 0;
unsigned tud_audio_n_available(int) { return unsigned(usb_pcm.size())*2; }
uint16_t tud_audio_n_read(int,void *buffer,unsigned bytes) {
  unsigned count = std::min(unsigned(usb_pcm.size()), bytes/2);
  auto *out = static_cast<int16_t *>(buffer);
  for (unsigned i=0;i<count;++i) { out[i]=usb_pcm.front(); usb_pcm.pop_front(); }
  return uint16_t(count*2);
}
bool tud_audio_n_fb_set(int,uint32_t value) { feedback_value=value; return true; }
int *tud_audio_n_get_ep_in_ff(int) { static int fifo; return &fifo; }
unsigned tu_fifo_count(int *) { return fifo_count; }
uint16_t tud_audio_n_write(int,const void *buffer,uint16_t bytes) {
  auto *in = static_cast<const int16_t *>(buffer);
  capture_output.assign(in,in+bytes/2); return bytes;
}
namespace wm8960_i2s {
int available_for_write() { return write_frames*4; }
unsigned queued_frames() { return dma_lead; }
size_t write(const uint8_t *buffer,size_t bytes) {
  const auto *samples=reinterpret_cast<const int16_t *>(buffer);
  dac_output.assign(samples,samples+bytes/2); return bytes;
}
int available() { return int(adc_pcm.size())*4; }
size_t read(uint8_t *buffer,size_t bytes) {
  auto *samples=reinterpret_cast<int16_t *>(buffer);
  unsigned frames=std::min(unsigned(adc_pcm.size()),unsigned(bytes/4));
  for (unsigned i=0;i<frames;++i) {
    samples[i*2]=samples[i*2+1]=adc_pcm.front(); adc_pcm.pop_front();
  }
  return frames*4;
}
}
'@
$checks = @'
int main() {
  for (int packet=0;packet<30;++packet) {
    for (int i=0;i<48;++i) usb_pcm.push_back(1000);
    tud_audio_rx_done_post_read_cb(0,96,0,0,1);
    assert(feedback_value >= (48u<<16)-16384 && feedback_value <= (48u<<16)+16384);
    assert(playback_head-playback_tail <= pcm_frames);
  }
  assert(playback_primed);
  assert(dac_output.size() == 96 && dac_output[0] == 1000 && dac_output[1] == 1000);
  gain[0]=0; render_audio(); for (auto s:dac_output) assert(s==0);
  tone_enabled=true;
  for (int i=0;i<6;++i) render_audio();
  assert(dac_output[0]==0 && dac_output[24]==1024 && dac_output[72]==-1024);
  tone_enabled=false;
  for (int i=0;i<6;++i) render_audio();
  assert(local_envelope==0); for (auto s:dac_output) assert(s==0);
  gain[0]=32767;
  playback_head=playback_tail=0; playback_primed=false; level_q8=target_frames*256;
  write_frames=0; dma_lead=0;
  tud_audio_rx_done_post_read_cb(0,0,0,0,1); assert(feedback_value>(48u<<16));
  playback_head=900; playback_tail=0; dma_lead=256; level_q8=target_frames*256;
  tud_audio_rx_done_post_read_cb(0,0,0,0,1); assert(feedback_value<(48u<<16));
  playback_head=pcm_frames;
  for (int i=0;i<49;++i) usb_pcm.push_back(1);
  capture_playback(); assert(pcm_drops==49);
  const auto runs=timer_runs;
  mutex_available=false; audio_timer(nullptr);
  assert(timer_runs==runs+1); // USB mutex must never gate DMA rendering.
  mutex_available=true; audio_timer(nullptr); assert(timer_runs==runs+2);
  for (int i=0;i<400;++i) adc_pcm.push_back(2000);
  tud_audio_tx_done_pre_load_cb(0,1,0,1);
  assert(capture_output.size()==98 && capture_output[0]==2000);
  adc_pcm.clear();
  tud_audio_tx_done_pre_load_cb(0,1,0,1);
  assert(capture_starves==1 && !capture_primed);
  for (auto s:capture_output) assert(s==0);
  tone_enabled=true; phase=0; gain[1]=0;
  tud_audio_tx_done_pre_load_cb(0,1,0,1);
  for (auto s:capture_output) assert(s==0);
  gain[1]=32767;
  for (int i=0;i<6;++i) tud_audio_tx_done_pre_load_cb(0,1,0,1);
  phase=0; tud_audio_tx_done_pre_load_cb(0,1,0,1);
  assert(capture_output[0]==0 && capture_output[12]==1024 && capture_output[36]==-1024);
  puts("PASS: feedback bounds, DMA render independent of USB mutex, quiet sine/fades, overflow and ADC starvation");
}
'@
$temp = Join-Path ([IO.Path]::GetTempPath()) ("wm8960-sample-" + [guid]::NewGuid())
$null = New-Item -ItemType Directory -Path $temp
try {
    $cpp = Join-Path $temp 'test.cpp'
    $exe = Join-Path $temp 'test.exe'
    $functions = @('capture_playback','render_audio','audio_timer','tud_audio_rx_done_post_read_cb',
        'tud_audio_tx_done_pre_load_cb') | ForEach-Object { Extract $_ }
    Set-Content -LiteralPath $cpp -Encoding ASCII -Value ($mock + $globals +
        ($functions -join "`n") + $checks)
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual Studio C++ tools required' }
    $vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
    & $env:ComSpec /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /std:c++17 /EHsc /Fe`"$exe`" `"$cpp`" && `"$exe`""
    if ($LASTEXITCODE -ne 0) { throw 'WM8960 sample regression failed' }
} finally {
    Remove-Item -LiteralPath $temp -Recurse -Force
}
