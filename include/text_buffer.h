/*
 * Copyright (c) 2015 Changli Gao <xiaosuo@gmail.com>
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef TEXT_BUFFER_H
#define TEXT_BUFFER_H

#define TEXT_BUFFER_COLS 80
#define TEXT_BUFFER_ROWS 25

enum {
	TEXT_BUFFER_BLACK,
	TEXT_BUFFER_BLUE,
	TEXT_BUFFER_GREEN,
	TEXT_BUFFER_CYAN,
	TEXT_BUFFER_RED,
	TEXT_BUFFER_MAGENTA,
	TEXT_BUFFER_BROWN,
	TEXT_BUFFER_LIGHT_GREY,
	TEXT_BUFFER_DARK_GREY,
	TEXT_BUFFER_LIGHT_BLUE,
	TEXT_BUFFER_LIGHT_GREEN,
	TEXT_BUFFER_LIGHT_CYAN,
	TEXT_BUFFER_LIGHT_RED,
	TEXT_BUFFER_LIGHT_MAGENTA,
	TEXT_BUFFER_LIGHT_BROWN,
	TEXT_BUFFER_WHITE
};

void text_buffer_draw_char_at(char c, int row, int col, int bg, int fg);
void text_buffer_draw_cursor(int row, int col);

void text_buffer_clear();
#define text_buffer_init() text_buffer_clear()
void text_buffer_draw_char(char c, int bg, int fg);
void text_buffer_write_char(char c);

#endif  /* TEXT_BUFFER_H_ */
