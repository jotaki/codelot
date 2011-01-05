/*
 * ncdplay - An ncurses cd player
 * Copyright (C) 2006 The ncdplay team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <cddb/cddb.h>

#include "cdrom.h"
#include "ncdplay.h"

struct ncd_global_data ncd_glob_d;

int main(int argc, char *argv[])
{
	struct ncd_global_data *ngdp = &ncd_glob_d;
	char *drive = "/dev/cdrom";	/* change this if you'd like. */
	int tmp;	/* used for different calculations, and what not */

	memset(ngdp, 0, sizeof(struct ncd_global_data));

	if(argc > 1) {
		for(tmp = 1; tmp < argc; tmp++) {
			if(argv[tmp][0] == '-') {
				switch(argv[tmp][1]) {
					case 'a': ngdp->auto_load = 1; break;
					case 'h': usage(argv[0]); break;
					default:
						printf("invalid op: %s\n", argv[tmp]);
						usage(argv[0]);
				}
			} else {
				printf("using drive %s\n", argv[tmp]);
				drive = argv[tmp];
			}
		}
	}

	signal(SIGINT, SIG_IGN);	/* ignore ^C */
	strncpy(ngdp->cdp, drive, sizeof(ngdp->cdp));
	/* this is a cheap hack */
	printf("\033]0;ncdplay on %s\007", ngdp->cdp);

	initscr();
	keypad(stdscr, TRUE);
	cbreak();
	noecho();

	refresh();

	if(has_colors())
		init_ncd_colors();
	else {
		printw("Sorry, it doesn't appear your terminal supports colors.\n");
		printw("Press any key to exit.");
		refresh();
		getch();
		endwin();
		return (-1);
	}

	ngdp->cdrom = -1;		/* cdrom not loaded yet. */
	tmp = COLS / 4;

	ngdp->win_info.ctls_width = tmp + 4;
	ngdp->win_info.ctls_height = LINES - 4;
	ngdp->win_info.pl_width = tmp + (COLS / 2) - 14;
	ngdp->win_info.pl_height = LINES - 4;
	ngdp->options = CD_STOP;

	ngdp->controls = win_ncd_new(LINES - 4, tmp + 4, 0, 3);
	ngdp->playlist = win_ncd_new(LINES - 4, tmp + (COLS / 2) - 14, 0, tmp+10);
/*	ngdp->status = win_ncd_new(3, COLS - 14, LINES - 4, 3); */
	set_current_win_p(ngdp->playlist, ngdp->win_info.pl_height,
		ngdp->win_info.pl_width, 0);

	if((tmp = init_drive())) {
		endwin();
		errno = tmp;
		perror("init_drive failed");
		return (errno);
	}
	draw_strings();

	refresh();

	ngdp->loop = 1;
	ncd_loop();	/* this is the loop (found in gui.c) */

	win_ncd_del(&ngdp->playlist);
	win_ncd_del(&ngdp->controls);
	win_ncd_del(&ngdp->status);

	if(ngdp->cdrom_info.disk)
		cddb_disc_destroy(ngdp->cdrom_info.disk);

	ioctl(ngdp->cdrom, CDROMSTOP, 0);

	if(ngdp->cdrom != -1)
		close(ngdp->cdrom);

	endwin();
	return (0);
}

void usage(const char *app)
{
	printf("Usage: %s [options] [device]\n", app);
	printf("Where [options] can be one of:\n");
	printf("\t-h\tShows this help file.\n");
	printf("\t-a\tAuto read cdrom drive.\n");
	printf("\n[device] is where your cdrom is located, ie: /dev/cdrom\n");
	exit(0);
}

/*
 * This function just sets the current `window' that the user
 * is functioning in.
 */

void set_current_win_p(WINDOW *w, int height, int width, int items)
{
	ncd_glob_d.current = w;
	ncd_glob_d._height = height;
	ncd_glob_d._width = width;
	ncd_glob_d.win_info.m_items = items;
}

/*
 * End program loop ..
 */

void end_program_loop(void)
{
	ncd_glob_d.loop = 0;
}

/*
 * This function gets the length in total that will be
 * printed to the screen. (used for truncation)
 */

int va_fmt_length(const char *fmt, va_list ap)
{
	char buf[COLS + 1];
	return (vsnprintf(buf, COLS, fmt, ap));
}
