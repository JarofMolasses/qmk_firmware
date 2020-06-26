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

//keymap for doing cool base conversions on the LCD
//note: Strings use a lot of storage, limit those

#include "lcd.h"
#include "i2cmaster.h" //fleury i2c
//#include "i2c_master.h"  //qmk i2c


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
enum custom_keycodes {
    m0 = SAFE_RANGE,
    m1,
    m2,
    m3,
    m4,
    m5,
    m6,
    m7,
    m8,
    m9,
    xa,
    xb,
    xc,
    xd,
    xe,
    xf,
    del,
    ent,
};


//base conversion setup variables
uint8_t inputMax = 16;
const uint8_t bufferMax = 16;

uint8_t inArray[16];                     //user input array
char outbuffer[16];                      //output string buffer
uint32_t val;                            //interpret the value of the input array in int  
uint8_t pos = 0;                         //array index

uint8_t displayX = 0;                    //display start x position for input readout
uint8_t displayY = 0;                    //display start y position for input readout

//base conversion and display functions
void setDisplay(uint8_t x, uint8_t y);
void reset(void);
void resetInput(void);
void clearArray(char[]);
void intVal(uint8_t);

void printInput(void);
void printBinOut(void);
void printDecOut(void);
void printHexOut(void);
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
    m4,  m5,   m6,   ent,  \
    m1,  m2,   m3,   KC_NO, \
    m0, TO(_BASE),   KC_NO, KC_NO  \
    ),

    [BIN] = LAYOUT(  //conv binary entry
    del,      TO(HEX),              \
    KC_NO,      KC_NO,   KC_NO,   KC_NO, \
    KC_NO,      KC_NO,   KC_NO,     ent, \
    m1,         KC_NO,   KC_NO,   KC_NO, \
    m0,     TO(_BASE),   KC_NO,   KC_NO  \
    ),

    [HEX] = LAYOUT(  //conv hex entry
    del,  TO(DEC),                  \
    m7,        m8,      m9,        KC_NO, \
    m4,        m5,      m6,          ent, \
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


        char testBuf[16];
        uint32_t testVal = 70000;

        lcd_clrscr();
        lcd_puts("TESTING HEX:");
        lcd_gotoxy(0, 1);
        ltoa(testVal, testBuf, 16);
        lcd_puts(testBuf);

        /*
        _delay_ms(3000);
        clearArray(testBuf);
        lcd_clrscr();
        testVal = 0xfffff;
        lcd_puts("TESTING DEC:");
        lcd_gotoxy(0, 1);
        sprintf(testBuf, "%lu", testVal);
        lcd_puts(testBuf);*/
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
                lcd_home(); lcd_puts("SHIFT");
            }
            break;
            
        case DEC:
            layer = 3;
            if (lcd) {
                hexprevious = 0;
                inputMax = 5;
                setDisplay(2, 0);

                resetInput();
                clearArray(outbuffer);

                lcd_clrscr();
                lcd_puts("d");
                lcd_gotoxy(0, 1);
                lcd_puts("DECIMAL INPUT");
                lcd_gotoxy(displayX, displayY);
            }
            break;

        case BIN:
            layer = 4;
            if (lcd) {
                hexprevious = 0;
                inputMax = 14;
                setDisplay(2, 1);
                
                resetInput();
                clearArray(outbuffer);

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
                    inputMax = 5;
                    setDisplay(11, 0);

                    resetInput();
                    clearArray(outbuffer);

                    lcd_clrscr();
                    lcd_gotoxy(9, 0);
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
            if (pos < inputMax)
            {
                inArray[pos] = 0;
                pos++;

                printInput();
            }
        }
        break;

    case m1:
        if (record->event.pressed) {
            if (pos < inputMax)
            {
                inArray[pos] = 1;
                pos++;

                printInput();
            }
        }
        break;

    case m2:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 2;
                pos++;

                printInput();
            }
        }
        break;


    case m3:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 3;
                pos++;

                printInput();
            }
        }
        break;

    case m4:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 4;
                pos++;

                printInput();
            }
        }
        break;

    case m5:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 5;
                pos++;

                printInput();
            }
        }
        break;

    case m6:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 6;
                pos++;

                printInput();
            }
        }
        break;

    case m7:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 7;
                pos++;

                printInput();
            }

        }
        break;

    case m8:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 8;
                pos++;

                printInput();
            }
        }
        break;

    case m9:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 9;
                pos++;

                printInput();
            }
        }
        break;

    case xa:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 0xa;
                pos++;

                printInput();
            }
        }
        break;

    case xb:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 0xb;
                pos++;

                printInput();
            }
        }
        break;

    case xc:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 0xc;
                pos++;

                printInput();
            }
        }
        break;

    case xd:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 0xd;
                pos++;

                printInput();
            }
        }
        break;

    case xe:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 0xe;
                pos++;

                printInput();
            }
        }
        break;

    case xf:
        if (record->event.pressed) {
            if (pos < inputMax) {
                inArray[pos] = 0xf;
                pos++;

                printInput();
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
            }
        }
        break;

    case ent:
        if (record->event.pressed) {
            if (layer == DEC)
            {
                intVal(10);
                resetInput();
                printOutput();
            }
            else if (layer == BIN)
            {
                intVal(2);
                resetInput();
                printOutput();
            }
            else if (layer == HEX)
            {
                intVal(16);
                resetInput();
                printOutput();
            }
        }
        break;

    /* old per-layer enter macros */
    //case d_ent:
    //    if (record->event.pressed) {
    //        intVal(10);
    //        resetInput();
    //        printOutput();
    //    }
    //    break;

    //case b_ent:
    //    if (record->event.pressed) {
    //        intVal(2);
    //        resetInput();
    //        printOutput();
    //    }
    //    break;

    //case x_ent:
    //    if (record->event.pressed) {
    //        intVal(16);
    //        resetInput();
    //        printOutput();
    //    }
    //    break;

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
    for (uint8_t i = 0; i < inputMax; i++)
    {
        inArray[i] = 0;
    }
    pos = 0;
}

