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
 * To make things easier, this will be the only header in this project.
 * Things in here may get a little messy ;)
 */

#ifndef NCD_PLAY_
# define NCD_PLAY_

# include <ncurses.h>
# include <stdarg.h>
# include <cddb/cddb.h>

# define OPT_REP1   0x01	/* Repeat 1*/
# define OPT_REPA   0x02	/* Repeat All */
# define OPT_SHUF   0x04	/* Shuffle */
# define OPT_SHUFR  0x08	/* SHuffle Repeat */

# define CD_STOP    0x0100	/* Our we doing nothing? ;P */
# define CD_PLAY	  0x0200	/* Our we playing a cd? */
# define ST_ASKN    0x0400	/* asking for cddb lookup. */

# define SetOption(n, o) \
	((n)->options |= OPT_##o)

# define ClrOption(n, o) \
	((n)->options &= ~OPT_##o)

# define IsRepeatOne(x) ((x & OPT_REP1) == OPT_REP1)
# define IsRepeatAll(x) ((x & OPT_REPA) == OPT_REPA)
# define IsRepeatable(x) (IsRepeatOne(x) || IsRepeatAll(x))

/* Data that needs to be made global will be defined in this struct. 
 * Actually, I'm being lazy, so a _lot_ of bs is going in here.
 */
struct ncd_global_data {
	WINDOW *controls;	/* The controls window */
	WINDOW *playlist;	/* this is the playlist window. */
	WINDOW *current;	/* pointer to the current window */
	WINDOW *status;		/* status window .. */
	int cdrom;		/* our cdrom handle. */
	int auto_load;	/* Auto load (read) the cd? i*/ 
	char cdp[0xff];	/* the CD path. */

	/* from here on out, I'm gonna be a little more organized ;)..maybe
	 * and less commenty.. sorry :P */

	struct {
		cddb_disc_t *disk;	/* this is our cddb disk */
		int tracks;
		struct {	/* *sighs* */
			int st_frame;
			int ed_frame;
		} msf_frame_index[0xAA];
	} cdrom_info;

# define msf_fi cdrom_info.msf_frame_index

	struct {
		int ctls_width;
		int ctls_height;

		int pl_width;
		int pl_height;

		/* This stuff should probably be in a menu struct .. it's 5:30am, I've
		 * been up all, day give me a break. :( */

		int m_index;
		int m_items;
		int item_count;
		unsigned long f_ptr[64];	/* 64 Menu items.. doubt it :P */
	} win_info;

	char loop;
	int _width;
	int _height;

	/* This is the options variable.. This variable actually holds more inform-
	 * ation than just `options', it holds the state that the cd is in, as well.
	 * the syntax of the variable is this:
	 * v-     cd  state   -v v-  options  -v
	 *_______________________________________
	 *|15|14|13|12|11|10|9|8|7|6|5|4|3|2|1|0|
	 *---------------------------------------
	 * hopefully you understood that diagram.. :|
	 */
	unsigned int options;
};


/* functions that can be found in gui.c */
void init_ncd_colors(void);
WINDOW *win_ncd_new(int height, int width, int y, int x);
void win_ncd_del(WINDOW **win);
void ncd_loop(void);
void draw_strings(void);
void ncd_add_to_list(int track, const char *fmt, ...);
void ncd_highlight_menu_item(int item);
void ncd_scroll_menu(int cur, int next);
void ncd_unhighlight_menu_item(int item);
void ncd_ctrls_add_item(void (*func)(), const char *fmt, ...);
void menu_read_cd(void);
void ncd_ctrls_edit_item(int item, const char *fmt, ...);
void status_window(const char *fmt, ...);
void status_window_app(const char *fmt, ...);
void ncd_list_put(int idx, const char *fmt, ...);
void update_status(int was_sig);
void menu_toggle_repeat(void);

/* functions that can be found in cdrom.c */
int init_drive(void);
void read_tracks(void);
int cd_has_changed(void);
void cdrom_play_track(int track);
void pause_resume_audio(void);
void stop_audio(void);
void cdrom_change_play_opt(int op);
void cdrom_eject(void);
void cdrom_skip_track(void);
void cdrom_prev_track(void);
void cdrom_seek_cue(void);

/* functions that can be found in ncdplay.c (along with main ;))*/
void usage(const char *app);
void set_current_win_p(WINDOW *w, int height, int width, int items);
void end_program_loop(void);
int va_fmt_length(const char *fmt, va_list ap);

/* functions in cddb.c */
void ncd_new_disc(void);
cddb_track_t *cddb_get_track_by_index(int trk);

/* this is meant to be referenced by ptr */
extern struct ncd_global_data ncd_glob_d;

#endif	/* !NCD_PLAY_ */
