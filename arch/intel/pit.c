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

#if (__i386__ || __x86_64__)

#include <arch/intel/intel.h>
#include <print.h>

////////////////////////////////////////////////////////////////////////////////

static struct {
	uint32_t phase;
	uint64_t ticks;
	uint64_t subticks;
} pit;

////////////////////////////////////////////////////////////////////////////////

static inline void pit_set_frequency(uint32_t f)
{
	int32_t div = 1193180 / f;
	pit.phase = f;	
	klogc(sinfo, "Setting PIT Frequency to %uHz\n", f);
	outb(0x43, 0x36);
	outb(0x40, div & 0xFF);
	outb(0x40, (div >> 8) & 0xFF);
}

static inline void pit_wait(uint32_t ms)
{
	uint64_t ticks = pit_total_ms() + ms;
	while (pit_total_ms() < ticks) {
		nop();
	}
}

static inline void pit_tone_on(uint32_t f)
{
	int32_t div = 1193180 / f;
	outb(0x43, 0xB6);
	outb(0x42, div & 0xFF);
	outb(0x42, (div >> 8) & 0xFF);

	uint8_t t = inb(0x61);
	if (t != (t|3)) {
		outb(0x61, t|3);
	}
}

static inline void pit_tone_off(void)
{
	outb(0x61, inb(0x61) & 0xfc);	
}

////////////////////////////////////////////////////////////////////////////////

static void pit_interrupt(uint8_t irq __attribute__((unused)))
{
	if (++pit.subticks % pit.phase == 0) {
		++pit.ticks;
		pit.subticks = 0;
	}
}

void init_pit(void)
{
	pit_set_frequency(1000);
	set_irq_handler(0x20, pit_interrupt);
}

////////////////////////////////////////////////////////////////////////////////

void beep(void)
{
	pit_tone_on(1000);
	pit_wait(100);
	pit_tone_off();
	pit_set_frequency(pit.phase);
}

////////////////////////////////////////////////////////////////////////////////

uint64_t pit_total_ms(void)
{
	return (pit.ticks * 1000) + ((1000 / pit.phase) * pit.subticks);
}

#endif