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

#if !defined(THREAD_H)
#define THREAD_H

#include <types.h>

#define MAX_THREADS		0x400	/* 1024 Threads is the maximum allowed. */

enum thread_state
{
	/* Thread is running normally */
	thread_running = 0,

	/* Thread must not be interrupted */
	thread_no_interrupt = (1 << 0),

	/* Thread has been terminated and not cleaned up yet. */
	thread_terminated = (1 << 1),

	/* Thread has exited. */
	thread_exited = (1 << 2),

	/* Thread is suspended, waiting to be resumed */
	thread_suspended = (1 << 3),

	/* Thread is suspended, but waiting for a timer to ellapse. */
	thread_sleeping = (1 << 4),

	/* Thread is suspended, but waiting for an IRQ to fire. */
	thread_irq = (1 << 5),

	/* Thread is suspended, but waiting for keyboard input */
	thread_keyboard = (1 << 6),
};

/**
 Thread structure representing all aspects of a thread, and keeping track of its
 state.
 */
struct thread
{
	uint32_t tid;
	enum thread_state state;
	void *owner;
	void *stack;
	void *stack_base;
	uint32_t stack_size;
	int(*start)(void);
	struct thread *next;
	uint64_t start_time;
	uint64_t run_time;
	uint64_t idle_time;
	uint64_t resumed_time;
	uint64_t suspended_time;
	uint64_t wake_time;
	uint64_t suspend_time;
} __attribute__((packed));

extern struct thread *kernel_main_thread;

/**
 Setup the threading environment, creating the "Main Kernel Thread" in the 
 process.
 */
oserr init_threading(void);

/**
 Create a new thread, with the specified starting point.
 */
struct thread *thread_create(int(*start)(void));

/**
 Yield the execution of the current thread. The current stack information should
 be provided so that it can be save for later.
 */
void thread_yield(uintptr_t stack_ptr, uintptr_t stack_base, uint8_t irq);

/**
 Switch the current thread stack.
 */
void switch_thread(void *stack_ptr, void *stack_base);

void thread_sleep(uint64_t ms);
void thread_wait_keyboard(void);

#endif