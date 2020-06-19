i have no idea 

## Update QMK Core and Submodules:
```
git pull https://github.com/qmk/qmk_firmware.git master
git submodule update --recursive --remote
git push origin master
```
## Add LCD library:
```
cop source files (https://www.avrfreaks.net/comment/2640876#comment-2640876) 
and add to qmk library: qmk_firmware/lib/lcd_dprentice
```
## Rules.mk
```
add path and build targets:


VPATH += $(LIB_PATH)/lcd_dprentice

SRC += lcd_i2cmaster.c \
       twimaster.c \
```
