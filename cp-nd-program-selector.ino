// Circuit Playground/Nord Drum Program Selector
// By Tom Hoffman, copyright 2024.
// GPL 3.0 License.

// This allows you to select programs on a Nord Drum (1) synthesizer,
// using a single Adafruit Circuit Playground connected by USB MIDI.
// The program selections are based on the factory presets, 
// with slight reorganizations so each grouping is 10 or less
// (as there are 10 LEDs on the Circuit Playground).
// The code could be modified to interact with different synths
// or microcontrollers.

#include <Adafruit_CircuitPlayground.h>
#include <USB-MIDI.h>

USBMIDI_CREATE_DEFAULT_INSTANCE();
using namespace MIDI_NAMESPACE;

// CONSTANTS

// NOTE!!!!!
// USB-MIDI channels go from 1-16 (as usually displayed to user)
// NOT 0-15 (as usually represented in code).
const uint8_t   nord_drum_channel = 1;
// need to set a channel to listen for clock
const uint8_t   selector_channel  = 15;
const uint8_t   LED_COUNT         = 10;
// Programs are organized by style and category.
const uint8_t   STYLES_COUNT      = 5;
const char      STYLES[5][8] = {"retro", "world", "real", "rock", "fx"};
const uint8_t   CATEGORIES_COUNT  = 3;
// "kit" is bass, snare and some combination of hi-hat, cymbal, percussion and tom-tom.
// these are best for general rock/pop/hip-hop beats.
// "perc" are non-drum kit percussion programs (no bass/snare).
const char      CATEGORIES[3][16] = {"kit", "perc", "drums"};
// "drums" are bass, snare, and two tom-toms (the non-cymbal parts of a drum kit).

uint8_t current_programs[LED_COUNT]; // keep the LED UI straightforward.
uint8_t current_program_count;
uint8_t selection_index         = 0;
uint8_t current_style           = 0;
uint8_t current_category        = 0;
bool    a_button_down           = false;
bool    b_button_down           = false;
bool    switch_is_right;

// This is the factory soundback, categories slightly rearranged so
// none are larger than 10, and "ethno" is a little more correct.
const char SOUNDBANK[99][3][32] = {{"retro", "Monologue", "drums"},
{"rock", "Classic Vistalite", "drums"},
{"retro", "Blue House", "kit"},
{"real", "Brushford", "kit"},
{"rock", "Bebox Delux", "drums"},
{"world", "Always Hip Hop", "kit"},
{"real", "Gran Casa Timp", "kit"},
{"retro", "Thanx to Burgees", "drums"},
{"fx", "Reso Sweep", "perc"},
{"retro", "Vince Gate", "drums"},
{"world", "UnoDosKickHat", "kit"},
{"real", "spectrum", "drums"},
{"rock", "Ateiste", "drums"},
{"retro", "Noisy Barrel Orchestra", "drums"},
{"retro", "Higgins Particle Hat", "kit"},
{"rock", "Clothed Funk Kit", "kit"},
{"world", "Komal Melodic", "perc"},
{"world", "Lalalatin", "perc"},
{"retro", "Bend Down Disco", "perc"},
{"rock", "Flying Dront Circus", "kit"},
{"world", "Tribunal", "perc"},
{"real", "King Kong Karma", "kit"},
{"fx", "Training with Kolal", "perc"},
{"real", "Tiny Tiny Pic", "kit"},
{"world", "Red Beat", "perc"},
{"real", "beatPerlife", "kit"},
{"retro", "Piccolosim", "perc"},
{"real", "Acoustic Flower King", "kit"},
{"rock", "Apostasy Steam Noise", "perc"},
{"fx", "DoReMinor Melodic", "perc"},
{"fx", "Must Bend Tolotto", "drums"},
{"world", "Sambalasala", "perc"},
{"fx", "Kiss the Click", "perc"},
{"retro", "Sweep Type 4tonight", "drums"},
{"fx", "Noise Click Trap", "kit"},
{"real", "Bend Timpanic", "drums"},
{"retro", "dododrum", "kit"},
{"fx", "Fast Sweep Melodic", "drums"},
{"world", "Bella Balinese", "perc"},
{"fx", "Noisy Royalty", "drums"},
{"world", "Steely Melodic", "drums"},
{"rock", "Soft Acoustic", "drums"},
{"real", "Tubular Bells and Triangle", "perc"},
{"world", "Ultra Tribal Dance", "perc"},
{"retro", "HeaHihat", "kit"},
{"fx", "Click Gate and Vinyl", "kit"},
{"real", "Double Snare", "kit"},
{"real", "Hells Bells", "perc"},
{"real", "Rototsthile", "drums"},
{"retro", "Melodic Technocrat", "kit"},
{"rock", "Dull Dusty", "drums"},
{"fx", "Retrophile Gated Noise", "drums"},
{"world", "Real Cuba Conga Cola", "perc"},
{"retro", "Retrograd", "drums"},
{"fx", "GasaGate", "perc"},
{"real", "Clap Trap", "drums"},
{"real", "Krimsonite", "drums"},
{"real", "Serious Decay", "kit"},
{"world", "Dry Tribe", "drums"},
{"fx", "Cinematikino", "perc"},
{"fx", "Toy Ambulance", "kit"},
{"retro", "Neophile", "drums"},
{"retro", "Stabby Hip Hop", "kit"},
{"real", "Retro Noise Reverb", "drums"},
{"retro", "Ulam Spiral", "drums"},
{"retro", "Sawkas Jungle Heat", "kit"},
{"retro", "Knick Knock Knack", "drums"},
{"real", "Bright Click Brush", "drums"},
{"retro", "New Romantic Tight", "kit"},
{"fx", "Intergalactic Battle", "perc"},
{"retro", "Nosampled Drum", "drums"},
{"retro", "Poor Tone", "kit"},
{"fx", "Clicks&Pops", "drums"},
{"fx", "Fat Gated Chattanoga", "drums"},
{"real", "Retro Real Snap Snare", "drums"},
{"world", "Slitz Box", "perc"},
{"world", "Real Tommy Steel", "perc"},
{"fx", "Macro Sweeper", "kit"},
{"fx", "Darwin's Sex Machine", "kit"},
{"real", "Apparatorium", "drums"},
{"", "Default", ""}};       

