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

#pragma once

// place overrides here

// OLED overrides
#define OLED_DISPLAY_128X64
#define OLED_TIMEOUT 600000

//OLED custom font
#define OLED_FONT_H "keyboards/ssopads/lib/molassesfont.c"

//at these addresses in the custom font bitmap are the funny symbols
#define LEFT_SYMBOL 0x1b
#define RIGHT_SYMBOL 0x1a
#define DOWN_SYMBOL 0x19
#define UP_SYMBOL 0x18
#define RETURN_SYMBOL 0x80
#define BSPC_SYMBOL1 0x81
#define BSPC_SYMBOL2 0x82



