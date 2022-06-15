#include <FastLED.h>
#define NUM_LEDS 208
#define DATA_PIN 0
#define LENGTH 13
#define NUM_BOARDS 9
#define NUM_SPOTS 9
#define BRIGHTNESS 10

CRGB leds[NUM_LEDS];

/* light led at array index */
void light_led(uint16_t array_index, CRGB color) {
  leds[array_index] = color;
}

/* get the array index for the given cordinates (i,j), i = row, j = column */
uint16_t index(uint16_t i, uint16_t j) {
  return j & 0x01 ? j * 16 + 15 - i : j * 16 + i;
}

/* get the array index for the given spot on the given board */
uint16_t index_board(uint16_t board, uint16_t spot) {
  /* translate to index */
  uint16_t i = board - (board % 3) + spot / 3;
  uint16_t j = (board % 3) * 3 + (spot % 3);

  /* skip game boarders */
  i += i / 3 + 1;
  j += j / 3 + 1;

  return index(i, j);
}


/* light led at (i,j) coordinates, i = row, j = column */
void light_led(uint16_t i, uint16_t j, CRGB color) {
  light_led(index(i, j), color);
}

/* light led on game board, board = 0-8, spot = 0-8 */
void light_led_board(uint16_t board, uint16_t spot, CRGB color) {
  light_led(index_board(board, spot), color);
}

/* fill an entire board with the given color */
void fill_board(uint16_t board, CRGB color) {
  for (uint16_t spot = 0; spot < NUM_SPOTS; spot++) {
    light_led_board(board, spot, color);
  }
}

/* fill an entire row with the given color */
void fill_row(uint16_t row, CRGB color) {
  for (uint16_t column = 0; column < LENGTH; column++) {
    light_led(row, column, color);
  }
}

/* fill an entire column with the given color */
void fill_column(uint16_t column, CRGB color) {
  for (uint16_t row = 0; row < LENGTH; row++) {
    light_led(row, column, color);
  }
}

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  fill_row(0, CRGB::White);
  fill_row(4, CRGB::White);
  fill_row(8, CRGB::White);
  fill_row(12, CRGB::White);
  fill_column(0, CRGB::White);
  fill_column(4, CRGB::White);
  fill_column(8, CRGB::White);
  fill_column(12, CRGB::White);
}

void loop() {
  for (uint16_t board = 0; board < NUM_BOARDS; board++) {
    for (uint16_t spot = 0; spot < NUM_SPOTS; spot++) {
      if (spot % 2) { light_led_board(board, spot, CRGB::Red); }
      else { light_led_board(board, spot, CRGB::Green); }
      FastLED.show();
    }
  }
  delay(500);
  for (uint16_t board = 0; board < NUM_BOARDS; board++) {
    fill_board(board, CRGB::Black);
    FastLED.show();
  }
}