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
 /*
 keymap for doing cool base conversions on the 20x4 LCD

 note:
 -Strings use a lot of storage, limit those
 */

#include "lcd.h"
#include "i2cmaster.h" //fleury i2c

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
 //#include <avr/io.h>
 //#include <avr/interrupt.h>

 // Defines the keycodes used by our macros in process_record_user
enum ssopad_layers { _BASE, _FUNC, _FUNC2, DEC, BIN, HEX, HEXSHIFT };

// Defines the macros for base conversion inputs
enum custom_keycodes {
    m0 = SAFE_RANGE, m1, m2, m3, m4, m5, m6, m7, m8, m9, xa, xb, xc, xd, xe, xf, del, res,
};

//base conversion variables
uint8_t inputDigits = 16;                //max number of input digits
const uint8_t bufferMax = 20;            //maximum size of output string

uint8_t inArray[20];                    //user input array
char outbuffer[20];                      //output string buffer

uint8_t pos = 0;                         //array index
uint8_t displayX = 0;                    //display start x position for input readout
uint8_t displayY = 0;                    //display start y position for input readout

uint8_t hexprevious = 0;                 //hexadecimal shift entry indicator - ie was hexshift the last layer

//misc variables
uint8_t lcd = 0;                         //lcd available indicator
uint8_t layer = 0;                       //current layer

//base conversion and display functions
void setDisplay(uint8_t x, uint8_t y);
void reset(void);
void resetInput(void);
void clearStr(char[]);
uint32_t intVal(uint8_t);

void lcd_clearln(uint8_t);
void printInput(void);
void printBinOut(uint8_t);
void printDecOut(uint8_t);
void printHexOut(uint8_t);
void printOutput(void);
void dispBase(void);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
    KC_BSPC, KC_GRV,                      \
    KC_HOME, KC_UP,   KC_PGUP,   KC_VOLU, \
    KC_LEFT, KC_DOWN, KC_RIGHT,  KC_VOLD, \
    KC_END,  KC_NO,   KC_PGDN,   KC_LGUI,  \
    KC_INS,   TG(1),   KC_DEL,    MO(2)  \
    ),

    [_FUNC] = LAYOUT(
    KC_BSPC, KC_PSLS,               \
    KC_7,  KC_8,   KC_9,   KC_PAST, \
    KC_4,  KC_5,   KC_6,   KC_PMNS, \
    KC_1,  KC_2,   KC_3,   KC_PPLS, \
    KC_0,  KC_TRNS, KC_PDOT, LT(2, KC_PENT) \
    ),

    [_FUNC2] = LAYOUT(
    BL_TOGG, TO(DEC),                  \
    KC_F7,  KC_F8,   KC_F9,   BL_INC,  \
    KC_F4,  KC_F5,   KC_F6,   BL_DEC,  \
    KC_F1,  KC_F2,   KC_F3,   KC_CAPS, \
    KC_F10,  KC_NO,   KC_NO,  KC_TRNS \
    ),

    [DEC] = LAYOUT(  //base converter decimal entry 
    del, TO(BIN),              \
    m7,        m8,      m9,   KC_NO, \
    m4,        m5,      m6,   res,   \
    m1,        m2,      m3,   KC_NO, \
    m0, TO(_BASE),   KC_NO,   KC_NO  \
    ),

    [BIN] = LAYOUT(  //base converter binary entry
    del,      TO(HEX),              \
    KC_NO,      KC_NO,   KC_NO,   KC_NO, \
    KC_NO,      KC_NO,   KC_NO,     res, \
    m1,         KC_NO,   KC_NO,   KC_NO, \
    m0,     TO(_BASE),   KC_NO,   KC_NO  \
    ),

    [HEX] = LAYOUT(  //base converter hex entry
    del,  TO(DEC),                  \
    m7,        m8,      m9,        KC_NO, \
    m4,        m5,      m6,          res, \
    m1,        m2,      m3,        KC_NO, \
    m0, TO(_BASE),   KC_NO,  MO(HEXSHIFT) \
    ),

    [HEXSHIFT] = LAYOUT(  //base converter hex, a-f
    del,     KC_TRNS,                  \
    KC_NO,   KC_NO,       KC_NO,      KC_NO,  \
    xe,      xf,          KC_NO,      KC_NO,  \
    xb,      xc,          xd,         KC_NO,  \
    xa,      TO(_BASE),   KC_TRNS,    KC_TRNS \
    ),
};

