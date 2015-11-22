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

/* 8042 */

#include <keyboard.h>
#include <stdio.h>
#include <stdint.h>
#include <interrupt.h>
#include <pic.h>
#include <ctype.h>
#include <circular_buffer.h>
#include <kernel.h>
#include <pthread.h>

#include <sys/io.h>

enum {
	KEYBOARD_IRQ		= 33
};

enum {
	KEYBOARD_PORT_DATA	= 0x60,
	KEYBOARD_PORT_STATUS	= 0x64,
	KEYBOARD_PORT_CMD	= 0x64
};

static struct {
	union {
		struct {
			uint8_t	scroll_lock:1;
			uint8_t	num_lock:1;
			uint8_t	caps_lock:1;
		} u;
		uint8_t		value;
	} led;
	struct {
		uint8_t	l_ctrl:1;
		uint8_t	r_ctrl:1;
		uint8_t	l_shift:1;
		uint8_t	r_shift:1;
		uint8_t	l_alt:1;
		uint8_t	r_alt:1;
	} key;
	uint8_t	esc;
	struct circular_buffer	cb;
	uint8_t			buffer[1024];
	pthread_t		waiter;
} keyboard;

static const uint8_t scan_code[3][0x3a] = {
	[0] = {
		0,
		0x1b,
		'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x08,
		'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
		'\n',
		0,
		'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
		'`',
		0,
		'\\',
		'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
		'*',
		0, ' '
	},
	[1] = {
		0,
		0x1b,
		'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0x08,
		'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
		'\n',
		0,
		'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',
		'~',
		0,
		'|',
		'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
		'*',
		0, ' '
	},
	[2] = {
		0,
		0,
		0, '@', 0, '$', 0, 0, '{', '[', ']', '}', '\\', 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '~',
		'\r',
		0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0,
		0, 0
	}
};

typedef void transcode_func_t(uint8_t code);

static void chr(uint8_t code)
{
	if ((code & 0x80) == 0) {
		int i = 2;
		if (!keyboard.key.r_alt)
			i = keyboard.key.l_shift || keyboard.key.r_shift;
		code = scan_code[i][code];
		if (code != 0) {
			if (i != 1 &&
			    (keyboard.led.u.caps_lock ||
			     keyboard.key.l_ctrl ||
			     keyboard.key.r_ctrl) &&
			    islower(code)) {
				code = toupper(code);
			}
			if (keyboard.key.l_ctrl || keyboard.key.r_ctrl) {
				if (code >= '@' && code < '@' + 0x20)
					code -= '@';
				else if (code == '?')
					code = 0x7f;
			}
			if (keyboard.key.l_alt)
				code |= 0x80;
			circular_buffer_write(&keyboard.cb, &code, 1);
		}
	}
}

static void ctrl(uint8_t code)
{
	if (keyboard.esc)
		keyboard.key.r_ctrl = ((code & 0x80) == 0);
	else
		keyboard.key.l_ctrl = ((code & 0x80) == 0);
}

static void l_shift(uint8_t code)
{
	keyboard.key.l_shift = ((code & 0x80) == 0);
}

static void r_shift(uint8_t code)
{
	keyboard.key.r_shift = ((code & 0x80) == 0);
}

static void alt(uint8_t code)
{
	if (keyboard.esc)
		keyboard.key.r_alt = ((code & 0x80) == 0);
	else
		keyboard.key.l_alt = ((code & 0x80) == 0);
}

static inline void keyboard_wait(void)
{
	uint8_t status;

	do {
		status = inb(KEYBOARD_PORT_STATUS);
	} while ((status & 2) != 0);
}

static void keyboard_update_led(void)
{
	keyboard_wait();
	outb(0xed, KEYBOARD_PORT_DATA);
	keyboard_wait();
	outb(keyboard.led.value, KEYBOARD_PORT_DATA);
}

static void caps_lock(uint8_t code)
{
	if ((code & 0x80) == 0) {
		keyboard.led.u.caps_lock = !keyboard.led.u.caps_lock;
		keyboard_update_led();
	}
}

static void num_lock(uint8_t code)
{
	if ((code & 0x80) == 0) {
		keyboard.led.u.num_lock = !keyboard.led.u.num_lock;
		keyboard_update_led();
	}
}

static void scroll_lock(uint8_t code)
{
	if ((code & 0x80) == 0) {
		keyboard.led.u.num_lock = !keyboard.led.u.num_lock;
		keyboard_update_led();
	}
}

static void sysrq(uint8_t code)
{
	(void)code;
	/* TODO */
}

static const uint8_t scan_code_keypad[2][13] = {
	[0] = {
		'H', 'A', '5' | 0x80,
		'-',
		'D', '5', 'C', '+',
		'4' | 0x80, 'B', '6' | 0x80,
		'2' | 0x80, 0x7f,
	},
	[1] = {
		'7', '8', '9',
		'-',
		'4', '5', '6', '+',
		'1', '2', '3',
		'0', '.',
	}
};

