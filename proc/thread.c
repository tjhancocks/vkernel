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

#include <thread.h>
#include <panic.h>
#include <print.h>
#include <stack.h>
#include <heap.h>
#include <arch.h>
#include <time.h>

////////////////////////////////////////////////////////////////////////////////

#define INITIAL_TID			1000
#define KERNEL_TID 			0

#define THREAD_STACK_SIZE	0x10000

#define THREAD_RUN_QUANTA	50

extern const void *kernel_stack;

static struct thread _kernel_main = {
	.tid = KERNEL_TID,
	.state = thread_running,
	.owner = NULL,
	.stack_base = &kernel_stack + 0x10000,
	.stack_size = 0x10000, /* 16KiB */
};

struct thread *kernel_main_thread = &_kernel_main;
static struct thread *_current_thread;
static uint32_t next_tid = INITIAL_TID;

////////////////////////////////////////////////////////////////////////////////

__attribute__((noreturn)) void __thread_start()
{
	/* We need to setup the thread and manage its entry and exit. */
	if (_current_thread == NULL || _current_thread->start == NULL) {
		panic(
			"Invalid Thread",
			"Attempted to start an invalid thread, which was either NULL or "
			"had no starting point.\n"
		);
	}

	klogc(sinfo, "Starting thread %d execution now.\n", _current_thread->tid);

	/* Setup time information for the thread. */
	uint64_t time = uptime_ms();
	_current_thread->start_time = time;
	_current_thread->resumed_time = time;
	_current_thread->suspend_time = time + THREAD_RUN_QUANTA;

	/* Call the threads main function. */
	int result = _current_thread->start();
	time = uptime_ms();

	/* Calculate the total runtime of the thread. This is how much time it
	   was actually the active thread. */
	uint64_t total_run = (
		(time - _current_thread->resumed_time) + _current_thread->run_time
	);

	klogc(sinfo, "Thread %d returned with status code %d. It ran for %llums\n", 
		_current_thread->tid, result, total_run);

	/* Mark the thread as finished so that it can be cleaned up, and then
	   hang. We can not actually return from here, as there is no where to
	   go! */
	_current_thread->state = thread_exited;

	while (true)
		hang();
}

////////////////////////////////////////////////////////////////////////////////

oserr init_threading(void)
{
	/* We're setting up a threading environment. We need to make sure we have
	   the main kernel thread setup. This will essentially be adopting the 
	   current stack. */
	kernel_main_thread->next = kernel_main_thread;
	_current_thread = kernel_main_thread;

	return e_ok;
}

////////////////////////////////////////////////////////////////////////////////

struct thread *thread_create(int(*start)(void))
{
	/* Setup the new thread instance */
	struct thread *thread = kalloc(sizeof(*thread));
	thread->tid = next_tid++;
	thread->start = start;
	thread->state = thread_running;

	/* Setup the thread stack. */
	thread->stack = kalloc(THREAD_STACK_SIZE);
	thread->stack_base = thread->stack + THREAD_STACK_SIZE;
	if (init_stack(thread->stack_base, start, &thread->stack) != e_ok) {
		klogc(swarn, "Failed to setup thread stack correctly.\n");
	}

	/* Insert the thread into the loop */
	thread->next = _current_thread->next;
	_current_thread->next = thread;

	return thread;
}

////////////////////////////////////////////////////////////////////////////////

__attribute__((noreturn)) void thread_yield(
	uintptr_t stack_ptr, uintptr_t stack_base, uint8_t irq
) {
	/* Check that the current thread is happy to yield. */
	/* TODO: This needs to be safe guarded so it can't be abused! */
	if (_current_thread->state & thread_no_interrupt)
		return;

	/* Are we ready to yield? The thread is allowed at least a certain amount
	   of time. */
	uint64_t time = uptime_ms();
	if (_current_thread->state & (thread_sleeping | thread_irq) == 0) {
		if (_current_thread->suspend_time > time)
		return;	
	}
	
	/* If the next thread is the current task then abort. Nothing will happen */
	if (_current_thread->next == _current_thread)
		return;

	/* Update the current thread before it's switched out. */
	_current_thread->stack = (void *)stack_ptr;
	_current_thread->stack_base = (void *)stack_base;

	/* Find the next available thread. If we find the current thread again, then
	   abort. */
	struct thread *thread = _current_thread;
	while ((thread = thread->next) != _current_thread) {
		/* Check for a timer condition on the thread. */
		if (thread->state & thread_sleeping) {
			if (thread->wake_time <= time) {
				/* Thread is ready to be resumed. Remove sleep state. */
				thread->state &= ~thread_sleeping;
			}
		}

		if (thread->state == thread_running) {
			/* The thread is ready to run! */
			break;
		}
	}

	/* Double check to make sure we're not attempting to switch to the current
	   thread. */
	if (_current_thread == thread)
		return;

	/* Update the times of the outgoing thread */
	_current_thread->run_time += time - _current_thread->resumed_time;
	_current_thread->suspended_time = time;

	/* Update the times of the incoming thread */
	_current_thread = thread;
	_current_thread->resumed_time = time;
	_current_thread->idle_time += time - _current_thread->suspended_time;

	/* Perform the switch. */
	switch_thread(_current_thread->stack, _current_thread->stack_base);
}

////////////////////////////////////////////////////////////////////////////////

void thread_sleep(uint64_t ms)
{
	_current_thread->wake_time = uptime_ms() + ms;
	_current_thread->state |= thread_sleeping;

	while (_current_thread->state & thread_sleeping)
		hang();
}