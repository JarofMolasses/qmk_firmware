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

#define ALIAS_MAX_LENGTH 5
#define ROW_OFFSET 3

void print_alias_matrix(const char *alias[][4]);

// Defines the keycodes used by our macros in process_record_user
enum ssopad_layers { _BASE, _FUNC, _FUNC2 };

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(/* BASE */
                     KC_BSPC, KC_GRV, KC_HOME, KC_UP, KC_PGUP, KC_VOLU, KC_LEFT, KC_DOWN, KC_RIGHT, KC_VOLD, KC_END, KC_NO, KC_PGDN, KC_LGUI, KC_INS, TG(1), KC_DEL, MO(2)),

    [_FUNC] = LAYOUT(/* NUM */
                     KC_BSPC, KC_PSLS, KC_7, KC_8, KC_9, KC_PAST, KC_4, KC_5, KC_6, KC_PMNS, KC_1, KC_2, KC_3, KC_PPLS, KC_0, KC_TRNS, KC_PDOT, LT(2, KC_PENT)),

    [_FUNC2] = LAYOUT(/* function */
                      BL_TOGG, KC_NO, KC_F7, KC_F8, KC_F9, BL_INC, KC_F4, KC_F5, KC_F6, BL_DEC, KC_F1, KC_F2, KC_F3, KC_CAPS, KC_F10, KC_NO, KC_NO, KC_TRNS),

};

//2D array of char pointers with key aliases
const char *base_alias[5][4] = {
    {"BSPC ", "`    ", "     ", "     "},
    {"HOME ", "UP   ", "PGUP ", "VOLU "},
    {"L    ", "DOWN ", "R    ", "VOLD "},
    {"END  ", "     ", "PGDN ", "LGUI "},
    {"INS  ", "NUM  ", "DEL  ", "FN   "}
};
const char *num_alias[5][4] = {
    {"BSPC ", "/    ", "     ", "     "},
    {"7    ", "8    ", "9    ", "*    "},
    {"4    ", "5    ", "6    ", "-    "},
    {"1    ", "2    ", "3    ", "+    "},
    {"0    ", "     ", ".    ", "R/FN "}
};
const char *func_alias[5][4] = {
    {"BL   ", "     ", "     ", "     "},
    {"F7   ", "F8   ", "F9   ", "BL+  "},
    {"F4   ", "F5   ", "F6   ", "BL-  "},
    {"F1   ", "F2   ", "F3   ", "CAPS "},
    {"F10  ", "     ", "     ", "     "}
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
            print_alias_matrix(num_alias);
            break;
        case _FUNC2:
            oled_write_ln_P(PSTR("Layer: FN"), false);
            print_alias_matrix(func_alias);
            break;
        case _BASE:
            oled_write_ln_P(PSTR("Layer: BASE"), false);
            print_alias_matrix(base_alias);


            //first alias printing tests
            /*for (int col = 0; col < MATRIX_COLS; col++) {

                for (int row = STARTING_ROW; row < (STARTING_ROW + MATRIX_ROWS); row++) {
                    oled_set_cursor(col*ALIAS_MAX_LENGTH, row); oled_write(base_alias[row-STARTING_ROW][col], false);
                }

            }*/

            break;

        default:
            break;
    }
}
#endif

//prints contents of alias arrays to OLED
void print_alias_matrix(const char *alias[][4]) {
    for (int col = 0; col < MATRIX_COLS; col++) {

        for (int row = 0; row < MATRIX_ROWS; row++) {

            int cursorCol = col * ALIAS_MAX_LENGTH, cursorRow = row + ROW_OFFSET;

            oled_set_cursor(cursorCol, cursorRow);
            if (strlen(alias[row][col]) == ALIAS_MAX_LENGTH) {
                 oled_write(alias[row][col], false);          //function cannot correct for alias string of wrong length, strlen(alias) is expected to be exactly ALIAS_MAX_LENGTH. should add some string processing to make this more robust?                                             
            }
            else {
                oled_write("_err_", false);                    //temporary: display error message if the alias string was not defined with the right length
            }
        }
    }
}