static void keypad(uint8_t code)
{
	if (code & 0x80)
		return;
	if (code == 0x53 &&
	    (keyboard.key.l_ctrl || keyboard.key.r_ctrl) &&
	    (keyboard.key.l_alt || keyboard.key.r_alt)) {
	    reboot();
	    return;
	}
	if (keyboard.esc == 0xe0 || !keyboard.led.u.num_lock ||
	    keyboard.key.l_shift || keyboard.key.r_shift) {
		if (code == 0x37) {
			/* TODO: print screen */
			return;
		}
		code = scan_code_keypad[0][code - 0x47];
		if (code & 0x80) {
			if (circular_buffer_space(&keyboard.cb) >= 4) {
				circular_buffer_write(&keyboard.cb, "\x1b[", 2);
				code &= 0x7f;
				circular_buffer_write(&keyboard.cb, &code, 1);
				code = '~';
				circular_buffer_write(&keyboard.cb, &code, 1);
			}
		} else if (isupper(code)) {
			if (circular_buffer_space(&keyboard.cb) >= 3) {
				circular_buffer_write(&keyboard.cb, "\x1b[", 2);
				circular_buffer_write(&keyboard.cb, &code, 1);
			}
		} else {
			circular_buffer_write(&keyboard.cb, &code, 1);
		}
	} else {
		if (code == 0x37)
			code = '*';
		else
			code = scan_code_keypad[1][code - 0x47];
		circular_buffer_write(&keyboard.cb, &code, 1);
	}
}

static void fn(uint8_t code)
{
	if ((code & 0x80) == 0 && circular_buffer_space(&keyboard.cb) >= 4) {
		circular_buffer_write(&keyboard.cb, "\x1b[[", 3);
		code = code - 0x3b + 'A';
		circular_buffer_write(&keyboard.cb, &code, 1);
	}
}

static void chr_keypad(uint8_t code)
{
	if (keyboard.esc == 0xe0) {
		code = '/';
		circular_buffer_write(&keyboard.cb, &code, 1);
	} else {
		chr(code);
	}
}

static transcode_func_t *transcode_func_table[] = {
	/* 00 */ chr,
	/* 01 */ chr,
	/* 02 */ chr, chr, chr, chr, chr, chr, chr, chr, chr, chr, chr, chr, chr,
	/* 0f */ chr, chr, chr, chr, chr, chr, chr, chr, chr, chr, chr, chr, chr,
	/* 1c */ chr,
	/* 1d */ ctrl,
	/* 1e */ chr, chr, chr, chr, chr, chr, chr, chr, chr, chr, chr,
	/* 29 */ chr,
	/* 2a */ l_shift,
	/* 2b */ chr,
	/* 2c */ chr, chr, chr, chr, chr, chr, chr, chr, chr, chr_keypad, r_shift,
	/* 37 */ keypad,
	/* 38 */ alt, chr,
	/* 3a */ caps_lock,
	/* 3b */ fn, fn, fn, fn, fn, fn, fn, fn, fn, fn,
	/* 45 */ num_lock,
	/* 46 */ scroll_lock,
	/* 47 */ keypad, keypad, keypad,
	/* 4a */ keypad,
	/* 4b */ keypad, keypad, keypad, keypad,
	/* 4f */ keypad, keypad, keypad,
	/* 52 */ keypad, keypad,
	/* 54 */ sysrq,
};

void keyboard_handler(struct interrupt_context *ctx)
{
	uint8_t code = inb(KEYBOARD_PORT_DATA);
	uint8_t bare_code = code & 0x7f;

	(void)ctx;
	switch (code) {
	case 0xe0:
	case 0xe1:
		keyboard.esc = code;
		break;
	default:
		if (bare_code < ARRAY_SIZE(transcode_func_table))
			transcode_func_table[bare_code](code);
		keyboard.esc = 0;
		if (keyboard.waiter)
			wake_up(keyboard.waiter);
		break;
	}
}

int getchar(void)
{
	uint8_t code;
	unsigned long flags = interrupt_disable();

	while (circular_buffer_read(&keyboard.cb, &code, 1) <= 0) {
		assert(keyboard.waiter == NULL);
		keyboard.waiter = pthread_current;
		pthread_current->state = PTHREAD_STATE_SLEEPING;
		schedule();
		keyboard.waiter = NULL;
	}
	interrupt_enable(flags);

	return code;
}

void keyboard_init(void)
{
	circular_buffer_init(&keyboard.cb, keyboard.buffer,
			     sizeof(keyboard.buffer));
	interrupt_register(KEYBOARD_IRQ, keyboard_handler);
	pic_enable(KEYBOARD_IRQ);
}

void reboot(void)
{
	printf("Rebooting...\n");
	keyboard_wait();
	outb(0xfe, KEYBOARD_PORT_CMD);
}
