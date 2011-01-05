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


/*
 * This module deals with drawing our gui, and handling input, and all that
 * really fun ui stuff.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <cddb/cddb.h>

#include "cdrom.h"
#include "ncdplay.h"

/*
 * This function initializes our pretty colors ;)
 */
void init_ncd_colors(void)
{
	start_color();

	init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
	init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
	init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
	init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
	init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
}

/*
 * This function creates a window.
 *
 * @parm height:	The height of the window.
 * @parm width:		The width of the window.
 * @parm y:		The y (row) location. 
 * @parm x:		The x (col) lcation.
 * @return:		WINDOW ptr
 */
WINDOW *win_ncd_new(int height, int width, int y, int x)
{
	WINDOW *new = newwin(height, width, y, x);
	wattron(new, COLOR_PAIR(COLOR_GREEN));
	box(new, 0, 0);
	wattroff(new, COLOR_PAIR(COLOR_GREEN));
	wrefresh(new);
	return (new);
}

/*
 * This function deletes a window.
 *
 * @param win:	The window to be deleted (destroyed).
 */
void win_ncd_del(WINDOW **win)
{
	if(win) {
		wborder(*win, 32, 32, 32, 32, 32, 32, 32, 32);
		wrefresh(*win);
		delwin(*win);
		*win = NULL;
	}
}

/*
 * This is actually the main program loop.
 */

void ncd_loop(void)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;
	int c = 0, *m_idx = &ngdp->win_info.m_index;

	mousemask(BUTTON_CTRL, NULL);
	signal(SIGALRM, update_status);
	alarm(3);

	while(ngdp->loop) {	/* Escape quits. */
		c = getch();

		switch(c) {
			case KEY_UP: {
				if(*m_idx > 1) {
					if(ngdp->current == ngdp->controls &&
							ngdp->win_info.f_ptr[*m_idx - 2] == (unsigned long) ~0) {
						ncd_scroll_menu(*m_idx, *m_idx-2);
						*m_idx -= 2;
					} else {
						ncd_scroll_menu(*m_idx, *m_idx-1);
						*m_idx -= 1;
					}
				}
			} break;

			case KEY_DOWN: {
				if(*m_idx < ngdp->win_info.m_items) {
					if(ngdp->current == ngdp->controls &&
							ngdp->win_info.f_ptr[*m_idx] == (unsigned long) ~0) {
						ncd_scroll_menu(*m_idx, *m_idx + 2);
						*m_idx += 2;
					} else {
						ncd_scroll_menu(*m_idx, *m_idx+1);
						*m_idx += 1;
					}
				}
			} break;

			case 0x09: {
				if(ngdp->cdrom_info.tracks == 0)
					break;

				ncd_unhighlight_menu_item(*m_idx);
				*m_idx = 1;
				if(ngdp->current == ngdp->controls)
					set_current_win_p(ngdp->playlist, ngdp->win_info.pl_height,
						ngdp->win_info.pl_width, ngdp->cdrom_info.tracks);
				else
					set_current_win_p(ngdp->controls, ngdp->win_info.ctls_height,
						ngdp->win_info.ctls_width, ngdp->win_info.item_count);
				ncd_highlight_menu_item(*m_idx);
			} break;

			case 0x0a: {
				unsigned long m_ptr = ngdp->win_info.f_ptr[*m_idx - 1];
				if(ngdp->current == ngdp->controls && m_ptr != 0 && m_ptr != (unsigned long) ~0) {
					void (*func)() = (void *) m_ptr;
					func();
				} else if(ngdp->current == ngdp->playlist) {
					cdrom_play_track(*m_idx);
					update_status(0);
				}
			} break;

			case 0x20: {
				pause_resume_audio();
				update_status(0);
			} break;

			case KEY_END: {
				stop_audio();
				update_status(0);
			} break;

			case KEY_RIGHT: {
				MEVENT m;
				if(getmouse(&m) == OK) {
					if(!(m.bstate & BUTTON_CTRL))
						cdrom_skip_track();
					else
						cdrom_seek_cue();
				}
			} break;

			case '>': cdrom_seek_cue(); break;

			case KEY_LEFT:
				cdrom_prev_track();
			break;

			case KEY_F(12):
				cdrom_eject();
			break;

			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
			case 0x34:
			case 0x35:
			case 0x36:
			case 0x37:
			case 0x38:
			case 0x39: {
				int trk = c - 0x30;
				if(trk == 0) trk = 10;
				cdrom_play_track(trk);
				update_status(0);
			} break;

			case KEY_F(1): cdrom_play_track(11); break;
			case KEY_F(2): cdrom_play_track(12); break;
			case KEY_F(3): cdrom_play_track(13); break;
			case KEY_F(4): cdrom_play_track(14); break;
			case KEY_F(5): cdrom_play_track(15); break;
			case KEY_F(6): cdrom_play_track(16); break;
			case KEY_F(7): cdrom_play_track(17); break;
			case KEY_F(8): cdrom_play_track(18); break;
			case KEY_F(9): cdrom_play_track(19); break;
			case KEY_F(10): cdrom_play_track(20); break;

			/* if the cd has 20+ tracks, you'll have to goto the track sorry ;p */

			case 0x1b:	/* Reserve.. just incase things go wrong. */
				ngdp->loop = 0;
			break;
		}
	}
}

