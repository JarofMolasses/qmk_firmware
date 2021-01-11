# SSD1306 OLED 

QMK has a cool and good SSD1306 driver. This keymap currently uses it to display the current layer and the keymap.
![image](https://user-images.githubusercontent.com/33560291/103841278-09f74700-5048-11eb-9266-775533154169.jpg)

It's mostly not hardcoded, but for now the special internal keycodes (layer changes, macros) are out of range and you have to add those aliases manually. 
(Awful code, don't use. Just for reference) For the custom characters, I've just used https://joric.github.io/qle/ to generate a custom font bitmap. Then, when an alias needs to use one of the symbols, construct that string char-by-char in the array and use the symbol's index in the font bitmap.



