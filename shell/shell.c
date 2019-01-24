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
#include <alloc.h>
#include <ramdisk.h>

////////////////////////////////////////////////////////////////////////////////

#define BUFFER_LEN	1024

static struct thread *kernel_shell_thread = NULL;

static void ksh_parse_command(const char *restrict buffer, bool *exit);
static void ksh_handle_command(uint8_t argc, const char **argv, bool *exit);
static void ksh_run_script(const char *restrict script, bool *exit);

////////////////////////////////////////////////////////////////////////////////

static inline bool isspace(char c)
{
	return (c == ' ' || c == '\t');
}

static void ksh_run_script(const char *restrict script, bool *exit)
{
	char statement[BUFFER_LEN] = { 0 };
	uint32_t ptr = 0;

	while (*script) {
		if (*script == '\n') {
			statement[ptr] = '\0';
			ksh_parse_command(statement, exit);
			memset(statement, '\0', BUFFER_LEN);
			ptr = 0;
		} 
		else if (ptr < BUFFER_LEN) {
			statement[ptr++] = *script;
		}
		++script;
	}
}

static void ksh_handle_command(uint8_t argc, const char **argv, bool *exit)
{
	if (strcmp(argv[0], "exit") == 0) {
		*exit = true;
	}
	else if (strcmp(argv[0], "syscall") == 0) {
		/* system call - requires at least 1 argument (argc >= 2) */
		if (argc >= 2) {
			/* todo */
		}
		else {
			kprint("Insufficient arguments provided.\n");
			kprint("  syscall [function_number] [...]\n");
		}
	}
	else if (strcmp(argv[0], "echo") == 0) {
		for (uint8_t i = 1; i < argc; ++i) {
			kprint("%s\n", argv[i]);
		}
	}
	else {
		char *script = ramdisk_open(&system_ramdisk, argv[0], NULL);
		if (script) {
			ksh_run_script(script, exit);
			kfree(script);
		}
		else {
			kprint("Unrecognised command '%s'\n", argv[0]);
		}
	}
}

static void ksh_parse_command(const char *restrict buffer, bool *exit)
{
	/* parse the command into its components. we can then see if there are
	   any commands built into the ramdisk for use. */
	char *ptr = buffer;
	char *tail = ptr + strlen(ptr) - 1;
	while (isspace(*ptr)) ++ptr;
	while (isspace(*tail)) *tail-- = '\0';

	/* Is the command a comment, or an empty line */
	if (*ptr == '#' || strlen(buffer) == 0) return;

	/* set up storage for the arguments - 16 arguments maximum */
	char *argv[16] = { NULL };
	uint8_t argc = 0;

	uint32_t ptr_len = strlen(ptr);
	uint32_t start = 0;
	uint32_t length = 0;
	bool escaped = false;
	bool in_string = false;

	for (uint32_t i = 0; i < ptr_len + 1; ++i) {
		char c = ptr[i];

		if ((!escaped && !in_string && isspace(c)) || c == '\0') {
			if ((length = i - start) > 0) {
				if (length >= 2 && ptr[start] == '"') {
					++start;
				}
				if (length >= 2 && ptr[start + length - 2] == '"') {
					length -= 2;
				}

				char *token = memcpy(kalloc(length + 1), ptr + start, length);
				token[length] = '\0';
				argv[argc++] = token;
			}

			start = i + 1;
		}
		else if (!escaped && c == '"' && !in_string) {
			in_string = true;
			start = i;
		}
		else if (!escaped && c == '"' && in_string) {
			in_string = false;
		}
		else if (c == '\\') {
			escaped = true;
			continue;
		}

		escaped = false;

		/* we reached the maximum arguments? */
		if (argc >= 16) {
			kprint("Too many arguments!\n");
			break;
		}
	}

	/* attempt to handle the command */
	ksh_handle_command(argc, argv, exit);

	/* clean up */
	for (uint8_t i = 0; i < argc; ++i) {
		kfree(argv[i]);
	}
}

////////////////////////////////////////////////////////////////////////////////

int kernel_shell_main(void)
{
	bool should_exit = false;
	while (!should_exit) {
		char buffer[BUFFER_LEN] = { 0 };
		kprint("# ");

		/* read input from the user and handle it */
		if (readline(&buffer, BUFFER_LEN) > 0) {
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