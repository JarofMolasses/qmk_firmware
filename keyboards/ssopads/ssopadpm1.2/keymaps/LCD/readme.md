## Adding support for PCF8574-based I2C LCDs
I could not find any examples of using I2C-backpacked HD44780 LCDs in QMK. This might be trivial to do with just base QMK libraries, but that kind of goes over my head 

## Update QMK Core and Submodules:
```
git pull https://github.com/qmk/qmk_firmware.git master
git submodule update --recursive --remote
git push origin master
```
## Add LCD library:
see: https://github.com/JarofMolasses/qmk_firmware/tree/master/lib/lcd_dprentice

cop  [I2C LCD source files](https://www.avrfreaks.net/comment/2640876#comment-2640876) posted by David Prentice\
I added to qmk_firmware/lib, but this may not be necessarily the best.

## rules.mk 
see: https://github.com/JarofMolasses/qmk_firmware/blob/master/keyboards/ssopads/ssopadpm1.2/keymaps/LCD/rules.mk

add path and build targets:
```
VPATH += $(LIB_PATH)/lcd_dprentice

SRC += lcd_i2cmaster.c \
       twimaster.c \
```

i have no idea how anything works
