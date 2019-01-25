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

#include <syscall.h>
#include <vargs.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

void *__syscall_kernel_info(int);

////////////////////////////////////////////////////////////////////////////////

void *__invoke_syscall(int syscall, va_list va)
{
	switch (syscall) {
		case syscall_kernel_info:
			return __syscall_kernel_info(atoi((void *)va_arg(va, void *)));
		default:
			panic(
				"Illegal Operation!", 
				"An invalid syscall (%d) was used.",
				syscall
			);
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////

void *__internal_syscall(int syscall,...)
{
	va_list va;
	va_start(va, syscall);

	/* Invoke the system call */
	void *result = __invoke_syscall(syscall, va);
	va_end(va);
	
	/* Finish up */
	return result;
}