void updateCurrentPrograms(uint8_t s, uint8_t c) {
  // reset all to 255 (no program)
  for (int i = 0; i < LED_COUNT; i++) {
    current_programs[i] = 255;
  }
  current_program_count = 0;
  char style[8];
  strcpy(style, STYLES[s]);
  char category[16];
  strcpy(category, CATEGORIES[c]);
  for (int i = 0; i < 100; i++) {
    if (!strcmp(SOUNDBANK[i][0], style) && !strcmp(SOUNDBANK[i][2], category)) {
      current_programs[current_program_count] = i;
      Serial.print(i+1);
      Serial.println(SOUNDBANK[i][1]);
      current_program_count++;
    }
  }
}

void sendPC() {
  MIDI.sendProgramChange(current_programs[selection_index], nord_drum_channel);
}

bool switchRight() { 
  // individual program selection
  return !(CircuitPlayground.slideSwitch());
}

bool switchLeft() {
  // chosing style and category
  return CircuitPlayground.slideSwitch();
}

bool switchChanged() {
  return (switch_is_right != switchRight()); 
}

void updateProgramSelectionDisplay() {
  for (int i = 0; i < (LED_COUNT); i++) {
    if (i < current_program_count) {
      CircuitPlayground.setPixelColor(i, 255, 255, 0);
    }
    else {
      CircuitPlayground.setPixelColor(i, 0, 0, 0);
    }
  }
  CircuitPlayground.setPixelColor(current_program_count - selection_index - 1, 204, 0, 204);
}

void updateStyleDisplay() {
  for (int i = 0; i < 5; i++) {
    if (i >= (4 - STYLES_COUNT)) {
      CircuitPlayground.setPixelColor(i, 0, 0, 255);
    }
    else {
      CircuitPlayground.setPixelColor(i, 0, 0, 0);
    }
  }
  CircuitPlayground.setPixelColor(4 - current_style, 255, 0, 255);
}

void updateCategoryDisplay() {
  for (int i = 5; i < 10; i++) {
    if (i <= 4 + CATEGORIES_COUNT) {
      CircuitPlayground.setPixelColor(i, 0, 255, 0);
    }
    else {
      CircuitPlayground.setPixelColor(i, 0, 0, 0);
    }
  }
  CircuitPlayground.setPixelColor(5 + current_category, 0, 255, 255);
}

void updateNeoPixels() {
  switch_is_right = switchRight();
  if (switch_is_right) {
    updateProgramSelectionDisplay();
  }
  else {
    updateStyleCategoryDisplay();
  }
}

void updateStyleCategoryDisplay() {
  updateStyleDisplay();
  updateCategoryDisplay();
}

void processLeftButton() {
  if (switch_is_right) { // program mode
    if (selection_index == 0) {
      selection_index = current_program_count - 1;
    }
    else {
      selection_index = selection_index - 1;
    }
  }
  else {
    current_style = (++current_style % STYLES_COUNT);
    updateCurrentPrograms(current_style, current_category);
    selection_index = 0;
  }
  sendPC();
  updateNeoPixels();
}

void processRightButton() {
  if (switch_is_right) { // program mode
    selection_index = ++selection_index % current_program_count;
  }
  else {
    current_category = (++current_category % CATEGORIES_COUNT);
    updateCurrentPrograms(current_style, current_category);
    selection_index = 0;
  }
  sendPC();
  updateNeoPixels();
}

void setup() {
  Serial.begin(31250);
  delay(1000);

  MIDI.begin(selector_channel);
  // belt and suspenders reset
  MIDI.setHandleStart(sendPC);
  MIDI.setHandleStop(sendPC);
  CircuitPlayground.begin(1);
  //Serial.println("Starting...");
  updateCurrentPrograms(current_style, current_category);
  updateNeoPixels();
  sendPC();  // this probably doesn't register on hot-plug
  Serial.println(current_program_count);
}

void loop() {
  MIDI.read();
  if (switchChanged()) {
    updateNeoPixels();
  }
  if (!a_button_down) {
    if (CircuitPlayground.leftButton()) { // and the button is being pressed
      processLeftButton();              // process new press
      a_button_down = true;               // note ongoing press
    }
  }
  else {                                             // if it was previously pressed
    a_button_down = CircuitPlayground.leftButton();  // store the current state
  }
  if (!b_button_down) {
    if (CircuitPlayground.rightButton()) {
      processRightButton();
      b_button_down = true;
    }
  }
  else {                                             // if it was previously pressed
    b_button_down = CircuitPlayground.rightButton();  // store the current state
  }
}
