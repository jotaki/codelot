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
 * This function initializes the cdrom drive.
 * @return: 0 on success, or errno
 */

int init_drive(void)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;
	struct cdrom_subchnl chan;
	int fd = -1;

	if((fd = open(ngdp->cdp, O_RDONLY | O_NONBLOCK)) < 0)
		return (errno);

#define A(x) chan.cdsc_##x

	A(format) = CDROM_LBA;
	ioctl(fd, CDROMSUBCHNL, &chan);

	if(A(audiostatus) == CDROM_AUDIO_PLAY) {
		ngdp->auto_load = A(trk) + 1;	/* force auto load. */
	} else if(A(audiostatus) == CDROM_AUDIO_PAUSED) {
		ngdp->auto_load = A(trk) + 1;	/* force auto load. */
	} else
		status_window(NULL);

#undef A

	ngdp->cdrom = fd;
	return (0);
}

/*
 * This function reads the tracks, and adds them to the playlist
 * TODO: add better error handling .. (ie: inform the user)
 */

void read_tracks(void)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;
	struct cdrom_tochdr cdrom_toc;
	cddb_disc_t *disk = NULL;
	int idx = 0;

	alarm(0);

	ncd_new_disc();
	disk = ngdp->cdrom_info.disk;

	/*
	 * clean up the window, from what ever the cddb function decided
	 * to plop on it.
	 */

	unsigned long tmp = (unsigned long) ngdp->playlist;

	win_ncd_del(&ngdp->playlist);
	ngdp->playlist = win_ncd_new(ngdp->win_info.pl_height,
		ngdp->win_info.pl_width, 0, (COLS / 4) + 10);

	if(tmp == (unsigned long) ngdp->current)	/* update current */
		ngdp->current = ngdp->playlist;

	if(ioctl(ngdp->cdrom, CDROMREADTOCHDR, &cdrom_toc) < 0)
		return;

	ngdp->cdrom_info.tracks = cdrom_toc.cdth_trk1;

	if(disk) {
		cddb_track_t *track;

		status_window("%s - %s", cddb_disc_get_artist(disk),
			cddb_disc_get_title(disk));

		idx = 1;
		for(track = cddb_disc_get_track_first(disk); track != NULL;
				track = cddb_disc_get_track_next(disk)) {
			ncd_add_to_list(idx++, "%s", cddb_track_get_title(track));
		}
	} else {
		for(idx = cdrom_toc.cdth_trk0; idx <= cdrom_toc.cdth_trk1; idx++)
			ncd_add_to_list(idx, "Track %d", idx);
	}

	if(ngdp->current == ngdp->playlist) {
		ngdp->win_info.m_items = cdrom_toc.cdth_trk1;
		if(ngdp->auto_load > 1)
			ncd_highlight_menu_item(ngdp->auto_load - 1);
		else
			ncd_highlight_menu_item(cdrom_toc.cdth_trk0);
	}

	alarm(3);
}

/*
 * This function does the very basic handling, of checking if the cd has
 * changed, if it has it returns true (1), or false (0), if it hasn't.
 */

int cd_has_changed(void)
{
	return (ioctl(ncd_glob_d.cdrom, CDROM_MEDIA_CHANGED, 0) == 1);
}

void cdrom_play_track(int track)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;
	struct cdrom_ti tracks;
	unsigned int state = ( ngdp->options & 0xff00 );
	unsigned int opts = ( ngdp->options & 0xff );

	if(track < 1 || track > ngdp->cdrom_info.tracks)
		return;

	if(state == CD_PLAY) {	/* playing a cd, need to stop it first ;P */
		ioctl(ngdp->cdrom, CDROMSTOP, 0);
		state = CD_STOP;
	}

	tracks.cdti_trk0 = track;
	tracks.cdti_ind0 = 0x00;
	tracks.cdti_ind1 = 0xff;
	tracks.cdti_trk1 = ngdp->cdrom_info.tracks;

	if(ioctl(ngdp->cdrom, CDROMPLAYTRKIND, &tracks) != -1)
		state = CD_PLAY;
	
	ngdp->options = state | opts;
}

/*
 * This function is for pausing and resuming audio.
 */