/*
 * This draw's appropriate strings.
 * xxxxx -- that's what it used to do, now it initializes all
 * the menu's and what not.
 */

void draw_strings(void)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;

	ncd_ctrls_add_item(menu_read_cd, "Read CD");

	/* TODO: add configuration file -- to save opts. */
	/* / = Repeat One, X = Repeat All */
	ncd_ctrls_add_item(menu_toggle_repeat, "Repeat  [ ]");
	ncd_ctrls_add_item(NULL, "Shuffle [ ]");	/* same for here */	
	ncd_ctrls_add_item(NULL, NULL);

	ncd_ctrls_add_item(cdrom_skip_track, "Skip Track");
	ncd_ctrls_add_item(cdrom_prev_track, "Prev Track");
	ncd_ctrls_add_item(cdrom_eject, "Eject CD");


	/* this `should' be the last entry. */
	ncd_ctrls_add_item(NULL, NULL);
	ncd_ctrls_add_item(end_program_loop, "Exit");
	

	if(ngdp->auto_load) {
		read_tracks();
		ngdp->win_info.m_index = (ngdp->auto_load > 1? ngdp->auto_load - 1: 1);
	} else {
		ncd_add_to_list(0, "No CD is currently loaded");
		set_current_win_p(ngdp->controls, ngdp->win_info.ctls_height,
			ngdp->win_info.ctls_width, ngdp->win_info.item_count);
		ncd_highlight_menu_item(1);
		ngdp->win_info.m_index = 1;
	}
}

/*
 * This function adds the tracks to the playlist
 */

void ncd_add_to_list(int track, const char *fmt, ...)
{
	WINDOW *list = ncd_glob_d.playlist;
	int max = ncd_glob_d.win_info.pl_width - 10;
	va_list ap;

	wmove(list, (track > 0? track: 1), 2);
	va_start(ap, fmt);

	if(track > 0)
		wprintw(list, "%2d. ", track);

	if(va_fmt_length(fmt, ap) < max) {
		va_end(ap);
		va_start(ap, fmt);
		vw_printw(list, fmt, ap);
	} else {
		va_end(ap);
		va_start(ap, fmt);
		char buf[max + 1];
		vsnprintf(buf, max, fmt, ap);
		buf[max - 2] = 0;
		wprintw(list, "%s...", buf);
	}

	va_end(ap);
	wrefresh(list);
}

/*
 * This function will highlight the menu item that the `cursor' is over.
 */

void ncd_highlight_menu_item(int item)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;

	mvwchgat(ngdp->current, item, 2, ngdp->_width - 3,
		A_REVERSE, COLOR_YELLOW, NULL);

	/* EW!! is this an ncurses bug? :| */
	touchwin(ngdp->current);
	wrefresh(ngdp->current);
}

/*
 * This function is in charge of doing the actual `scrolling'
 */

void ncd_scroll_menu(int cur, int next)
{
	/* TODO: actually `scroll' */

	ncd_unhighlight_menu_item(cur);
	ncd_highlight_menu_item(next);
}

/*
 * This function is in charge of removing the highlight
 */

void ncd_unhighlight_menu_item(int item)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;
	mvwchgat(ngdp->current, item, 2, ngdp->_width - 3,
		A_NORMAL, COLOR_WHITE, NULL);
	touchwin(ngdp->current);
	wrefresh(ngdp->current);
}

/*
 * This function is quite interesting really, this function will add
 * a menu item to the `controls' panel, and then link a function to
 * it, when the user hits enter.
 */

void ncd_ctrls_add_item(void (*func)(), const char *fmt, ...)
{
	WINDOW *ctrl = ncd_glob_d.controls;
	va_list ap;
	int *idx = &ncd_glob_d.win_info.item_count;

	if(fmt == NULL) {
		ncd_glob_d.win_info.f_ptr[*idx] = ~0;
		*idx += 1;
		ncd_glob_d.win_info.item_count = *idx;
		wrefresh(ctrl);
		return;
	}	

	ncd_glob_d.win_info.f_ptr[*idx] = (unsigned long) func;
	*idx += 1; 

	wmove(ctrl, *idx, 2);
	va_start(ap, fmt);
	vw_printw(ctrl, fmt, ap);
	va_end(ap);

	ncd_glob_d.win_info.item_count = *idx;
	wrefresh(ctrl);
}

/*
 * this function is just a front end to read_tracks, it just make sure
 * that the playlist gets cleared first.
 */

void menu_read_cd(void)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;

	/* this should never really happen, but .. */
	if(ngdp->current == ngdp->playlist)
		return;	/* probably should resetup the data, and so on.. but like I said
		         * this `should' never really happen */

	win_ncd_del(&ngdp->playlist);
	ngdp->playlist = win_ncd_new(ngdp->win_info.pl_height,
		ngdp->win_info.pl_width, 0, (COLS / 4) + 10);

	read_tracks();
}

