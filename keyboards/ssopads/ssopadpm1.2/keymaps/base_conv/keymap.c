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
keymap for doing cool base conversions on the 16x2 LCD

note:
-Strings use a lot of storage, limit those;
*/

#include "lcd.h"
#include "i2cmaster.h" //fleury i2c

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
//#include <avr/io.h>
//#include <avr/interrupt.h>


//lcd available flag
uint8_t lcd = 0;
//record of last layer
uint8_t layer = 0;
//hexadecimal shift entry flag - ie was hexshift the last layer
uint8_t hexprevious = 0;

// Defines the keycodes used by our macros in process_record_user
enum ssopad_layers{ _BASE, _FUNC, _FUNC2, DEC, BIN, HEX, HEXSHIFT};

// Defines the macros for base conversion inputs
enum custom_keycodes{
    m0 = SAFE_RANGE, m1, m2, m3, m4, m5, m6, m7, m8, m9, xa, xb, xc, xd, xe, xf, del, res,
};


//base conversion setup variables
uint8_t inputDigits = 16;                //max number of input digits
const uint8_t bufferMax = 16;

uint32_t inArray[16];                     //user input array
char outbuffer[16];                      //output string buffer
uint8_t pos = 0;                         //array index

uint8_t displayX = 0;                    //display start x position for input readout
uint8_t displayY = 0;                    //display start y position for input readout

//base conversion and display functions
void setDisplay(uint8_t x, uint8_t y);
void reset(void);
void resetInput(void);
void clearArray(char[]);
//void intVal(uint32_t);              //does not work in startup test
uint32_t intVal(uint8_t);            //works in startup test

void printInput(void);
void printBinOut(uint8_t);
void printDecOut(uint8_t);
void printHexOut(uint8_t);
void printOutput(void);

void lcd_clearln(uint8_t);


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

    [DEC] = LAYOUT(  //conv decimal entry 
    del, TO(BIN),                  \
    m7,  m8,   m9,   KC_NO,  \
    m4,  m5,   m6,   res,  \
    m1,  m2,   m3,   KC_NO, \
    m0, TO(_BASE),   KC_NO, KC_NO  \
    ),

    [BIN] = LAYOUT(  //conv binary entry
    del,      TO(HEX),              \
    KC_NO,      KC_NO,   KC_NO,   KC_NO, \
    KC_NO,      KC_NO,   KC_NO,     res, \
    m1,         KC_NO,   KC_NO,   KC_NO, \
    m0,     TO(_BASE),   KC_NO,   KC_NO  \
    ),

    [HEX] = LAYOUT(  //conv hex entry
    del,  TO(DEC),                  \
    m7,        m8,      m9,        KC_NO, \
    m4,        m5,      m6,          res, \
    m1,        m2,      m3,        KC_NO, \
    m0, TO(_BASE),   KC_NO,  MO(HEXSHIFT) \
    ),

    [HEXSHIFT] = LAYOUT(  //conv hex, a-f
    del,     KC_TRNS,                  \
    KC_NO,   KC_NO,       KC_NO,      KC_NO,  \
    xe,      xf,          KC_NO,      KC_NO,  \
    xb,      xc,          xd,         KC_NO,  \
    xa,      TO(_BASE),   KC_TRNS,    KC_TRNS \
    ),

};

void matrix_init_user(void) {
    if(lcd_init(LCD_DISP_ON_CURSOR) == 0)      //init and check for lcd
    {
        lcd = 1;                     //raise lcd flag if init successful

        lcd_clrscr();
        lcd_puts("PRO MICRO v1.2");
        lcd_gotoxy(3, 1);
        lcd_puts("& knuckles");


        //pos = 7;
        //lcd_clrscr();
        //lcd_puts("HEX:");

        //lcd_gotoxy(9, 0);
        //ultoa(intVal(10), outbuffer, 16);
        //lcd_puts(outbuffer);

    }
}

