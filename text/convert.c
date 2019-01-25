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

#include <string.h>
#include <types.h>

#define BASE_UPPER   "0123456789ABCDEF"
#define BASE_LOWER   "0123456789abcdef"
#define IS_UPPER(_v) ((_v) & 0x80)
#define BASE(_v)     ((_v) & 0x7F)
#define DIGITS(_v)   (IS_UPPER(_v) ? BASE_UPPER : BASE_LOWER)

uint32_t ulltoa_base(char *ptr, unsigned long long n, uint8_t base)
{
	const char *digits = DIGITS(base);
	base = BASE(base);

	if (n == 0) {
		*ptr = *digits;
		return 1;
	}

	register unsigned long long v = n;
	register unsigned long long dV = 0;
	register uint32_t len = 1;

	while (v >= (unsigned long long)base) {
		dV = v % base;
		v /= base;
		*ptr-- = *(digits + dV);
		++len;
	}

	if (v > 0) {
		dV = v % base;
		*ptr-- = *(digits + dV);
		++len;
	}

	return (len - 1);
}

uint32_t lltoa_base(char *ptr, signed long long n, uint8_t base)
{
	const char *digits = DIGITS(base);
	base = BASE(base);

	if (n == 0) {
		*ptr = *digits;
		return 1;
	}

	char neg = (n < 0);
	register long long v = n * (neg ? -1 : 1);
	register long long dV = 0;
	register uint32_t len = 1;

	while (v >= (long long)base) {
		dV = v % base;
		v /= base;
		*ptr-- = *(digits + dV);
		++len;
	}

	if (v > 0) {
		dV = v % base;
		*ptr-- = *(digits + dV);
		++len;
	}

	if (neg) {
		*ptr-- = '-';
		++len;
	}

	return (len - 1);
}

uint32_t ftoa(char *ptr, double v, int precision)
{
	/* TODO: This implementation could be better! It currently has bugs and 
	   doesn't handle all floating point conversion cases. */
	
	uint32_t len = 0;

	/* Handle the precision first */
	if (precision > 0) {
		double dV = v - (long long)v;

		while ((long long)dV != dV) {
			dV *= 10;
		}

		char precision_buffer[32] = { 0 };
		char *precision_ptr = precision_buffer + 30;
		uint32_t precision_len = lltoa_base(precision_ptr, (long long)dV, 10);
		precision_ptr -= precision_len;

		ptr -= precision;
		for (int i = 0; i < precision; ++i) {
			if (i < precision_len) {
				*ptr++ = *precision_ptr++;
				len++;
			}
			else {
				*ptr++ = '0';
				len++;
			}
		}
		ptr -= precision;

		++len;
		*ptr-- = '.';
	}

	/* Render the integer aspect last */
	len += lltoa_base(ptr, (long long)v, 10);
	
	/* Finish up */
	return len;
}

int atoi(const char *restrict str)
{
	int result = 0;
	while (*str) {
		if (*str >= '0' && *str <= '9') {
			result *= 10;
			result += (*str - '0');
		}
		else {
			return 0;
		}
		++str;
	}
	return result;
}