//clear character array; fill with string terminators
void clearArray(char arr[]) {
    for (uint8_t i = 0; i < bufferMax; i++)
    {
        arr[i] = '\0';
    }
}

//compute current int value of input array
//@param base: base of input number system
void intVal(uint8_t base) {
    val = 0;

    uint16_t multiplier = 1;

    /*
    int i = pos-1;        //start at LSB, exponent 0

    while(i >= 0)        //run through each digit
    {
        val += (inArray[i] * multiplier);
        multiplier *= base;
        i--; 
    }
    */

    for (int i = pos - 1; i >= 0; i--)
    {
        val += (inArray[i] * multiplier);
        multiplier *= base;
    }
}

//clear everything and print current state of input array to LCD to displayX, displayY
void printInput(void) {

    lcd_clrscr();                           //just bloody erase the whole thing
    if (layer == DEC)                       //rewrite the previous legend
    {
        lcd_puts("d");

    }
    else if (layer == BIN) {
        lcd_gotoxy(0, 1);
        lcd_puts("0b");

    }
    else if(layer == HEX ||layer == HEXSHIFT)
    { 
        lcd_gotoxy(9, 0);
        lcd_puts("0x");

    }

    for (uint8_t i = 0; i < pos; i++) {
        lcd_gotoxy(i+displayX, displayY);

        if (inArray[i] >= 10) {
            lcd_putc(inArray[i] + 87);       //print as hex
        }
        else {
            lcd_putc(inArray[i] + 48);       //print as dec or bin
        }
    }
}

//reset all
void reset(void) {
    resetInput();
    printInput();
    lcd_gotoxy(displayX, displayY);
}

//for 16 char displays
//why the heck does the hex display still overflow at >65535
void printBinOut(void) {
    clearArray(outbuffer);
    utoa(val, outbuffer, 2);
    lcd_gotoxy(0, 1);
    lcd_puts("0b             ");
    lcd_gotoxy(2, 1);

    if (val <= 0x3fff) {
        lcd_puts(outbuffer);
    }
    else
    {
        lcd_puts("   OVERFLOW    ");
    }
}
void printDecOut(void) {
    clearArray(outbuffer);

    //itoa(val, outbuffer, 10);             //reminder: itoa returns negative two's complement
    sprintf(outbuffer, "%lu", val);         //maybe use sprintf for decimals

    lcd_home();
    lcd_puts("d      ");
    lcd_gotoxy(2, 0);
    lcd_puts(outbuffer);
}
void printHexOut(void) {
    clearArray(outbuffer);
    ultoa(val, outbuffer, 16);               
    lcd_gotoxy(9, 0);
    lcd_puts("0x     ");
    lcd_gotoxy(11, 0);
    lcd_puts(outbuffer);
}

/*
translate stored int value into outputs, prints output to lcd
assumes val is calculated
@param base: base of current layer number system
*/
void printOutput(void) {
    if(layer == DEC)
    {
        printBinOut();
        printHexOut();
    }

    else if (layer == BIN)
    {
        printDecOut();
        printHexOut();
    }

    else if (layer == HEX || layer  == HEXSHIFT)
    {
        printBinOut();
        printDecOut();
    }
    lcd_gotoxy(displayX, displayY);
}