void matrix_init_user(void) {
    if (lcd_init(LCD_DISP_ON_CURSOR) == 0) {     //init and check for lcd
        lcd = 1;                                //update lcd variable if init successful

        lcd_clrscr();
        lcd_puts("PRO MICRO v1.2");
        lcd_gotoxy(3, 1);
        lcd_puts("& knuckles");


        //wip: how 2 print 64-bit integers?
        /*
        lcd_clrscr();
        double test64 = (uint64_t)1<<63;

        sprintf(outbuffer, "%.lf", test64);
        lcd_puts("64bit int testing");
        lcd_gotoxy(0, 1); lcd_puts(outbuffer);
        */

        //pos = 7;
        //lcd_clrscr();
        //lcd_puts("TEST HEX:");

        //lcd_gotoxy(9, 0);
        //ultoa(intVal(10), outbuffer, 16);
        //lcd_puts(outbuffer);
    }
}

/*
void matrix_scan_user(void) {

}

void led_set_user(uint8_t usb_led) {
}
*/

uint32_t layer_state_set_user(uint32_t state) {
    switch (biton32(state)) {
    case _FUNC:
        layer = 1;
        writePinLow(B0);
        writePinHigh(D5);

        if (lcd) {
            hexprevious = 0;
            lcd_clrscr();
            lcd_gotoxy(0, 1); lcd_puts("LAYER: NUM");
        }
        break;

    case _FUNC2:
        layer = 2;
        writePinHigh(B0);
        writePinLow(D5);

        if (lcd) {
            hexprevious = 0;
            lcd_clearln(0); lcd_home(); lcd_puts("SHIFT");
        }
        break;

    case DEC:
        layer = 3;
        if (lcd) {
            hexprevious = 0;
            inputDigits = 8;
            setDisplay(0, 1);

            resetInput();

            lcd_clrscr();
            dispBase();
            lcd_gotoxy(displayX, displayY);
        }
        break;

    case BIN:
        layer = 4;
        if (lcd) {
            hexprevious = 0;
            inputDigits = 18;
            setDisplay(2, 2);

            resetInput();

            lcd_clrscr();
            dispBase();
            lcd_gotoxy(0, 2);
            lcd_puts("0b");
            lcd_gotoxy(displayX, displayY);
        }
        break;

    case HEX:
        layer = 5;
        if (lcd) {
            if (!hexprevious) {             //only reset if the last layer was not hexshift
                inputDigits = 6;
                setDisplay(2, 3);

                resetInput();

                lcd_clrscr();
                dispBase();
                lcd_gotoxy(0, 3);
                lcd_puts("0x");
                lcd_gotoxy(displayX, displayY);
            }
        }
        break;

    case HEXSHIFT:
        hexprevious = 1;
        break;

    default:
        layer = 0;
        writePinHigh(B0);
        writePinHigh(D5);

        if (lcd) {
            lcd_clrscr();
            lcd_gotoxy(0, 1); lcd_puts("LAYER: BASE");
        }
        break;
    }
    return state;
}

/*massive memory usage*/
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case m0:
        if (record->event.pressed) {
            if (pos < inputDigits)
            {
                inArray[pos] = 0;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case m1:
        if (record->event.pressed) {
            if (pos < inputDigits)
            {
                inArray[pos] = 1;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case m2:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 2;
                pos++;

                printInput(); printOutput();
            }
        }
        break;


    case m3:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 3;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case m4:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 4;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case m5:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 5;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case m6:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 6;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case m7:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 7;
                pos++;

                printInput(); printOutput();
            }

        }
        break;

    case m8:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 8;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case m9:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 9;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case xa:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 0xa;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case xb:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 0xb;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case xc:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 0xc;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case xd:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 0xd;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case xe:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 0xe;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case xf:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 0xf;
                pos++;

                printInput(); printOutput();
            }
        }
        break;

    case del:
        if (record->event.pressed) {
            if (pos > 0) {
                pos--;

                inArray[pos] = 0;
                lcd_gotoxy(pos + displayX, displayY);
                lcd_putc(' ');

                printOutput();
            }
        }
        break;

    case res:
        if (record->event.pressed) {
            reset();
        }
        break;
    }
    return true;
}

