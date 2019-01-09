/*
  Copyright (c) 2018-2019 Tom Hancocks
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
 */

#include <format.h>
#include <vargs.h>
#include <string.h>

enum token_flags
{
	flag_left_justified = 1 << 1,
	flag_force_sign = 1 << 2,
	flag_alternate_form = 1 << 3,
	flag_zero_pad = 1 << 4,
};

enum token_modifiers
{
	mod_int = 0,
	mod_short = 1,
	mod_long = 2,
	mod_long_double = 3,
	mod_long_long = 4,
};

enum token_type
{
	type_sint = 1,
	type_oct = 2,
	type_unsigned = 3,
	type_lohex = 4,
	type_uphex = 5,
	type_double = 6,
	type_exp = 7,
	type_char = 8,
	type_str = 9,
	type_ptr = 10,
};

void write_format_va(
	struct format_info *info, 
	const char *restrict fmt, 
	va_list va
) {
	/* If we do not have a means of producing output, then simply return now
	   and avoid wasting processing time. */
	if (info == NULL || info->out == NULL)
		return;

	char *read = fmt;
	char *write = info->buffer;

	uint32_t n = BUFFER_SIZE;

	while (*read && n > 0) {
		/* The first step is to check if this is a format. If not, then just
		   echo the character straight out to the buffer. If it is escaped then
		   echo it out. */
		if (*read != '%') {
			*write++ = *read++;
			--n;
			continue;
		}
		read++;

		if (*read == '%') {
			*write++ = *read++;
			--n;
			continue;
		}

		/* We're looking at a token. We now have to parse the token to try and
		   understand what we're being asked to display.
		   The following reference:
		      https://www-s.acm.illinois.edu/webmonkeys/book/c_guide/2.12.html
		   has been used for this format.
		
		   %[flags][width][.precision][modifier][type]
		*/

		/* [flags] */
		enum token_flags flags = 0;
		while (*read == '+' || *read == '-' || *read == '#' || *read == '0') {
			switch (*read++) {
				case '+': /* Force Sign */
					flags |= flag_force_sign;
					break;
				case '-': /* Left Justify */
					flags |= flag_left_justified;
					break;
				case '#': /* Alternate Form */
					flags |= flag_alternate_form;
					break;
				case '0': /* Zero padded */
					flags |= flag_zero_pad;
					break;
			}
		}

		/* [width] */
		/*	  -1 represents a '*' being specified, and thus comes from an arg */
		int width = 0;
		while (width >= 0 && (*read == '*' || (*read >= '0' && *read <= '9'))) {
			switch (*read) {
				case '*': /* Width is specified with an argument */
					width = -1;
					break;
				default:
					width *= 10;
					width += (*read - '0');
					break;
			}
			++read;
		}

		/* [.precision] */
		/*   -1 represents a '*' being specified, and thus comes from an arg
		     -2 represents the default precision
		     0 represents no precision */
		int precision = -2;
		if (*read == '.') {
			++read;
			while (*read == '*' || (*read >= '0' && *read <= '9')) {
				switch (*read) {
					case '*': /* Precision is specified width an argument. */
						precision = -1;
						break;
					default:
						precision *= 10;
						precision += (*read - '0');
						break;
				}
				++read;
				if (precision < 0) {
					break;
				}
			}
		}

		/* [modifier] */
		enum token_modifiers mods = mod_int;
		while (*read == 'h' || *read == 'l' || *read == 'L') {
			switch (*read++) {
				case 'h': /* Short */
					mods = mod_short;
					break;
				case 'l': /* Long / Long Long */
					if (mods == mod_long) {
						mods |= mod_long_long;
					}
					else if (mods != mod_long_long) {
						mods |= mod_long;
					}
					break;
				case 'L': /* Long Double */
					mods |= mod_long_double;
				default:
					break;
			}
		}

		/* [type] */
		enum token_type type = 0;
		switch (*read++) {
			case 'd':
			case 'i': /* signed int */
				type = type_sint;
				break;
			case 'o': /* unsigned int (octal) */
				type = type_oct;
				break;
			case 'u': /* unsigned int */
				type = type_unsigned;
				break;
			case 'x': /* unsigned int (lower hex) */
				type = type_lohex;
				break;
			case 'X': /* unsigned int (upper hex) */
				type = type_uphex;
				break;
			case 'f': /* double */
				type = type_double;
				break;
			case 'c': /* char */
				type = type_char;
				break;
			case 's': /* string */
				type = type_str;
				break;
			case 'p': /* pointer */
				type = type_ptr;
				break;
		}

		/* Special types can be translated to another, such as pointers */
		uint8_t base = 10;

		if (type == type_ptr) {
			type = type_uphex;
			#if __i386__
			mods = mod_long;
			width = 8;
			#endif
			flags |= flag_zero_pad | flag_alternate_form;
		}

		if (type == type_lohex || type == type_uphex) {
			base = 16 | (type == type_uphex ? 0x80 : 0x00);
			type = type_unsigned;
		}
		else if (type == type_oct) {
			base = 8;
			type = type_unsigned;
		}

		if (mods == mod_short) {
			mods &= ~mod_short;
			if (type != type_unsigned || type != type_sint) {
				type = type_sint;
			}
		}

		/* At this point the token itself is parsed, and can be rendered. This
		   is slightly complex task that needs to be carried out in a number
		   of stages in order to be performed correctly. */
		char tmp_buffer[INTERNAL_BUFFER_SIZE] = { 0 };
		char *tmp_ptr = tmp_buffer + INTERNAL_BUFFER_SIZE - 2;
		uint32_t len = 0;

		if (type == type_str) {
			tmp_ptr = (void *)va_arg(va, uintptr_t);
			uint32_t str_len = strlen(tmp_ptr);
			goto WRITE_TMP_BUFFER;
		}
		else if (type == type_char) {
			char c = (signed char)va_arg(va, signed int);
			*tmp_ptr = c;
			goto WRITE_TMP_BUFFER;
		}
		else if (mods == mod_int) {
			if (type == type_unsigned) {
				unsigned int ui = (unsigned int)va_arg(va, unsigned int);
				len = utoa_base(tmp_ptr, ui, base);
				tmp_ptr -= len;
			}
			else {
				signed int si = (signed int)va_arg(va, signed int);
				len = itoa_base(tmp_ptr, si, base);
				tmp_ptr -= len;
			}
		}
		else if (mods == mod_long) {
			if (type == type_unsigned) {
				unsigned long ul = (unsigned long)va_arg(va, unsigned long);
				len = ultoa_base(tmp_ptr, ul, base);
				tmp_ptr -= len;
			}
			else {
				signed long sl = (signed long)va_arg(va, signed long);
				len = ltoa_base(tmp_ptr, sl, base);
				tmp_ptr -= len;
			}
		}
		else if (mods == mod_long_long) {
			if (type == type_unsigned) {
				unsigned long long ull = (unsigned long long)va_arg(
					va, unsigned long long
				);
				len = ulltoa_base(tmp_ptr, ull, base);
				tmp_ptr -= len;
			}
			else {
				signed long long sll = (signed long long)va_arg(
					va, signed long long
				);
				len = lltoa_base(tmp_ptr, sll, base);
				tmp_ptr -= len;
			}
		}
		else if (mods == mod_long_double) {
			/* TODO */
		}

	ALIGN_AND_PAD:
		if (width > len) {
			for (; len < width; ++len) {
				*tmp_ptr-- = flags & flag_zero_pad ? '0' : ' ';
			}
		}

		if (flags & flag_alternate_form) {
			if ((base & 0x7F) == 16) {
				*tmp_ptr-- = 'x';
				*tmp_ptr-- = '0';
			}
			else if (base == 8) {
				*tmp_ptr-- = 'O';
			}
		}

		++tmp_ptr;

	WRITE_TMP_BUFFER:
		while (n-- && *tmp_ptr) {
			*write++ = *tmp_ptr++;
		}
	}

	/* Ensure NUL termination. */
	*write++ = 0;

	/* Send the buffer to the output function. */
	info->out(info->buffer);
}

void write_format(struct format_info *info, const char *restrict fmt,...) 
{
	va_list va;
	va_start(va, fmt);
	write_format_va(info, fmt, va);
	va_end(va);
}