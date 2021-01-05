/* Copyright 2019 molasses
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
#include <string.h>
#include <stdio.h>

#define ALIAS_MAX_LENGTH 5
#define ROW_OFFSET 3
#define BUFFER_SIZE 6
#define NUMBER_OF_LAYERS 3

void oled_print_static_aliases(uint8_t);
void oled_print_keymap_aliases(uint8_t);
void clearStr(char[]);

enum ssopad_layers { _BASE, _FUNC, _FUNC2 };

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(/* BASE */
        KC_BSPC, KC_GRV,      \
        KC_HOME, KC_UP, KC_PGUP, KC_VOLU,    \
        KC_LEFT, KC_DOWN, KC_RIGHT, KC_VOLD, \
        KC_END, KC_NO, KC_PGDN, KC_LGUI,     \
        KC_INS, TG(_FUNC), KC_DEL, MO(_FUNC2)     \
     ),


    [_FUNC] = LAYOUT(/* NUM */
        KC_BSPC, KC_PSLS,\
        KC_7, KC_8, KC_9, KC_PAST,\
        KC_4, KC_5, KC_6, KC_PMNS,\
        KC_1, KC_2, KC_3, KC_PPLS,\
        KC_0, KC_TRNS, KC_PDOT, LT(_FUNC2, KC_PENT)\
    ),

    [_FUNC2] = LAYOUT(/* function */
        KC_BSPC, KC_NO,\
        KC_F7, KC_F8, KC_F9, BL_INC,\
        KC_F4, KC_F5, KC_F6, BL_DEC,\
        KC_F1, KC_F2, KC_F3, KC_CAPS,\
        KC_F10, KC_NO, KC_NO, KC_TRNS\
    ),

};

/* 2D backup array of aliases for each layer
use these to fill out any special keycodes outside of the 0x0 - 0xFF range
*/
const char * static_aliases[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = {
        {"    ", "    ", "     ", "     "},
        {"    ", "    ", "     ", "     "},
        {"    ", "    ", "     ", "     "},
        {"    ", "    ", "     ", "     "},
        {"    ", "NUM ", "     ", "FN   "}
    },

    [_FUNC] = {
        {"", "", "", ""},
        {"", "", "", ""},
        {"", "", "", ""},
        {"", "", "", ""},
        {"", "BASE ", "", "R/FN "}
    },

    [_FUNC2] = {
        {"", "", "", ""},
        {"", "", "", "BL+  "},
        {"", "", "", "BL-  "},
        {"", "", "", ""},
        {"", "", "", ""}
    }
};


/* smarter array of aliases in PROGMEM? it's FUCKING massive, though*/
#define NUMBER_OF_ALIASES 0xF0
const char keycode_aliases[NUMBER_OF_ALIASES][BUFFER_SIZE] PROGMEM = {
    "     ",
    "TRNS ",
    "POSTF",
    "UNDEF",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M", //0x10
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "↲",
    "ESC",
    "BSPC",
    "TAB",
    "SPC",
    "-",
    "=",
    "[",
    "]",
    "\\",
    "#",
    ";",
    "'",
    "`",
    ",",
    ".",
    "/",
    "CAPS",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "PTSC",
    "SCLK",
    "PAUSE",
    "INS",
    "HOME",
    "PGUP",
    "DEL",
    "END",
    "PGDN",
    "R",
    "L",    //0x50
    "DOWN",
    "UP",
    "NUMLK",
    "/",    //Keypad versions follow
    "*",
    "-",
    "+",
    "↲",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    ".",
    "\\",
    "APP",
    "PWR",
    "=",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "EXEC",
    "HELP",
    "MENU",
    "SEL",
    "STOP",
    "AGAIN",
    "UNDO",
    "CUT",
    "COPY",
    "PASTE",
    "FIND",
    "MUTE",
    "VOL+",
    "VOL-",
    "CAPS_L",
    "NUM_L",
    "SC_L",
    ",",
    "=",
    "I1",
    "I2",
    "I3",
    "I4",
    "I5",
    "I6",
    "I7",
    "I8",
    "I9",
    "L1",
    "L2",
    "L3",
    "L4",
    "L5",
    "L6",
    "L7",
    "L8",
    "L9",
    "A_ERS",
    "SYSR",
    "CANC",
    "CLR",
    "PRIOR",
    "RET",
    "SEP",
    "OUT",
    "OPER",
    "CLR_A",
    "CRSEL",
    "EXSEL",
    "S_PWR",
    "SLP",
    "WAKE",
    "MUTE",
    "VOL+",
    "VOL-",
    "NEXT",
    "PREV",
    "STOP",
    "PLAY",
    "MSEL",
    "EJEC",
    "MAIL",
    "CALC",
    "MCOMP",
    "SEARC",
    "HOME",
    "BACK",
    "FWD",
    "STOP",
    "REFR",
    "FAV",
    "FF",
    "REW",
    "BR+",
    "BR-",
    "FN0",
    "FN1",
    "FN2",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "CTRL",     //0xE0
    "SHIFT",
    "ALT",
    "LGUI",
    "CTRL",
    "SHIFT",
    "ALT",
    "RGUI",
};


