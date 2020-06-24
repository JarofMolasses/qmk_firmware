# Hex-Dec-Bin base converter for the SSOpad, 20x4 LCD version
Work in progress: it should be easy enough to adjust some code and make this work with a 20x4 LCD too. 

This keymap adds some higher layers which run a simple base converter on the LCD. \
I used QMK macros to run the code and not send anything to the host computer from these layers.


It uses the i2c LCD library ported in here: https://github.com/JarofMolasses/qmk_firmware/tree/I2C_LCD/lib/lcd_dprentice
