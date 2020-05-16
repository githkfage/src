/* $OpenBSD: cmd-refresh-client.c,v 1.33 2020/05/16 15:45:29 nicm Exp $ */

/*
 * Copyright (c) 2007 Nicholas Marriott <nicholas.marriott@gmail.com>
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
#include <string.h>

#include "tmux.h"

/*
 * Refresh client.
 */

static enum cmd_retval	cmd_refresh_client_exec(struct cmd *,
			    struct cmdq_item *);

const struct cmd_entry cmd_refresh_client_entry = {
	.name = "refresh-client",
	.alias = "refresh",

	.args = { "cC:Df:F:lLRSt:U", 0, 1 },
	.usage = "[-cDlLRSU] [-C XxY] [-f flags] " CMD_TARGET_CLIENT_USAGE
		" [adjustment]",

	.flags = CMD_AFTERHOOK|CMD_CLIENT_TFLAG,
	.exec = cmd_refresh_client_exec
};

static enum cmd_retval
cmd_refresh_client_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args	*args = cmd_get_args(self);
	struct client	*tc = cmdq_get_target_client(item);
	struct tty	*tty = &tc->tty;
	struct window	*w;
	const char	*size, *errstr;
	u_int		 x, y, adjust;

	if (args_has(args, 'c') ||
	    args_has(args, 'L') ||
	    args_has(args, 'R') ||
	    args_has(args, 'U') ||
	    args_has(args, 'D'))
	{
		if (args->argc == 0)
			adjust = 1;
		else {
			adjust = strtonum(args->argv[0], 1, INT_MAX, &errstr);
			if (errstr != NULL) {
				cmdq_error(item, "adjustment %s", errstr);
				return (CMD_RETURN_ERROR);
			}
		}

		if (args_has(args, 'c'))
			tc->pan_window = NULL;
		else {
			w = tc->session->curw->window;
			if (tc->pan_window != w) {
				tc->pan_window = w;
				tc->pan_ox = tty->oox;
				tc->pan_oy = tty->ooy;
			}
			if (args_has(args, 'L')) {
				if (tc->pan_ox > adjust)
					tc->pan_ox -= adjust;
				else
					tc->pan_ox = 0;
			} else if (args_has(args, 'R')) {
				tc->pan_ox += adjust;
				if (tc->pan_ox > w->sx - tty->osx)
					tc->pan_ox = w->sx - tty->osx;
			} else if (args_has(args, 'U')) {
				if (tc->pan_oy > adjust)
					tc->pan_oy -= adjust;
				else
					tc->pan_oy = 0;
			} else if (args_has(args, 'D')) {
				tc->pan_oy += adjust;
				if (tc->pan_oy > w->sy - tty->osy)
					tc->pan_oy = w->sy - tty->osy;
			}
		}
		tty_update_client_offset(tc);
		server_redraw_client(tc);
		return (CMD_RETURN_NORMAL);
	}

	if (args_has(args, 'l')) {
		tty_putcode_ptr2(&tc->tty, TTYC_MS, "", "?");
		return (CMD_RETURN_NORMAL);
	}

	if (args_has(args, 'F')) /* -F is an alias for -f */
		server_client_set_flags(tc, args_get(args, 'F'));
	if (args_has(args, 'f'))
		server_client_set_flags(tc, args_get(args, 'f'));

	if (args_has(args, 'C')) {
		if (args_has(args, 'C')) {
			if (!(tc->flags & CLIENT_CONTROL)) {
				cmdq_error(item, "not a control client");
				return (CMD_RETURN_ERROR);
			}
			size = args_get(args, 'C');
			if (sscanf(size, "%u,%u", &x, &y) != 2 &&
			    sscanf(size, "%ux%u", &x, &y) != 2) {
				cmdq_error(item, "bad size argument");
				return (CMD_RETURN_ERROR);
			}
			if (x < WINDOW_MINIMUM || x > WINDOW_MAXIMUM ||
			    y < WINDOW_MINIMUM || y > WINDOW_MAXIMUM) {
				cmdq_error(item, "size too small or too big");
				return (CMD_RETURN_ERROR);
			}
			tty_set_size(&tc->tty, x, y, 0, 0);
			tc->flags |= CLIENT_SIZECHANGED;
			recalculate_sizes();
		}
		return (CMD_RETURN_NORMAL);
	}

	if (args_has(args, 'S')) {
		tc->flags |= CLIENT_STATUSFORCE;
		server_status_client(tc);
	} else {
		tc->flags |= CLIENT_STATUSFORCE;
		server_redraw_client(tc);
	}
	return (CMD_RETURN_NORMAL);
}
