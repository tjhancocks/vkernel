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

#if !defined(ARCH_H)
#define ARCH_H

#include <types.h>

#if __i386__
#	include <arch/intel/intel.h>
#endif

/**
 When called, the CPU will be put in a "hung" state, frozen and unresponsive.
 This should only be used to halt execution of the system in the event of an
 unknown or unstable state being reached.
 */
void hang_forever(void);

/**
 When called, the CPU will be put in a "waiting" state until it receives an
 interrupt, at which point it will resume execution.
 This is useful for energy saving.
 */
void hang(void);

/**
 Initialise the current architecture of the system. This will invoke the correct
 initialisation calls for the architecture of the host system, resulting in all
 architecture specific aspects being ready for use.
 */
void init_arch(void);

#endif