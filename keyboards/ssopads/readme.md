# SSOpad, weird 18-key macro pad prototype

### neat pictures
V1.2 Teensy, with underglow 
![V1.2 Teensy, underglow](https://user-images.githubusercontent.com/33560291/85184022-05ea1000-b243-11ea-93e6-429593b7d0d9.jpg)

### Hardware
see PCB files at: https://github.com/JarofMolasses/SSOpad

### Supports both Teensy and Pro Micro/Elite-C
the PCB has the reset pins of the Pro Micro broken out for a 6x3.5mm SMD tact switch underneath


# Adding support for PCF8574-based I2C LCDs
I could not find any examples of using I2C-backpacked HD44780 LCDs in QMK. This might be trivial to do with just base QMK libraries, but that kind of goes over my head 

### Update QMK Core and Submodules:
```
git pull https://github.com/qmk/qmk_firmware.git master
git submodule update --recursive --remote
git push origin master
```
### Add I2C LCD library (with some tweaks):
see: https://github.com/JarofMolasses/qmk_firmware/tree/master/lib/lcd_dprentice

I used these: [I2C LCD source files](https://www.avrfreaks.net/comment/2640876#comment-2640876) posted by David Prentice\
I've since made quite a few changes to these, in order to make it work with the Teensy (port B5 was being used in the original) and to make cold-swapping LCDs possible. (with Fleury's I2C library as is, powering on the keyboard without an LCD attached would lock up the firmware.) 

### Changes to rules.mk 
see: https://github.com/JarofMolasses/qmk_firmware/blob/master/keyboards/ssopads/ssopadpm1.2/keymaps/LCD/rules.mk \
add path and build targets:
```
VPATH += $(LIB_PATH)/lcd_dprentice

SRC += lcd_i2cmaster.c \
       twimaster.c \
```
### Changes to [keymap]/config.h
add I2C address override, if your LCD does not use 0x27:
```
#define PCF8574A 0x3F //for example, set address to 0x3F
```

### Calling LCD functions 
Generally, I'd use keymap.c to run the LCD, something like:
```
#include "lcd.h"
#include "i2cmaster.h"
#include <stdbool.h>

//Set up a flag for the lcd initialization
bool lcd = false;

//Initialize the lcd in matrix_init_user(): (make sure matrix_init_kb() calls matrix_init_user(), if the former is used as well)
matrix_init_user(){
    if(lcd_init(LCD_DISP_ON) == 0)
    {
        lcd = true;
        
        //other startup screen stuff, if you like
        lcd_clrscr();
        lcd_puts("h");
        lcd_home();
    }
}
```





