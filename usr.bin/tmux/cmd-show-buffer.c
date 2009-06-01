/* $OpenBSD: cmd-show-buffer.c,v 1.1 2009/06/01 22:58:49 nicm Exp $ */

/*
 * Copyright (c) 2007 Nicholas Marriott <nicm@users.sourceforge.net>
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

#include <stdlib.h>

#include "tmux.h"

/*
 * Show a session paste buffer.
 */

int	cmd_show_buffer_exec(struct cmd *, struct cmd_ctx *);

const struct cmd_entry cmd_show_buffer_entry = {
	"show-buffer", "showb",
	CMD_BUFFER_SESSION_USAGE,
	0,
	cmd_buffer_init,
	cmd_buffer_parse,
	cmd_show_buffer_exec,
	cmd_buffer_send,
	cmd_buffer_recv,
	cmd_buffer_free,
	cmd_buffer_print
};

int
cmd_show_buffer_exec(struct cmd *self, struct cmd_ctx *ctx)
{
	struct cmd_buffer_data	*data = self->data;
	struct session		*s;
	struct paste_buffer	*pb;
	u_int			 size;
	char			*buf, *ptr;
	size_t			 len;

	if ((s = cmd_find_session(ctx, data->target)) == NULL)
		return (-1);

	if (data->buffer == -1) {
		if ((pb = paste_get_top(&s->buffers)) == NULL) {
			ctx->error(ctx, "no buffers");
			return (-1);
		}
	} else if ((pb = paste_get_index(&s->buffers, data->buffer)) == NULL) {
		ctx->error(ctx, "no buffer %d", data->buffer);
		return (-1);
	}

	if (pb != NULL) {
		size = s->sx;

		buf = xmalloc(size + 1);
		len = 0;

		ptr = pb->data;
		do {
			buf[len++] = *ptr++;

			if (len == size) {
				buf[len] = '\0';
				ctx->print(ctx, buf);

				len = 0;
			}
		} while (*ptr != '\0');
		buf[len] = '\0';
		ctx->print(ctx, buf);
	}

	return (0);
}