/*Return current int value of input array*/
uint32_t intVal(uint8_t base) {
    uint32_t val = 0;
    uint32_t multiplier = 1;

    for (int i = pos - 1; i >= 0; i--) {
        val += multiplier * inArray[i];
        multiplier *= base;
    }
    return val;
}

void setDisplay(uint8_t x, uint8_t y) {
    displayX = x; displayY = y;
}

/*Clear a line at row y*/
void lcd_clearln(uint8_t y)
{
    lcd_gotoxy(0, y);
    lcd_puts("                    ");
}

/*Reset input array and index*/
void resetInput(void) {
    for (uint8_t i = 0; i < 16; ++i) {
        inArray[i] = 0;
    }
    pos = 0;
}

/*Clear a character array; fill with string terminators*/
void clearStr(char arr[]) {
    for (uint8_t i = 0; i < bufferMax; ++i) {
        arr[i] = '\0';
    }
}

/*Print input array to LCD, behaviour depends on active layer*/
void printInput(void) {
    if (layer == DEC) {                       //rewrite the previous legend
        lcd_clearln(1);
    }
    else if (layer == BIN) {
        lcd_gotoxy(0, 2);
        lcd_puts("0b                  ");
    }
    else if (layer == HEX || layer == HEXSHIFT) {
        lcd_gotoxy(0, 3);
        lcd_puts("0x                  ");
    }
    for (uint8_t i = 0; i < pos; ++i) {
        lcd_gotoxy(i + displayX, displayY);

        if (inArray[i] >= 10) {
            lcd_putc(inArray[i] + 87);       //print as hex
        }
        else {
            lcd_putc(inArray[i] + 48);       //print as dec or bin
        }
    }
    //lcd_gotoxy(displayX + pos, displayY);
}

/*Reset input to zero and clear the field*/
void reset(void) {
    resetInput();
    //printInput();

    lcd_clrscr();
    dispBase();
    lcd_gotoxy(0, 2); lcd_puts("0b");
    lcd_gotoxy(0, 3); lcd_puts("0x");
    lcd_gotoxy(displayX, displayY);
}

/*"Low level" printing function*/
void printBinOut(uint8_t base) {
    clearStr(outbuffer);
    uint32_t val = intVal(base);

    lcd_gotoxy(0, 2);
    lcd_puts("0b                  ");

    /*
    display overflow prevention because it seems like the display doesn't overflow very gracefully
    */
    if (val <= 0x3ffff) {                                 
        ultoa(val, outbuffer, 2);
        lcd_gotoxy(2,2);
        lcd_puts(outbuffer);
    }

}
void printDecOut(uint8_t base) {
    clearStr(outbuffer);
    ultoa(intVal(base), outbuffer, 10);           //use ultoa() for unsigned 32bit ints
    //sprintf(outbuffer, "%llu", intVal(base));
    lcd_gotoxy(0, 1); lcd_puts("                    ");
    lcd_gotoxy(0, 1);
    lcd_puts(outbuffer);
}
void printHexOut(uint8_t base) {
    clearStr(outbuffer);
    ultoa(intVal(base), outbuffer, 16);
    //sprintf(outbuffer, "%llX", intVal(base));
    lcd_gotoxy(0, 3);lcd_puts("0x                  ");
    lcd_gotoxy(2,3);
    lcd_puts(outbuffer);
}

/*Print outputs to LCD - behaviour depends on active layer*/
void printOutput(void) {
    if (layer == DEC) {
        printBinOut(10);    //weird display overflow/ input wipe: THIS WAS THE CULPRIT
        printHexOut(10);
    }
    else if (layer == BIN) {
        printDecOut(2);
        printHexOut(2);
    }
    else if (layer == HEX || layer == HEXSHIFT) {
        printBinOut(16);
        printDecOut(16);
    }
    dispBase();
    lcd_gotoxy(displayX, displayY);     //return cursor to input field
}


void dispBase(void) {
    lcd_home();
    if (layer == DEC) {
        lcd_puts("  [DECIMAL INPUT]");
    }
    else if (layer == BIN) {
        lcd_puts("   [BINARY INPUT]");
    }
    else if (layer == HEX || HEXSHIFT) {
        lcd_puts("    [HEX INPUT]");
    }
}