//compute value of input array in int 
uint32_t intVal(uint8_t base)
{
    uint32_t val = 0;

    uint32_t multiplier = 1;

    for (int i = pos - 1; i >= 0; i--)
    {
        val += multiplier * inArray[i];
        multiplier *= base;
    }

    return val;
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

            if(lcd){
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
                inputDigits = 7;
                setDisplay(0, 0);

                resetInput();

                lcd_clrscr();
                lcd_gotoxy(0, 1);
                lcd_puts("DECIMAL INPUT");
                lcd_gotoxy(displayX, displayY);
            }
            break;

        case BIN:
            layer = 4;
            if (lcd) {
                hexprevious = 0;
                inputDigits = 14;
                setDisplay(2, 1);
                
                resetInput();

                lcd_clrscr();
                lcd_puts("BINARY INPUT");
                lcd_gotoxy(0, 1);
                lcd_puts("0b");
                lcd_gotoxy(displayX, displayY);
            }
            break;

        case HEX:
            layer = 5;
            if (lcd) {
                if (!hexprevious) {             //only reset if the last layer was not hexshift
                    inputDigits = 5;
                    setDisplay(10, 0);

                    resetInput();

                    lcd_clrscr();
                    lcd_gotoxy(8, 0);
                    lcd_puts("0x");
                    lcd_gotoxy(0, 1);
                    lcd_puts("HEX INPUT");
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

            if(lcd){
                lcd_clrscr();
                lcd_gotoxy(0, 1); lcd_puts("LAYER: BASE");
            }
            break;
        }
    return state;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case m0:
        if (record->event.pressed) {
            if (pos < inputDigits)
            {
                inArray[pos] = 0;
                pos++;

                printInput();printOutput();
            }
        }
        break;

    case m1:
        if (record->event.pressed) {
            if (pos < inputDigits)
            {
                inArray[pos] = 1;
                pos++;

                printInput();printOutput();
            }
        }
        break;

    case m2:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 2;
                pos++;

                printInput();printOutput();
            }
        }
        break;


    case m3:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 3;
                pos++;

                printInput();printOutput();
            }
        }
        break;

    case m4:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 4;
                pos++;

                printInput();printOutput();
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

                printInput();printOutput();
            }
        }
        break;

    case m7:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 7;
                pos++; 

                printInput();printOutput();
            }

        }
        break;

    case m8:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 8;
                pos++;

                printInput();printOutput();
            }
        }
        break;

    case m9:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 9;
                pos++;  

                printInput();printOutput();
            }
        }
        break;

    case xa:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 0xa;
                pos++; 

                printInput();printOutput();
            }
        }
        break;

    case xb:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 0xb;
                pos++;  

                printInput();printOutput();
            }
        }
        break;

    case xc:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 0xc;
                pos++;  

                printInput();printOutput();
            }
        }
        break;

    case xd:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 0xd;
                pos++;    

                printInput();printOutput();
            }
        }
        break;

    case xe:
        if (record->event.pressed) {
            if (pos < inputDigits) {
                inArray[pos] = 0xe;
                pos++;  

                printInput();printOutput();
            }
        }
        break;

    case xf:
        if (record->event.pressed) {
            if (pos < inputDigits) {

                inArray[pos] = 0xf;
                pos++;  

                printInput();printOutput();
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

void setDisplay(uint8_t x, uint8_t y) {
    displayX = x; displayY = y;
}

//clears a line from left to right on the lcd
//@params y: row to clear
void lcd_clearln(uint8_t y) {
    lcd_gotoxy(0,y);
    lcd_puts("                ");
}

//set input to zero, reset index to 0
void resetInput(void) {
    for (uint8_t i = 0; i < 16; ++i)
    {
        inArray[i] = 0;
    }
    pos = 0;
}

//clear character array; fill with string terminators
void clearArray(char arr[]) {
    for (uint8_t i = 0; i < bufferMax; ++i)
    {
        arr[i] = '\0';
    }
}


//clear everything and print current state of input array to LCD to displayX, displayY
void printInput(void) {

    lcd_clrscr();                     //just bloody erase the whole thing
    if (layer == DEC)                       //rewrite the previous legend
    {
        //lcd_puts("d");
    }
    else if (layer == BIN) {
        lcd_gotoxy(0, 1);
        lcd_puts("0b");

    }
    else if(layer == HEX ||layer == HEXSHIFT)
    { 
        lcd_gotoxy(8, 0);
        lcd_puts("0x");

    }
    for (uint8_t i = 0; i < pos; ++i) {
        lcd_gotoxy(i+displayX, displayY);

        if (inArray[i] >= 10) {
            lcd_putc(inArray[i] + 87);       //print as hex
        }
        else {
            lcd_putc(inArray[i] + 48);       //print as dec or bin
        }
    }

    //lcd_gotoxy(displayX + pos, displayY);
}

//reset all
void reset(void) {
    resetInput();
    printInput();
    lcd_gotoxy(displayX, displayY);
}

//for 16 char displays
void printBinOut(uint8_t base) {
    clearArray(outbuffer);
    uint32_t val = intVal(base);

    lcd_clearln(1);
    lcd_gotoxy(0, 1);
    lcd_puts("0b");

    /*
    display overflow prevention,
    because this function seems to cause some display glitches when it overflows
    */
    if(val <= 0x3fff)    //i like the 0b prefix for clarity, so the maximum binary width is 14-bit on the 16x2 LCD
    {
        ultoa(val, outbuffer, 2);
        lcd_gotoxy(2, 1);
        lcd_puts(outbuffer);
    }

}
void printDecOut(uint8_t base) {
    clearArray(outbuffer);

    //ultoa(intVal(base), outbuffer, 10);             //reminder: itoa returns negative two's complement
    sprintf(outbuffer, "%lu", intVal(base));          //maybe use sprintf for decimals

    lcd_gotoxy(0,0);
    lcd_puts("        ");

    lcd_gotoxy(0, 0);
    lcd_puts(outbuffer);
}
void printHexOut(uint8_t base) {
    clearArray(outbuffer);
    ultoa(intVal(base), outbuffer, 16);

    lcd_gotoxy(8, 0);
    lcd_puts("0x      ");

    lcd_gotoxy(10, 0);
    lcd_puts(outbuffer);
}


//translate stored int value into outputs, prints output to lcd
void printOutput(void) {
    if(layer == DEC)
    {
        printBinOut(10);    //weird display overflow/ input wipe: THIS WAS THE CULPRIT
        printHexOut(10);
    }

    else if (layer == BIN)
    {
        printDecOut(2);
        printHexOut(2);
    }

    else if (layer == HEX || layer  == HEXSHIFT)
    {
        printBinOut(16);
        printDecOut(16);
    }
    
    lcd_gotoxy(displayX, displayY);     //return cursor to input field 
}



