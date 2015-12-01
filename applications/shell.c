/*
 * Copyright (c) 2015 Changli Gao <xiaosuo@gmail.com>
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <application.h>
#include <pthread.h>
#include <shell.h>
#include <timer.h>

#include <readline/readline.h>

#ifndef CONFIG_PROMPT
#define CONFIG_PROMPT "turnix # "
#endif

#ifndef CONFIG_MOTD
#define CONFIG_MOTD "\nTurnix Copyright(C) 2015 Changli Gao <xiaosuo@gmail.com>\n\n"
#endif

enum {
	SHELL_ARGS_MAX_NUM = 20
};

static bool is_exit = false;
extern const struct shell_cmd shell_cmd_begin[];
extern const struct shell_cmd shell_cmd_end[];
static unsigned long stack[256];

static int cmd_list(void)
{
	const struct shell_cmd *cmd;

	for (cmd = shell_cmd_begin; cmd < shell_cmd_end; ++cmd)
		printf("%s: %s\n", cmd->exe, cmd->usage ? : cmd->exe);

	return 0;
}

static const struct shell_cmd *cmd_find(const char *exe)
{
	const struct shell_cmd *cmd;

	for (cmd = shell_cmd_begin; cmd < shell_cmd_end; ++cmd) {
		if (strcmp(cmd->exe, exe) == 0)
			return cmd;
	}

	return NULL;
}

static int cmd_exec(int argc, char *argv[])
{
	const struct shell_cmd *cmd;

	cmd = cmd_find(argv[0]);
	if (!cmd) {
		printf("no such command: %s\n", argv[0]);
		return -1;
	}

	return cmd->handler(argc, argv);
}

static int cmd_parse(char *buf, char *argv[], int argv_max)
{
	char *saveptr;
	int argc = 0;

	for (argc = 0; argc < argv_max; ++argc) {
	       argv[argc] = strtok_r(buf, " \t\r\n", &saveptr);
	       if (!argv[argc])
		       break;
	       buf = NULL;
	}
	if (argc >= argv_max) {
		printf("invalid command line\n");
		return -1;
	}

	return argc;
}

static void loop()
{
	char *line;
	char *argv[SHELL_ARGS_MAX_NUM];
	int argc;

	while (!is_exit) {
		line = readline(CONFIG_PROMPT);
		argc = cmd_parse(line, argv, SHELL_ARGS_MAX_NUM);
		if (argc <= 0)
			continue;
		cmd_exec(argc, argv);
	}
}

static int do_exit(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	is_exit = true;

	return 0;
}

static __shell_cmd struct shell_cmd cmd_exit = {
	.exe		= "exit",
	.handler	= do_exit,
};

static int do_help(int argc, char *argv[])
{
	if (argc == 1) {
		return cmd_list();
	} else {
		const struct shell_cmd *cmd;
		int i;

		for (i = 1; i < argc; ++i) {
			cmd = cmd_find(argv[i]);
			if (!cmd) {
				printf("no such command: %s\n", argv[i]);
			} else {
				printf("%s: %s\n", cmd->exe,
				       cmd->usage ? : cmd->exe);
			}
		}
	}

	return 0;
}

static __shell_cmd struct shell_cmd cmd_help = {
	.exe		= "help",
	.handler	= do_help,
	.usage		= "help [cmd]..."
};

static int do_reboot(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	reboot();

	return 0;
}

static __shell_cmd struct shell_cmd cmd_reboot = {
	.exe		= "reboot",
	.handler	= do_reboot,
};

static int do_date(int argc, char *argv[])
{
	struct tm tm;
	struct timeval tv;
	struct timezone tz;

	(void)argc;
	(void)argv;
	gettimeofday(&tv, &tz);
	localtime_r(&tv.tv_sec, &tm);
	printf("%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d\n",
	       tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
	       tm.tm_hour, tm.tm_min, tm.tm_sec,
	       tz.tz_minuteswest > 0 ? '-' : '+',
	       abs(tz.tz_minuteswest) / 60, abs(tz.tz_minuteswest) % 60);

	return 0;
}

static __shell_cmd struct shell_cmd cmd_date = {
	.exe		= "date",
	.handler	= do_date,
};

static int do_uptime(int argc, char *argv[])
{
	struct timeval tv;
	struct tm tm;

	(void)argc;
	(void)argv;

	uptime(&tv);
	if (tv.tv_sec >= SECS_PER_DAY) {
		printf("%ld days, ", tv.tv_sec / SECS_PER_DAY);
		tv.tv_sec %= SECS_PER_DAY;
	}
	gmtime_r(&tv.tv_sec, &tm);
	printf("%02d:%02d:%02d.%06ld\n", tm.tm_hour, tm.tm_min, tm.tm_sec,
	       tv.tv_usec);

	return 0;
}

static __shell_cmd struct shell_cmd cmd_uptime = {
	.exe		= "uptime",
	.handler	= do_uptime,
};

static int do_cal(int argc, char *argv[])
{
	struct tm tm;
	time_t now = time(NULL);
	static const char * const mon_str[] = {
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	};
	static const int const days_of_mon[] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};
	int i, wday, day, days;

	(void)argc;
	(void)argv;

	localtime_r(&now, &tm);
	printf("   %s %d\n", mon_str[tm.tm_mon], 1900 + tm.tm_year);
	printf("Su Mo Tu We Th Fr Sa\n");
	wday = (tm.tm_wday + 35 - (tm.tm_mday - 1)) % 7;
	for (i = 0; i < wday; ++i)
		printf("   ");
	days = days_of_mon[tm.tm_mon] +
		(tm.tm_mon == 1 && is_leapyear(1900 + tm.tm_year));
	for (day = 1; day <= days; ++day) {
		printf("%2d ", day);
		if (wday == 6) {
			putchar('\n');
			wday = 0;
		} else {
			++wday;
		}
	}
	if (wday != 0)
		putchar('\n');

	return 0;
}

static __shell_cmd struct shell_cmd cmd_cal = {
	.exe		= "cal",
	.handler	= do_cal,
};

static void show_thread(pthread_t th)
{
	static const char * const state_str[] = {
		[PTHREAD_STATE_NONE]		= "none",
		[PTHREAD_STATE_INIT]		= "init",
		[PTHREAD_STATE_RUNNING]		= "running",
		[PTHREAD_STATE_SLEEPING]	= "sleeping",
		[PTHREAD_STATE_EXIT]		= "exit",
	};

	printf("%p %s %s %lu.%03lu\n", th, state_str[th->state], th->name,
	       th->stime.tv_sec, th->stime.tv_usec / USECS_PER_MSEC);
}

static int do_ps(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	pthread_foreach(show_thread);

	return 0;
}

static __shell_cmd struct shell_cmd cmd_ps = {
	.exe		= "ps",
	.handler	= do_ps,
};

static void *shell(void *args)
{
	(void)args;
	for (;;) {
		is_exit = false;

		printf("%s", CONFIG_MOTD);
		loop();
	}

	return NULL;
}

static void shell_init(void)
{
	struct sched_param sched_param;
	pthread_attr_t attr;
	pthread_t tid;

	pthread_attr_init(&attr);
	pthread_attr_setstack(&attr, stack, sizeof(stack));
	sched_param.sched_priority = sched_get_priority_max(SCHED_RR);
	pthread_attr_setschedparam(&attr, &sched_param);
	pthread_create(&tid, &attr, shell, NULL);
	pthread_setname_np(tid, "shell");
	pthread_detach(tid);
	pthread_attr_destroy(&attr);
}
application_init(shell_init);
