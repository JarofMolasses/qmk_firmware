# SSD1306 OLED 

QMK has a cool and good SSD1306 driver. This keymap currently uses it to display the active layer.
![image](https://user-images.githubusercontent.com/33560291/103841278-09f74700-5048-11eb-9266-775533154169.jpg)


(the code is awful and primitive) \
It's mostly not hardcoded, but for now the special internal keycodes (layer changes, macros) are out of range and you have to add those aliases manually. 
For the custom characters, I've just used https://joric.github.io/qle/ to generate a custom font bitmap. Then, when an alias needs to use one of the symbols, construct that string char-by-char in the array and refer to the symbol's address in the font bitmap.



