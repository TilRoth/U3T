#include <FastLED.h>
#include <Arduino_AVRSTL.h>
#include <stdint.h>

#define NUM_LEDS 256
#define DATA_PIN A5
#define LENGTH 13
#define NUM_BOARDS 9
#define NUM_SPOTS 9
#define BRIGHTNESS 3

CRGB leds[NUM_LEDS];

/* light led at array index */
void light_led(uint8_t array_index, CRGB color) {
    leds[array_index] = color;
}

/* get the array index for the given cordinates (i,j), i = row, j = column */
uint8_t index(uint8_t i, uint8_t j) {
    return j & 0x01 ? j * 16 + 15 - i : j * 16 + i;
}

/* get the array index for the given spot on the given board */
uint8_t index_board(uint8_t board, uint8_t spot) {
    /* translate to index */
    uint8_t i = board - (board % 3) + spot / 3;
    uint8_t j = (board % 3) * 3 + (spot % 3);

    /* skip game boarders */
    i += i / 3 + 1;
    j += j / 3 + 1;

    return index(i, j);
}


/* light led at (i,j) coordinates, i = row, j = column */
void light_led(uint8_t i, uint8_t j, CRGB color) {
    light_led(index(i, j), color);
}

/* light led on game board, board = 0-8, spot = 0-8 */
void light_led_board(uint8_t board, uint8_t spot, CRGB color) {
    light_led(index_board(board, spot), color);
}

/* fill an entire board with the given color */
void fill_board(uint8_t board, CRGB color) {
    for (uint8_t spot = 0; spot < NUM_SPOTS; spot++) {
        light_led_board(board, spot, color);
    }
}

/* fill an entire row with the given color */
void fill_row(uint8_t row, CRGB color) {
    for (uint8_t column = 0; column < LENGTH; column++) {
        light_led(row, column, color);
    }
}

/* fill an entire column with the given color */
void fill_column(uint8_t column, CRGB color) {
    for (uint8_t row = 0; row < LENGTH; row++) {
        light_led(row, column, color);
    }
}

void setup() {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    pinMode(0, INPUT_PULLUP);
    pinMode(1, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);
    pinMode(6, INPUT_PULLUP);
    pinMode(7, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);

    fill_row(4, CRGB::White);
    fill_row(8, CRGB::White);
    fill_column(4, CRGB::White);
    fill_column(8, CRGB::White);
}


uint8_t count = 0;
uint8_t buttonState[9] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
uint8_t lastButtonState[9] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
unsigned long lastDebounceTime[9];
unsigned long debounceDelay = 50;

uint8_t checkButtonPress(){
    for (uint8_t i = 0; i < 9; ++i) {
        uint8_t reading = digitalRead(i);
        if (reading != lastButtonState[i]) {
            lastDebounceTime[i] = millis();
        }

        if ((millis() - lastDebounceTime[i]) > debounceDelay) {
            if (reading != buttonState[i]) {
                buttonState[i] = reading;
                if (buttonState[i] == LOW) {
                    return i;
                }
            }
        }

        lastButtonState[i] = reading;
    }
    return 9;
}

uint8_t waitUntilButtonPress(){
    FastLED.show();
    uint8_t res = 9;
    while(res == 9){
        res = checkButtonPress();
    }
    return res;
}

void outlineBigBoard(CRGB color){
    fill_row(0, color);
    fill_row(12, color);
    fill_column(0, color);
    fill_column(12, color);
}

void outlineSmallBoard(uint8_t board, CRGB color){
    uint8_t startrow = (board % 3) * 4;
    uint8_t startcolumn = (board / 3) * 4;
    for (uint8_t i = startrow; i < startrow + 5; ++i) {
        for (uint8_t j = startcolumn; j < startcolumn + 5; ++j) {
            if ((i - startrow) % 4 != 0 && (j - startcolumn) % 4 != 0){
                continue;
            }
            light_led(j,i, color);
        }
    }
}

// 0 = unfinished | 1 = blue won | 2 = yellow won | 3 = tie
uint8_t boardstatus[9] = {};
uint8_t globalstatus = 0;

bool currentplayer = true; // true = blue | false = yellow

uint8_t boards[9][9]; // 0 = empty | 1 = player true/blue | 2 = player false/yellow

uint8_t selectedboard = 9; // 9 = choice pending

uint8_t checkBoardStatus(uint8_t board[]){
    for (uint8_t i = 0; i < 3; ++i) {

        //check columns
        if(board[i] != 0 && board[i] == board[i + 3] && board[i] == board[i + 6]){
            return board[i];
        }

        //check rows
        if(board[i * 3] != 0 && board[i * 3] == board[i * 3 + 1] && board[i * 3] == board[i * 3 + 2]){
            return board[i * 3];
        }
    }

    //check diagonals
    if(board[0] != 0 && board[0] == board[4] && board[4] == board[8]){
        return board[0];
    }
    if(board[2] != 0 && board[2] == board[4] && board[4] == board[6]){
        return board[2];
    }

    for (uint8_t i = 0; i < 9; ++i) {
        if (board[i] == 0) {
            return 0;
        }
    }
    return 3; // no spot empty but no winner => tie
}

void loop() {
    
    if(globalstatus != 0){
        for (uint8_t i = 0; i < NUM_BOARDS; ++i) {
            fill_board(i, globalstatus == 1 ? CRGB::Blue : globalstatus == 2 ? CRGB::Yellow : CRGB::Red);
            FastLED.show();
        }
        delay(10000);
        return;
    }
    
    if (selectedboard == 9){
        outlineBigBoard(currentplayer ? CRGB::Blue : CRGB::Yellow);
        uint8_t selection = waitUntilButtonPress();
        if(boardstatus[selection] == 0){
            outlineBigBoard(CRGB::White);
            selectedboard = selection;
        } else {
            //TODO visual feedback board already finished
        }
    } else {
        outlineSmallBoard(selectedboard, currentplayer ? CRGB::Blue : CRGB::Yellow);
        uint8_t selection = waitUntilButtonPress();
        if (boards[selectedboard][selection] == 0) {
            boards[selectedboard][selection] = currentplayer ? 1 : 2;
            boardstatus[selectedboard] = checkBoardStatus(boards[selectedboard]);
            if (boardstatus[selectedboard] != 0) {
                globalstatus = checkBoardStatus(boardstatus);
                if(globalstatus == 0){
                    fill_board(selectedboard, boardstatus[selectedboard] == 1 ? CRGB::Blue : boardstatus[selectedboard] == 2 ? CRGB::Yellow : CRGB::Red);
                }
            } else {
                light_led_board(selectedboard, selection, currentplayer ? CRGB::Blue : CRGB::Yellow);
            }
            outlineSmallBoard(selectedboard, CRGB::White);
            selectedboard = boardstatus[selection] == 0 ? selection : 9;
            currentplayer = not currentplayer;
        }else{
            //TODO visual feedback cell not empty
        }
    }


}