void matrix_init_user(void) {}

void matrix_scan_user(void) {}

void led_set_user(uint8_t usb_led) {
    // underglow
}

uint32_t layer_state_set_user(uint32_t state) {
    switch (biton32(state)) {
        case _FUNC:
            writePinLow(B0);
            writePinHigh(D5);
            break;

        case _FUNC2:
            writePinHigh(B0);
            writePinLow(D5);
            break;

        default:
            writePinHigh(B0);
            writePinHigh(D5);
            break;
    }
    return state;
}

#ifdef OLED_DRIVER_ENABLE
void oled_task_user(void) {
    switch (get_highest_layer(layer_state)) {
        case _FUNC:
            oled_write_ln_P(PSTR("Layer: NUM"), false);
            oled_print_keymap_aliases(_FUNC);
            break;

        case _FUNC2:
            oled_write_ln_P(PSTR("Layer: FN"), false);
            oled_print_keymap_aliases(_FUNC2);
            break;

        case _BASE:
            oled_write_ln_P(PSTR("Layer: BASE"), false);
            oled_print_keymap_aliases(_BASE);

        default:
            break;
    }
}
#endif

//test: prints contents of hardcoded alias arrays to OLED
void oled_print_static_aliases(uint8_t layer) {
    for (int col = 0; col < MATRIX_COLS; col++) {

        for (int row = 0; row < MATRIX_ROWS; row++) {

            char buffer[BUFFER_SIZE];

            sprintf(buffer, "%-5s", static_aliases[layer][row][col]);

            int cursorCol = col * ALIAS_MAX_LENGTH, cursorRow = row + ROW_OFFSET;
            oled_set_cursor(cursorCol, cursorRow);

            oled_write(buffer, false);
        }
    }
}

//print keycode aliases to OLED, from keymap
//FIXME: need mechanism for handling transparent keys - scroll up layers until non-transparent key is found
void oled_print_keymap_aliases(uint8_t layer)
{
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {

            bool trans_OOB = false;                                                      //flag indicates trans key is covering OOB key
            uint8_t static_alias_layer = layer;                                          //normally, static_alias_layer = layer
            char buffer[BUFFER_SIZE];
            char buffer_intermediate[BUFFER_SIZE];                                       //for whitespace formatting purposes when using aliases

            uint16_t keycode = pgm_read_word(&(keymaps[layer][row][col]));               //retrieve PROGMEM values using pgmspace commands

            /* two ways to print the raw hex */
            //ultoa(keycode, buffer, 16);               
            //sprintf(buffer, "%-5x", keycode);

            if (keycode < NUMBER_OF_ALIASES) {                                                  //"normal" keycodes are within this range. special ones are too big for the progmem array
                if (keycode == KC_ROLL_OVER) {                                                  //if key is trans
                    for (int i = layer-1; i >= 0; i--) {                                        //while loop is preferrable. change later 
                        uint16_t check_lower_keycode = pgm_read_word(&(keymaps[i][row][col]));
                        if (check_lower_keycode != KC_ROLL_OVER) {
                            if (check_lower_keycode < NUMBER_OF_ALIASES) {
                                keycode = check_lower_keycode;                                  //change keycode to first non-transparent key, if recognizable
                                break;
                            }
                            else {                                                              //otherwise set flag and set custom static alias layer
                                trans_OOB = true;
                                static_alias_layer = i;
                                break;
                            }
                        }
                    }
                }
                if (trans_OOB) {                                                                                //if trans flag set, refer to backup but at custom static alias layer
                    sprintf(buffer, "%-5s", static_aliases[static_alias_layer][row][col]);
                    //sprintf(buffer, "%-5x", keycode);                                                         //option: print raw hex
                    //sprintf(buffer, "%-5s", "FN");                                                            //option: indicate special key 
                }
                else {                                                                                          //otherwise, normal stuff
                    memcpy_P(&buffer_intermediate, &keycode_aliases[keycode], sizeof buffer_intermediate);      //memcpy the alias to first buffer from flash
                    sprintf(buffer, "%-5s", buffer_intermediate);                                               //sprintf to the second buffer for formatting
                }
            }
            else {
                sprintf(buffer, "%-5s", static_aliases[layer][row][col]);                                       //refer to the hardcoded backup at regular layer 
                //sprintf(buffer, "%-5x", keycode);                                                             //option: print raw hex
                //sprintf(buffer, "%-5s", "FN");                                                                //option: indicate special key 
            }

            /*physical display layout compensation*/
            int cursorRow = row + ROW_OFFSET;                      
            int cursorCol = col * ALIAS_MAX_LENGTH; 

            oled_set_cursor(cursorCol, cursorRow);

            oled_write(buffer, false);
        }
    }
}

/*Clear a character array; fill with string terminators*/
void clearStr(char arr[]) {
    for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
        arr[i] = '\0';
    }
}

