/* $OpenBSD: command.c,v 1.2 2012/07/10 08:16:27 nicm Exp $ */

/*
 * Copyright (c) 2012 Nicholas Marriott <nicm@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <event.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "cu.h"

void	pipe_command(void);
void	send_file(void);

void
pipe_command(void)
{
	const char	*cmd;
	pid_t		 pid;
	int		 fd;

	cmd = get_input("Local command?");
	if (cmd == NULL || *cmd == '\0')
		return;

	switch (pid = fork()) {
	case -1:
		err(1, "fork");
	case 0:
		fd = open(_PATH_DEVNULL, O_RDWR);
		if (fd < 0 || dup2(fd, STDIN_FILENO) == -1)
			_exit(1);
		close(fd);

		/* attach stdout to line */
		if (dup2(line_fd, STDOUT_FILENO) == -1)
			_exit(1);

		if (closefrom(STDOUT_FILENO + 1) != 0)
			_exit(1);

		execl(_PATH_BSHELL, "sh", "-c", cmd, (void*)NULL);
		_exit(1);
	default:
		while (waitpid(pid, NULL, 0) == -1 && errno == EINTR)
			/* nothing */;
		break;
	}
}

void
send_file(void)
{
	const char	*file;
	FILE		*f;
	char		 buf[BUFSIZ], *expanded;
	size_t		 len;

	file = get_input("Local file?");
	if (file == NULL || *file == '\0')
		return;

	expanded = tilde_expand(file);
	f = fopen(expanded, "r");
	if (f == NULL) {
		warn("%s", file);
		return;
	}

	while (!feof(f) && !ferror(f)) {
		len = fread(buf, 1, sizeof(buf), f);
		if (len != 0)
			bufferevent_write(line_ev, buf, len);
	}

	fclose(f);
	free(expanded);
}

void
set_speed(void)
{
	const char	*s, *errstr;
	int		 speed;

	s = get_input("New speed?");
	if (s == NULL || *s == '\0')
		return;

	speed = strtonum(s, 0, UINT_MAX, &errstr);
	if (errstr != NULL) {
		warnx("speed is %s: %s", errstr, s);
		return;
	}

	set_line(speed);
}

void
do_command(char c)
{
	switch (c) {
	case '.':
	case '\004': /* ^D */
		event_loopexit(NULL);
		break;
	case '\032': /* ^Z */
		restore_termios();
		kill(getpid(), SIGTSTP);
		set_termios();
		break;
	case 'S':
		set_speed();
		break;
	case '$':
		pipe_command();
		break;
	case '>':
		send_file();
		break;
	case '#':
		ioctl(line_fd, TIOCSBRK, NULL);
		sleep(1);
		ioctl(line_fd, TIOCCBRK, NULL);
		break;
	case '?':
		printf("\r\n"
		    "~#      send break\r\n"
		    "~$      pipe local command to remote host\r\n"
		    "~>      send file to remote host\r\n"
		    "~S      set speed\r\n"
		    "~?      get this summary\r\n"
		);
		break;
	}
}
