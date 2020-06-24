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

#include "lcd.h"
#include "i2cmaster.h" //fleury i2c
//#include "i2c_master.h"  //qmk i2c

#include <stdbool.h>
//#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
//#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

//lcd available flag
uint8_t lcd = 0;

//hexadecimal shift entry flag
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
    d_ent,
    b_ent,
    x_ent,
    res,
};


//record of last layer
uint8_t lastLayer = 0;

//base conversion setup variables
uint8_t inputMax = 10;
const uint8_t bufferMax = 16;

int inArray[10];                            //user input array
char outbuffer[16];                         //output string buffer
int val;                                    //interpret the value of the input array in int  
uint8_t pos = 0;                            //array index

uint8_t displayX = 0;               //display start x position for input readout
uint8_t displayY = 0;               //display start y position for input readout


//base conversion functions
void setDisplay(unsigned char x, unsigned char y);
void reset(void);
void resetInput(void);
void clearArray(char arr[]);
void intVal(uint8_t base);

void printInput(void);
void printBinOut(void);
void printDecOut(void);
void printHexOut(void);
void printOutput(uint8_t base);


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
    m4,  m5,   m6,   d_ent,  \
    m1,  m2,   m3,   KC_NO, \
    m0, TO(_BASE),   res, KC_NO  \
    ),

    [BIN] = LAYOUT(  //conv binary entry
    del,   TO(HEX),                  \
    KC_NO,  KC_NO,      KC_NO,   KC_NO,  \
    KC_NO,  KC_NO,      KC_NO,   b_ent,  \
    m1,      KC_NO,      KC_NO,   KC_NO, \
    m0,     TO(_BASE),   res,   KC_NO \
    ),

    [HEX] = LAYOUT(  //conv hex entry
    del, TO(DEC),                  \
    m7,  m8,   m9,   KC_NO,  \
    m4,  m5,   m6,   x_ent,  \
    m1,  m2,   m3,   KC_NO, \
    m0, TO(_BASE),   res,  MO(HEXSHIFT) \
    ),

    [HEXSHIFT] = LAYOUT(  //conv hex, a-f
    del,   KC_TRNS,                  \
    KC_NO,   KC_NO,      KC_NO,    KC_NO,  \
    xe,      xf,         KC_NO,    KC_NO,  \
    xb,      xc,          xd,      KC_NO,  \
    xa,     TO(_BASE),   KC_TRNS,    KC_TRNS \
    ),

};

void matrix_init_user(void) {

    if(lcd_init(LCD_DISP_ON_CURSOR_BLINK) == 0)      //init and check for lcd
    {
        lcd = 1;                     //raise lcd flag if init successful

        lcd_clrscr();
        lcd_puts("PRO MICRO v1.2");
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
            lastLayer = 1;
           
            writePinLow(B0);
            writePinHigh(D5);

            if(lcd){
                hexprevious = 0;
                lcd_home(); lcd_puts("                ");
                lcd_gotoxy(0, 1); lcd_puts("LAYER: NUM     ");
            }

            break;

        case _FUNC2:
            lastLayer = 2;
            writePinHigh(B0);
            writePinLow(D5);

            if (lcd) {
                hexprevious = 0;
                lcd_home(); lcd_puts("SHIFT          ");
            }
            break;
            
        case DEC:
            lastLayer = 3;
            if (lcd) {
                resetInput();
                setDisplay(2, 0);
                hexprevious = 0;
                inputMax = 5;
               
                clearArray(outbuffer);

                lcd_clrscr();
                lcd_puts("d             ");
                lcd_gotoxy(0, 1);
                lcd_puts("DECIMAL INPUT  ");
                lcd_gotoxy(displayX, displayY);
            }
            break;

        case BIN:
            lastLayer = 4;
            if (lcd) {
                resetInput();
                setDisplay(2, 1);
                hexprevious = 0;
                inputMax = 14;
               
                clearArray(outbuffer);

                lcd_clrscr();
                lcd_puts("BINARY INPUT   ");
                lcd_gotoxy(0, 1);
                lcd_puts("0b             ");
                lcd_gotoxy(displayX, displayY);
            }
            break;

        case HEX:
            lastLayer = 5;
            if (lcd) {
                
                setDisplay(11, 0);
                clearArray(outbuffer);
                inputMax = 4;

                if (!hexprevious) {
                    resetInput();

                    lcd_clrscr();
                    lcd_gotoxy(9, 0);
                    lcd_puts("0x            ");
                    lcd_gotoxy(0, 1);
                    lcd_puts("HEX INPUT      ");
                    lcd_gotoxy(displayX, displayY);
                }
            }
            break;

        case HEXSHIFT:
            hexprevious = 1;

            break;

        default:
            lastLayer = 0;
            writePinHigh(B0);
            writePinHigh(D5);

            if(lcd){
                lcd_home(); lcd_puts("                ");
                lcd_gotoxy(0, 1); lcd_puts("LAYER: BASE    ");
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
                lcd_gotoxy(pos+displayX, displayY);
                lcd_putc(' ');
            }
        }
        break;

    case d_ent:
        if (record->event.pressed) {
            intVal(10);
            resetInput();
            printOutput(10);
        }
        break;

    case b_ent:
        if (record->event.pressed) {
            intVal(2); 
            resetInput();
            printOutput(2);
        }
        break;

    case x_ent:
        if (record->event.pressed) {
            intVal(16); 
            resetInput();
            printOutput(16);
        }
        break;

    case res:
        if (record->event.pressed) {

            /*resetInput();
            printInput();
            lcd_gotoxy(displayX, displayY);*/
            reset();
        }
    }
    return true;
}