/*
 * This function `edits' a Menu entry.
 * TODO (?): allow for edit of function (for plygins manybe...)
 */

void ncd_ctrls_edit_item(int item, const char *fmt, ...)
{
	WINDOW *ctrl = ncd_glob_d.controls;
	va_list ap;

	wmove(ctrl, item, 2);
	va_start(ap, fmt);
	vw_printw(ctrl, fmt, ap);
	va_end(ap);

	wrefresh(ctrl);
}

/*
 * This function just puts stuff in the `status' window
 */

void status_window(const char *fmt, ...)
{
	WINDOW *status = ncd_glob_d.status;
	int max = COLS - 18;
	va_list ap;

	win_ncd_del(&status);
	status = ncd_glob_d.status = win_ncd_new(3, COLS - 7, LINES - 4, 3);

	wmove(status, 1, 2);
	wattron(status, COLOR_PAIR(COLOR_RED));

	if(fmt) {
		va_start(ap, fmt);
		if(va_fmt_length(fmt, ap) < max) {
			va_end(ap);
			va_start(ap, fmt);
			vw_printw(status, fmt, ap);
		} else {
			va_end(ap);
			va_start(ap, fmt);
			char buf[max + 1];
			vsnprintf(buf, max, fmt, ap);
			buf[max - 2] = 0;
			wprintw(status, "%s...", buf);
		}
		va_end(ap);
	} else
		wprintw(status, "ncdplay - an ncurses cd player");

	wattroff(status, COLOR_PAIR(COLOR_RED));
	wrefresh(status);
}

void status_window_app(const char *fmt, ...)
{
	WINDOW *status = ncd_glob_d.status;
	va_list ap;

	wattron(status, COLOR_PAIR(COLOR_RED));
	va_start(ap, fmt);
	vw_printw(status, fmt, ap);
	va_end(ap);
	wattroff(status, COLOR_PAIR(COLOR_RED));

	wrefresh(status);
}

void ncd_list_put(int idx, const char *fmt, ...)
{
	WINDOW *list = ncd_glob_d.playlist;
	va_list ap;

	wmove(list, idx, 2);

	va_start(ap, fmt);
	vw_printw(list, fmt, ap);
	va_end(ap);

	wrefresh(list);
}

/*
 * This function is just our hook for SIGALRM
 */
void update_status(int was_sig)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;

	if(cd_has_changed()) {
		if(ngdp->auto_load) {
			read_tracks();
		} else {
			if(ngdp->current != ngdp->controls) {
				ncd_unhighlight_menu_item(ngdp->win_info.m_index);
				set_current_win_p(ngdp->controls, ngdp->win_info.ctls_height,
					ngdp->win_info.ctls_width, ngdp->win_info.item_count);
				ncd_highlight_menu_item(1);
				ngdp->win_info.m_index = 1;
			}
		}
		if(was_sig)
			alarm(3);	/* 3 second timer. */
		return;
	}

#define A(x) chan.cdsc_##x
	struct cdrom_subchnl chan;
	A(format) = CDROM_LBA;
	ioctl(ngdp->cdrom, CDROMSUBCHNL, &chan);

	cddb_track_t *track = cddb_get_track_by_index(A(trk));

	if(A(audiostatus) == CDROM_AUDIO_PLAY) {
		if(track)
			status_window("%s - %s | %i. %s |>",
				cddb_disc_get_artist(ngdp->cdrom_info.disk),
				cddb_disc_get_title(ngdp->cdrom_info.disk), A(trk),
				cddb_track_get_title(track));
		else
			status_window("Uknown | %i. Track %i |>", A(trk), A(trk));
	} else if(A(audiostatus) == CDROM_AUDIO_PAUSED) {
		if(track)
			status_window("%s - %s | %i. %s ||",
				cddb_disc_get_artist(ngdp->cdrom_info.disk),
				cddb_disc_get_title(ngdp->cdrom_info.disk), A(trk),
				cddb_track_get_title(track));
		else
			status_window("Uknown | %i. Track %i ||", A(trk), A(trk));
	} else {
		if(ngdp->cdrom_info.tracks != 0) { 
			if(track)
				status_window("%s - %s",
					cddb_disc_get_artist(ngdp->cdrom_info.disk),
					cddb_disc_get_title(ngdp->cdrom_info.disk));
			else
				status_window("Uknown");

			if((ngdp->options & CD_STOP) == CD_STOP)
				status_window(NULL);
			else {
				if(IsRepeatAll(ngdp->options))
					cdrom_play_track(1);
			}
		}
	}
#undef A

	if(was_sig)
		alarm(3);
}

/*
 * This is the menu callback for the Repeat option.
 * This option basically toggles whether or not we repeat.
 */
/*
 * TODO: Repeat One
 */

void menu_toggle_repeat(void)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;

	ncd_unhighlight_menu_item(2);

	if(IsRepeatAll(ngdp->options)) {
		ncd_ctrls_edit_item(2, "Repeat  [ ]");
		ClrOption(ngdp, REPA);
	} else {
		ncd_ctrls_edit_item(2, "Repeat  [X]");
		SetOption(ngdp, REPA);
	}

	ncd_highlight_menu_item(2);
}
	
