/* $OpenBSD: cmd-lock-client.c,v 1.1 2009/09/24 14:17:09 nicm Exp $ */

/*
 * Copyright (c) 2009 Nicholas Marriott <nicm@users.sourceforge.net>
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

#include "tmux.h"

/*
 * Lock a single client.
 */

int	cmd_lock_client_exec(struct cmd *, struct cmd_ctx *);

const struct cmd_entry cmd_lock_client_entry = {
	"lock-client", "lockc",
	CMD_TARGET_CLIENT_USAGE,
	0, 0,
	cmd_target_init,
	cmd_target_parse,
	cmd_lock_client_exec,
	cmd_target_free,
	cmd_target_print
};

int
cmd_lock_client_exec(struct cmd *self, struct cmd_ctx *ctx)
{
	struct cmd_target_data	*data = self->data;
	struct client		*c;

	if ((c = cmd_find_client(ctx, data->target)) == NULL)
		return (-1);

	server_lock_client(c);
	recalculate_sizes();

	return (0);
}