void setDisplay(unsigned char x, unsigned char y) {
    displayX = x; displayY = y;
}

//clears a line from left to right on the lcd
//@params x,y: coordinates of erase start
void clearln(uint8_t x, uint8_t y) {
    lcd_gotoxy(x,y);
    lcd_puts("                ");
}

//set input to zero, reset index to 0
void resetInput(void) {
    for (int i = 0; i < inputMax; i++)
    {
        inArray[i] = 0;
    }

    pos = 0;
}

//clear character array; fill with line endings
void clearArray(char arr[]) {
    for (int i = 0; i < bufferMax; i++)
    {
        arr[i] = '\0';
    }
}

//compute current int value of input array
//@param base: base of input number system
void intVal(uint8_t base) {
    val = 0;

    unsigned int multiplier = 1;      
    int i = pos-1;        //start at LSB, exponent 0

    while(i >= 0)        //run through each digit
    {
        val += inArray[i] * multiplier;
        multiplier *= base;
        i--;
    }
}

//clear everything and print current state of input array to LCD to displayX, displayY
void printInput(void) {

    //unsigned char oppositeRow;  //on a 2 line display (y = 1 or 0), the opposite row is just !y
    //oppositeRow = !displayY;

    //lcd_gotoxy(displayX, displayY);         //erase input line to the right of label
    //lcd_puts("               ");
    //int i;

    //for (i = displayX - 3; i >= 0; i--) {   //erase input line to left (goofy)
    //    lcd_gotoxy(i, displayY);
    //    lcd_puts(" ");
    //}
    //clearln(0, !displayY);                  //erase entire opposite line 


    lcd_clrscr();                           //just bloody erase the whole thing
    if (lastLayer == DEC)                   //rewrite the previous legend
    {
        lcd_puts("d");

    }
    else if (lastLayer == BIN) {
        lcd_gotoxy(0, 1);
        lcd_puts("0b");

    }
    else if(lastLayer == HEX ||lastLayer == HEXSHIFT)
    { 
        lcd_gotoxy(9, 0);
        lcd_puts("0x");

    }

    for (int i = 0; i < pos; i++) {
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

//for 16 char displays, binary always prints to the second line, dec to first line, etc
void printBinOut(void) {
    clearArray(outbuffer);
    itoa(val, outbuffer, 2);
    lcd_gotoxy(0, 1);
    lcd_puts("0b              ");
    lcd_gotoxy(2, 1);
    lcd_puts(outbuffer);
}
void printDecOut(void) {
    clearArray(outbuffer);

    //itoa(val, outbuffer, 10);             //reminder: itoa returns negative two's complement
    sprintf(outbuffer, "%u", val);          //maybe use sprintf for decimals

    lcd_home();
    lcd_puts("d      ");
    lcd_gotoxy(2, 0);
    lcd_puts(outbuffer);
}
void printHexOut(void) {
    clearArray(outbuffer);
    itoa(val, outbuffer, 16);
    lcd_gotoxy(9, 0);
    lcd_puts("0x         ");
    lcd_gotoxy(11, 0);
    lcd_puts(outbuffer);
}

//translate stored int value into outputs, prints output to lcd
//@param base: base of current layer number system
//assumes val is calculated 
void printOutput(uint8_t base) {

    if(base == 10)
    {
        printBinOut();

        printHexOut();
    }

    else if (base == 2)
    {
        printDecOut();

        printHexOut();
    }

    else if (base == 16)
    {
        printBinOut();

        printDecOut();
    }
}
