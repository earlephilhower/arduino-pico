// The speaker will play the tune to Happy Birthday continuously
// Author: Tony DiCola
// License: MIT (https://opensource.org/licenses/MIT)

#include <Arduino.h>

#ifdef USE_TINYUSB
// For Serial when selecting TinyUSB.  Can't include in the core because Arduino IDE
// will not link in libraries called from the core.  Instead, add the header to all
// the standard libraries in the hope it will still catch some user cases where they
// use these libraries.
// See https://github.com/earlephilhower/arduino-pico/issues/167#issuecomment-848622174
#include <Adafruit_TinyUSB.h>
#endif

// pin_buzzer should be defined by the supported variant e.g CPlay Bluefruit or CLUE.
// Otherwise please define the pin you would like to use for tone output
#ifndef PIN_BUZZER
#define PIN_BUZZER    A0
#endif

uint8_t const pin_buzzer = PIN_BUZZER;

// A few music note frequencies as defined in this tone example:
//   https://www.arduino.cc/en/Tutorial/toneMelody
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988

// Define note durations.  You only need to adjust the whole note
// time and other notes will be subdivided from it directly.
#define WHOLE         2200       // Length of time in milliseconds of a whole note (i.e. a full bar).
#define HALF          WHOLE/2
#define QUARTER       HALF/2
#define EIGHTH        QUARTER/2
#define EIGHTH_TRIPLE QUARTER/3
#define SIXTEENTH     EIGHTH/2

// Play a note of the specified frequency and for the specified duration.
// Hold is an optional bool that specifies if this note should be held a
// little longer, i.e. for eighth notes that are tied together.
// While waiting for a note to play the waitBreath delay function is used
// so breath detection and pixel animation continues to run.  No tones
// will play if the slide switch is in the -/off position or all the
// candles have been blown out.
void playNote(int frequency, int duration, bool hold = false, bool measure = true) {
  (void) measure;

  if (hold) {
    // For a note that's held play it a little longer than the specified duration
    // so it blends into the next tone (but there's still a small delay to
    // hear the next note).
    tone(pin_buzzer, frequency, duration + duration / 32);
  } else {
    // For a note that isn't held just play it for the specified duration.
    tone(pin_buzzer, frequency, duration);
  }

  delay(duration + duration / 16);
}

// Song to play when the candles are blown out.
void celebrateSong() {
  // Play a little charge melody, from:
  //  https://en.wikipedia.org/wiki/Charge_(fanfare)
  // Note the explicit boolean parameters in particular the measure=false
  // at the end.  This means the notes will play without any breath measurement
  // logic.  Without this false value playNote will try to keep waiting for candles
  // to blow out during the celebration song!
  playNote(NOTE_G4, EIGHTH_TRIPLE, true, false);
  playNote(NOTE_C5, EIGHTH_TRIPLE, true, false);
  playNote(NOTE_E5, EIGHTH_TRIPLE, false, false);
  playNote(NOTE_G5, EIGHTH, true, false);
  playNote(NOTE_E5, SIXTEENTH, false);
  playNote(NOTE_G5, HALF, false);
}

void setup() {
  // Initialize serial output and Circuit Playground library.
  Serial.begin(115200);

  pinMode(pin_buzzer, OUTPUT);
  digitalWrite(pin_buzzer, LOW);
}

void loop() {
  // Play happy birthday tune, from:
  //  http://www.irish-folk-songs.com/happy-birthday-tin-whistle-sheet-music.html#.WXFJMtPytBw
  // Inside each playNote call it will play a note and drive the NeoPixel animation
  // and check for a breath against the sound sensor.  Once all the candles are blown out
  // the playNote calls will stop playing music.
  playNote(NOTE_D4, EIGHTH, true);
  playNote(NOTE_D4, EIGHTH);
  playNote(NOTE_E4, QUARTER);       // Bar 1
  playNote(NOTE_D4, QUARTER);
  playNote(NOTE_G4, QUARTER);
  playNote(NOTE_FS4, HALF);         // Bar 2
  playNote(NOTE_D4, EIGHTH, true);
  playNote(NOTE_D4, EIGHTH);
  playNote(NOTE_E4, QUARTER);       // Bar 3
  playNote(NOTE_D4, QUARTER);
  playNote(NOTE_A4, QUARTER);
  playNote(NOTE_G4, HALF);          // Bar 4
  playNote(NOTE_D4, EIGHTH, true);
  playNote(NOTE_D4, EIGHTH);
  playNote(NOTE_D5, QUARTER);       // Bar 5
  playNote(NOTE_B4, QUARTER);
  playNote(NOTE_G4, QUARTER);
  playNote(NOTE_FS4, QUARTER);      // Bar 6
  playNote(NOTE_E4, QUARTER);
  playNote(NOTE_C5, EIGHTH, true);
  playNote(NOTE_C5, EIGHTH);
  playNote(NOTE_B4, QUARTER);       // Bar 7
  playNote(NOTE_G4, QUARTER);
  playNote(NOTE_A4, QUARTER);
  playNote(NOTE_G4, HALF);          // Bar 8

  celebrateSong();

  // One second pause before repeating the loop and playing
  delay(1000);
}