void pause_resume_audio(void)
{
	struct cdrom_subchnl chan;

#define A(x) chan.cdsc_##x

	A(format) = CDROM_LBA;
	ioctl(ncd_glob_d.cdrom, CDROMSUBCHNL, &chan);

	if(A(audiostatus) == CDROM_AUDIO_PLAY) {
		ioctl(ncd_glob_d.cdrom, CDROMPAUSE, 0);
	} else if(A(audiostatus) == CDROM_AUDIO_PAUSED) {
		ioctl(ncd_glob_d.cdrom, CDROMRESUME, 0);
	}

#undef A
}

/*
 * STOP THE AUDIO!!
 */

void stop_audio(void)
{
	ioctl(ncd_glob_d.cdrom, CDROMSTOP, 0);
	ncd_glob_d.options = (ncd_glob_d.options & 0xff) | CD_STOP;
}

/*
 * Change options
 */

void cdrom_change_play_opt(int op)
{
	/* TODO */
}

/*
 * Eject the CD.
 */

void cdrom_eject(void)
{
	int fd = ncd_glob_d.cdrom;
	int t = 0;

	stop_audio();

	t = ioctl(fd, CDROM_DRIVE_STATUS, 0);
	if(t != CDS_TRAY_OPEN)
		ioctl(fd, CDROMEJECT, 0);
	else if(t == CDS_TRAY_OPEN)
		ioctl(fd, CDROMCLOSETRAY, 0);

	update_status(0);
}

/*
 * Skip to the next track
 */

void cdrom_skip_track(void)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;
	struct cdrom_subchnl chan;
	unsigned int opts = ( ngdp->options & 0xff );
	int fd = ngdp->cdrom;

	if(IsRepeatOne(opts))	/* Not supported, cause Silly */
		return;

#define A(x) chan.cdsc_##x
	A(format) = CDROM_LBA;
	ioctl(fd, CDROMSUBCHNL, &chan);

	if(A(audiostatus) == CDROM_AUDIO_PLAY)
		cdrom_play_track(A(trk) + 1);
#undef A

	update_status(0);
}

/*
 * Play the previous track
 */

void cdrom_prev_track(void)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;
	struct cdrom_subchnl chan;
	unsigned int opts = ( ngdp->options & 0xff );
	int fd = ngdp->cdrom;

	if(IsRepeatOne(opts))	/* Not supported, cause Silly */
		return;

#define A(x) chan.cdsc_##x
	A(format) = CDROM_LBA;
	ioctl(fd, CDROMSUBCHNL, &chan);

	if(A(audiostatus) == CDROM_AUDIO_PLAY)
		cdrom_play_track(A(trk) - 1);
#undef A

	update_status(0);
}

void cdrom_seek_cue(void)
{
	struct ncd_global_data *ngdp = &ncd_glob_d;
	struct cdrom_subchnl chan;
	struct cdrom_msf msf;
	int fd = ngdp->cdrom;

#define A(x) chan.cdsc_##x
	A(format) = CDROM_MSF;
	ioctl(fd, CDROMSUBCHNL, &chan);

	if(A(audiostatus) != CDROM_AUDIO_PLAY)
		return;

	int min = A(absaddr.msf.minute);
	int sec = A(absaddr.msf.second);
	int frm = A(absaddr.msf.frame);

	sec += 3;
	if(sec > 60) { min++; sec -= 60; }
	frm = ((min * CD_SECS + sec) * CD_FRAMES + A(absaddr.msf.frame));

	msf.cdmsf_min0 = min;
	msf.cdmsf_sec0 = sec;
	msf.cdmsf_frame0 = frm;

	int tt = ngdp->cdrom_info.tracks - 1;
	if(IsRepeatOne(ngdp->options))
		tt = A(trk) - 1;

	msf.cdmsf_min1 = ngdp->msf_fi[tt].ed_frame / CD_FRAMES / CD_SECS;
	msf.cdmsf_sec1 = ngdp->msf_fi[tt].ed_frame / CD_FRAMES % CD_SECS;
	msf.cdmsf_frame1 = ngdp->msf_fi[tt].ed_frame % CD_FRAMES % CD_SECS;

	/* ioctl(fd, CDROMSTOP, 0); */
	ioctl(fd, CDROMPLAYMSF, &msf);
#undef A
}
