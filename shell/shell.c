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

#include <shell.h>
#include <thread.h>
#include <print.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

static struct thread *kernel_shell_thread = NULL;

////////////////////////////////////////////////////////////////////////////////

static void ksh_parse_command(const char *restrict buffer, bool *exit)
{
	if (strcmp(buffer, "exit") == 0) {
		*exit = true;
	}
	else if (strcmp(buffer, "uname") == 0) {
		kprint("vkernel\n");
	}
	else {
		kprint("Unrecognised command '%s'\n", buffer);
	}
}

////////////////////////////////////////////////////////////////////////////////

int kernel_shell_main(void)
{
	bool should_exit = false;
	while (!should_exit) {
		char buffer[1024] = { 0 };
		kprint("# ");

		/* read input from the user and handle it */
		if (readline(&buffer, 1024) > 0) {
			kprint("\n");
			ksh_parse_command(buffer, &should_exit);
		}
		else {
			kprint("\n");
		}
	}

	kprint("Kernel Shell is exiting.\n");

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void launch_kernel_shell(void)
{
	kernel_shell_thread = thread_create(kernel_shell_main);
}