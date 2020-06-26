# Hex-Dec-Bin base converter for the SSOpad

This keymap adds some higher layers which run a simple base converter on the LCD. One layer for decimal input, one for binary input, etc.\
I used QMK macros to run the code (does not send anything to the host computer from these layers). It is built off of the base LCD keymap here: https://github.com/JarofMolasses/qmk_firmware/tree/I2C_LCD/keyboards/ssopads/ssopadpm1.2/keymaps/LCD

![image](https://user-images.githubusercontent.com/33560291/85812099-9d080980-b714-11ea-8366-1add45b196e2.png)


### Notes: 
### 26 Juin
-turns out the binary display didn't overflow very gracefully, and that is most likely what interfered with the larger numbers\
-reinstated overflow notice: the binary output will go blank if the input value is larger than 0x3fff

### 25 Juin
-Strings are large, limit them as much as possible\
-entering some really big numbers still breaks some stuff, which is partially why the decimal input is capped at 6 digits 
- for instance, entering 9999999 would wipe the screen and enter a 7 in the decimal field (?)
- i'll deal with that later
