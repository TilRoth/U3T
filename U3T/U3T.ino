#include <FastLED.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

const int rs = A4, en = A3, d4 = 13, d5 = 12, d6 = 11, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define NUM_LEDS 256
#define DATA_PIN A5
#define LENGTH 13
#define NUM_BOARDS 9
#define NUM_SPOTS 9
#define BRIGHTNESS 3

CRGB leds[NUM_LEDS];

#define BMATRIX_ROWS 3
#define BMATRIX_COLUMNS 3

char keys[BMATRIX_ROWS][BMATRIX_COLUMNS] = {
        {1,2,3},
        {4,5,6},
        {7,8,9}
};

byte pin_rows[BMATRIX_ROWS] = {9, 8, 7}; //connect to the row pinouts of the keypad
byte pin_column[BMATRIX_COLUMNS] = {5, 4, 3}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, BMATRIX_ROWS, BMATRIX_COLUMNS );

/* light led at array index */
void light_led(uint8_t array_index, CRGB color) {
    leds[array_index] = color;
}

/* get the array index for the given cordinates (i,j), i = row, j = column */
uint8_t index(uint8_t i, uint8_t j) {
    i++; j++;
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

void lcd_show(String line1, String line2 = ""){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0,1);
    lcd.print(line2);
}

void setup() {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    fill_row(4, CRGB::White);
    fill_row(8, CRGB::White);
    fill_column(4, CRGB::White);
    fill_column(8, CRGB::White);

    /* Setup LED */
    lcd.begin(16, 2);
    lcd_show("Ultimate","Tic-Tac-Toe");

    /* Setup board */
    outlineBigBoard(CRGB::White);
    FastLED.show();
    delay(1000);
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
            if ((i - startrow) % 4 != 0 and (j - startcolumn) % 4 != 0){
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

uint8_t waitUntilButtonPress(bool noblink=false){
    FastLED.show();
    unsigned long last_blink = millis();
    bool next_white = true;
    while(true){
        uint8_t res = keypad.getKey();
        if(res){
            return res-1;
        }
        if(not noblink and selectedboard != 9 and millis() - last_blink > 500){
            outlineSmallBoard(selectedboard, next_white ? CRGB::White : currentplayer ? CRGB::Blue : CRGB::Yellow);
            FastLED.show();
            last_blink = millis();
            next_white = not next_white;
        }
    }
}

template<typename Functor>
uint8_t checkBoardStatus(uint8_t board[], const Functor &light_spot, String win_string){
    uint8_t check_lines[8][3] = {{0,1,2},{3,4,5},{6,7,8},
                                 {0,3,6},{1,4,7},{2,5,8},
                                 {0,4,8},{2,4,6}};

    for (auto line : check_lines) {
        uint8_t first = line[0], second = line[1], third = line[2];
        if((board[first] == 1 or board[first] == 2) and board[first] == board[second] and board[first] == board[third]){

            // show winning string
            lcd_show("Player " + String(board[first] ? "1" : "2"), win_string);

            // blink winning line
            CRGB color = board[first] == 1 ? CRGB::Blue : CRGB::Yellow;
            for (int j = 0; j < 2; ++j) {
                light_spot(first, color);
                light_spot(second, color);
                light_spot(third, color);
                FastLED.show();
                delay(500);
                light_spot(first, CRGB::Black);
                light_spot(second, CRGB::Black);
                light_spot(third, CRGB::Black);
                FastLED.show();
                delay(500);
            }

            return board[first];
        }
    }

    for (uint8_t i = 0; i < 9; ++i) {
        if (board[i] == 0) {
            return 0;
        }
    }
    return 3; // no spot empty but no winner => tie
}

void(* resetFunc) () = nullptr;

void loop() {

    if(globalstatus != 0){
        lcd_show("Player " + String((globalstatus ? "1":"2")) + " Win");
        for (uint8_t i = 0; i < NUM_BOARDS; ++i) {
            fill_board(i, globalstatus == 1 ? CRGB::Blue : globalstatus == 2 ? CRGB::Yellow : CRGB::Red);
        }
        FastLED.show();
        waitUntilButtonPress(true);
        resetFunc();
    }

    lcd_show("Player " + String((currentplayer ? "1":"2")) + "'s Turn");

    if (selectedboard == 9){
        outlineBigBoard(currentplayer ? CRGB::Blue : CRGB::Yellow);
        uint8_t selection = waitUntilButtonPress();
        if(boardstatus[selection] == 0){
            outlineBigBoard(CRGB::White);
            selectedboard = selection;
        } else {
            lcd_show("Invalid move!");
            for (int i = 0; i < 3; ++i) {
                if(i > 0){
                    delay(300);
                }
                outlineSmallBoard(selection, CRGB::Red);
                FastLED.show();
                delay(300);
                outlineSmallBoard(selection, CRGB::White);
                outlineBigBoard(currentplayer ? CRGB::Blue : CRGB::Yellow);
                FastLED.show();
            }
        }
    } else {
        outlineSmallBoard(selectedboard, currentplayer ? CRGB::Blue : CRGB::Yellow);
        uint8_t selection = waitUntilButtonPress();
        if (boards[selectedboard][selection] == 0) {
            boards[selectedboard][selection] = currentplayer ? 1 : 2;
            boardstatus[selectedboard] = checkBoardStatus(boards[selectedboard], [](int spot, CRGB color){light_led_board(selectedboard,spot,color);}, "wins the board!");
            if (boardstatus[selectedboard] != 0) {
                globalstatus = checkBoardStatus(boardstatus, [](int spot, CRGB color){fill_board(spot,color);}, "wins the game!");
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
            lcd_show("Invalid move!");
            for (int i = 0; i < 3; ++i) {
                if(i > 0){
                    delay(300);
                }
                light_led_board(selectedboard, selection, CRGB::Red);
                FastLED.show();
                delay(300);
                light_led_board(selectedboard, selection, boards[selectedboard][selection] == 1 ? CRGB::Blue : CRGB::Yellow);
                FastLED.show();
            }
        }
    }


}
