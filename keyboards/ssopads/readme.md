# SSOpad, weird 18-key macro pad prototype

## neat pictures
V1.2 Teensy, underglow on a perfboard
![V1.2 Teensy, underglow](https://user-images.githubusercontent.com/33560291/85184022-05ea1000-b243-11ea-93e6-429593b7d0d9.jpg)

#### Hardware
see PCB files at: https://github.com/JarofMolasses/SSOpad

#### Supports both Teensy and Pro Micro/Elite-C
the PCB has the reset pins of the Pro Micro broken out for a 6x3.5mm SMD tact switch underneath

## Adding support for PCF8574-based I2C LCDs
I could not find any examples of using I2C-backpacked HD44780 LCDs in QMK. This might be trivial to do with just base QMK libraries, but that kind of goes over my head 

#### Update QMK Core and Submodules:
```
git pull https://github.com/qmk/qmk_firmware.git master
git submodule update --recursive --remote
git push origin master
```
#### Add LCD library:
see: https://github.com/JarofMolasses/qmk_firmware/tree/master/lib/lcd_dprentice

cop  [I2C LCD source files](https://www.avrfreaks.net/comment/2640876#comment-2640876) posted by David Prentice\
I added to qmk_firmware/lib, but this may not be necessarily the best.

#### rules.mk 
see: https://github.com/JarofMolasses/qmk_firmware/blob/master/keyboards/ssopads/ssopadpm1.2/keymaps/LCD/rules.mk

add path and build targets:
```
VPATH += $(LIB_PATH)/lcd_dprentice

SRC += lcd_i2cmaster.c \
       twimaster.c \
```

i have no idea how anything works




