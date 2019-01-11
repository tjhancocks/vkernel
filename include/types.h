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

#if !defined(TYPES_H)
#define TYPES_H
	
	/* Standard bit width definitions that remain constant between 
	   architectures. */
	typedef unsigned char                __kernel_uint8_t;
	typedef   signed char                __kernel_int8_t;
	typedef unsigned short               __kernel_uint16_t;
	typedef   signed short               __kernel_int16_t;
	typedef unsigned int                 __kernel_uint32_t;
	typedef   signed int                 __kernel_int32_t;
	typedef unsigned long long           __kernel_uint64_t;
	typedef   signed long long           __kernel_int64_t;
	
	/* Architecutre dependant types. */
#	if __i386__
		typedef __kernel_uint32_t        __kernel_unative_t;
		typedef __kernel_int32_t         __kernel_native_t;
#	elif __x86_64__
		typedef __kernel_uint64_t        __kernel_unative_t;
		typedef __kernel_int64_t         __kernel_native_t;
#	endif
	
	/* Integer types for representing pointers. */
	typedef __kernel_unative_t           __kernel_uintptr_t;
	typedef __kernel_native_t            __kernel_intptr_t;

	/* Useful error types and return codes. */
	typedef __kernel_native_t            oserr;

	/* Common oserr return codes */
	enum { e_fail = 0, e_ok = 1 };
	
	/* In the event that no Standard C library is being included into the
	   kernel, make sure we have some common integer definitions in place. */
#	if !defined(INC_LIBK) || !defined(INC_LIBC)
		typedef __kernel_uint8_t         uint8_t;
		typedef __kernel_int8_t          int8_t;
		typedef __kernel_uint16_t        uint16_t;
		typedef __kernel_int16_t         int16_t;
		typedef __kernel_uint32_t        uint32_t;
		typedef __kernel_int32_t         int32_t;
		typedef __kernel_uint64_t        uint64_t;
		typedef __kernel_int64_t         int64_t;
		typedef __kernel_unative_t       unative_t;
		typedef __kernel_native_t        native_t;
		typedef __kernel_uintptr_t       uintptr_t;
		typedef __kernel_intptr_t        intptr_t;

#		if !defined(NULL)
#			define NULL                  ((void *)0)
#		endif

#		if !defined(MAX)
#			define MAX(a, b)			(((a) > (b)) ? (a) : (b))
#		endif

#		if !defined(MIN)
#			define MIN(a, b)			(((a) < (b)) ? (a) : (b))
#		endif

#		define bool						_Bool
#		define true						1
#		define false					0
		
#	endif

#endif