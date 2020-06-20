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

#include "lcd.h"
#include "i2cmaster.h"

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include QMK_KEYBOARD_H

enum ssopad_layers { _BASE, _FUNC, _FUNC2};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT( /* function2 */
	KC_BSPC, KC_GRV,                      \
    KC_HOME, KC_UP,   KC_PGUP,   KC_VOLU, \
    KC_LEFT, KC_DOWN, KC_RIGHT,  KC_VOLD, \
	KC_END,  KC_NO,   KC_PGDN,   KC_LGUI, \
	KC_INS,   TG(1),   KC_DEL,    MO(2)   \
    ),

    [_FUNC] = LAYOUT( /* Base */
	KC_BSPC, KC_PSLS,                 \
    KC_7,    KC_8,   KC_9,   KC_PAST, \
    KC_4,    KC_5,   KC_6,   KC_PMNS, \
	KC_1,    KC_2,   KC_3,   KC_PPLS, \
	KC_0,    KC_TRNS, KC_PDOT, LT(2, KC_PENT) \
    ),
     
    [_FUNC2] = LAYOUT( /* function */
	BL_TOGG, KC_NO,                  \
    KC_F7,  KC_F8,   KC_F9,   BL_INC,  \
    KC_F4,  KC_F5,   KC_F6,   BL_DEC,  \
	KC_F1,  KC_F2,   KC_F3,   KC_CAPS, \
	KC_F10,  KC_NO,   KC_NO,  KC_TRNS \
  ),

};

void matrix_init_user(void) {
    setPinOutput(D4);
    //setPinOutput(D6);   //really weird stuff, prevents LED from being too bright

    i2c_init();
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    lcd_puts("ssopad1.2 teensy");
}

void matrix_scan_user(void) {
    switch (biton32(layer_state)) {
        case _FUNC:
            writePinHigh(D4);
            writePinLow(D6);
            break;

        case _FUNC2:
            writePinHigh(D6);
            writePinLow(D4);
            break;

        default:
            writePinLow(D4);
            writePinLow(D6);
            break;
    }
   
}

void led_set_user(uint8_t usb_led) {}


/*
uint32_t layer_state_set_kb(uint32_t state) {
    // if we are on layer 1
    if (state == 2) { 
        // light indicator led
          //PORTD |= (1 << PD0);
          lcd_clrscr();
          lcd_puts("NUM LOCK ON    ");

    } else {
          //PORTD &= ~(1 << PD0);
          lcd_clrscr();
          lcd_puts("NUM LOCK OFF    ");

    }
    return state;
}
*/



