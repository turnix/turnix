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

#include <text_buffer.h>
#include <string.h>
#include <interrupt.h>
#include <sys/io.h>

enum {
	TEXT_BUFFER_SPACE	= ((TEXT_BUFFER_BLACK << 12) | \
				   (TEXT_BUFFER_LIGHT_GREY << 8) | ' '),
};

enum {
	TEXT_BUFFER_PORT_CMD	= 0x3d4,
	TEXT_BUFFER_PORT_DATA	= 0x3d5
};

enum {
	TEXT_BUFFER_CMD_MSB	= 14,
	TEXT_BUFFER_CMD_LSB	= 15
};

volatile unsigned short * const text_buffer_base_addr = (void *)0xb8000;

struct {
	char row, col;
} text_buffer;

void text_buffer_draw_char_at(char c, int row, int col, int bg, int fg)
{
	unsigned short v = ((bg & 0xf) << 12) | ((fg & 0xf) << 8) | c;

	row = row % TEXT_BUFFER_ROWS;
	col = col % TEXT_BUFFER_COLS;
	text_buffer_base_addr[row * TEXT_BUFFER_COLS + col] = v;
}

void text_buffer_draw_cursor(int row, int col)
{
	unsigned short value = row * TEXT_BUFFER_COLS + col;

	outb(TEXT_BUFFER_CMD_MSB, TEXT_BUFFER_PORT_CMD);
	outb(value >> 8, TEXT_BUFFER_PORT_DATA);
	outb(TEXT_BUFFER_CMD_LSB, TEXT_BUFFER_PORT_CMD);
	outb(value, TEXT_BUFFER_PORT_DATA);
}

void text_buffer_clear()
{
	int i;

	text_buffer.row = 0;
	text_buffer.col = 0;
	for (i = 0; i < TEXT_BUFFER_COLS * TEXT_BUFFER_ROWS; ++i)
		text_buffer_base_addr[i] = TEXT_BUFFER_SPACE;
	text_buffer_draw_cursor(text_buffer.row, text_buffer.col);
}

void text_buffer_draw_char(char c, int bg, int fg)
{
	switch (c) {
	case 0x08: /* backspace */
		if (text_buffer.col != 0) {
			text_buffer.col--;
		} else if (text_buffer.row != 0) {
			text_buffer.row--;
			text_buffer.col = TEXT_BUFFER_COLS - 1;
		}
		text_buffer_draw_char_at(' ', text_buffer.row, text_buffer.col,
					 TEXT_BUFFER_BLACK,
					 TEXT_BUFFER_LIGHT_GREY);
		break;
	case '\r':
		return;
	case '\n':
		text_buffer.col = 0;
		++text_buffer.row;
		break;
	case '\t': {
			int n = 8 - text_buffer.col % 8;

			while (n-- > 0)
				text_buffer_write_char(' ');
		}
		break;
	default:
		text_buffer_draw_char_at(c, text_buffer.row, text_buffer.col,
					 bg, fg);
		if (++text_buffer.col >= TEXT_BUFFER_COLS) {
			text_buffer.col = 0;
			++text_buffer.row;
		}
		break;
	}
	if (text_buffer.row >= TEXT_BUFFER_ROWS) {
		int i;

		text_buffer.row = TEXT_BUFFER_ROWS - 1;
		/* scroll up */
		memmove((void *)text_buffer_base_addr,
			(void *)(text_buffer_base_addr + TEXT_BUFFER_COLS),
			2 * TEXT_BUFFER_COLS * (TEXT_BUFFER_ROWS - 1));
		for (i = 0; i < TEXT_BUFFER_COLS; ++i)
			text_buffer_base_addr[TEXT_BUFFER_COLS * (TEXT_BUFFER_ROWS - 1) + i] = TEXT_BUFFER_SPACE;

	}
	text_buffer_draw_cursor(text_buffer.row, text_buffer.col);
}

void text_buffer_write_char(char c)
{
	text_buffer_draw_char(c, TEXT_BUFFER_BLACK, TEXT_BUFFER_LIGHT_GREY);
}

int putchar(int c)
{
	unsigned long flags = interrupt_disable();

	text_buffer_write_char(c);
	interrupt_enable(flags);

	return c;
}
