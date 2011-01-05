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

void ncd_new_disc(void)
{
	struct cdrom_tochdr cdrom_toc;
	struct ncd_global_data *ngdp = &ncd_glob_d;
	cddb_disc_t *disk = ncd_glob_d.cdrom_info.disk;
	int fd = ncd_glob_d.cdrom;

	if(disk != NULL) {
		cddb_disc_destroy(disk);
		ncd_glob_d.cdrom_info.disk = NULL;
	}

	if(ioctl(fd, CDROMREADTOCHDR, &cdrom_toc) < 0)
		return;

	disk = cddb_disc_new();
	if(disk == NULL)
		return;

	cddb_track_t * cddb_track;
	int dsk_st = 0, dsk_ed = 0, i;

	for(i = cdrom_toc.cdth_trk0; i <= cdrom_toc.cdth_trk1; i++) {
		struct cdrom_tocentry cdrom_entry = {
			.cdte_track = i,
			.cdte_format = CDROM_MSF,
		};

		if(ioctl(fd, CDROMREADTOCENTRY, &cdrom_entry) < 0) {
			cddb_disc_destroy(disk);
			return;
		}

		struct cdrom_tocentry cdrom_next_entry = {
			.cdte_track = (i == cdrom_toc.cdth_trk1 ? CDROM_LEADOUT : i + 1),
			.cdte_format = CDROM_MSF,
		};

		if(ioctl(fd, CDROMREADTOCENTRY, &cdrom_next_entry) < 0) {
			cddb_disc_destroy(disk);
			return;
		}

#define msf cdrom_entry.cdte_addr.msf
		int stf = ((msf.minute * CD_SECS + msf.second) * CD_FRAMES + msf.frame);
#undef msf
#define msf cdrom_next_entry.cdte_addr.msf
		int edf = ((msf.minute * CD_SECS + msf.second) * CD_FRAMES + msf.frame) - 1;
#undef msf

		ngdp->msf_fi[i - 1].st_frame = stf;
		ngdp->msf_fi[i - 1].ed_frame = edf;

		if(i == cdrom_toc.cdth_trk0) dsk_st = stf;
		if(i == cdrom_toc.cdth_trk1) dsk_ed = edf;

		cddb_track = cddb_track_new();
		if(cddb_track == NULL) {
			cddb_disc_destroy(disk);
			return;
		}

		cddb_track_set_frame_offset(cddb_track, stf);
		cddb_disc_add_track(disk, cddb_track);
	}
	cddb_disc_set_length(disk, (dsk_ed - dsk_st) / CD_FRAMES);

	if(!cddb_disc_calc_discid(disk)) {
		cddb_disc_destroy(disk);
		return;
	}

	cddb_conn_t *cddb_conn = cddb_new();
	if(!cddb_conn) {
		cddb_disc_destroy(disk);
		return;
	}

	cddb_set_server_name(cddb_conn, "freedb.org");
	cddb_set_server_port(cddb_conn, 888);

	int match = cddb_query(cddb_conn, disk);
	if(match <= 0) {
		cddb_disc_destroy(disk);
		return;
	}

	ncd_list_put(1, "disk id: 0x%08x | %i match(s)",
		cddb_disc_get_discid(disk), match);

	ncd_list_put(2, "please look at the status bar, and answer", match);

	int found = 0;
	while(match > 0) {
		if(!cddb_read(cddb_conn, disk)) {
			cddb_disc_destroy(disk);
			return;
		}

		status_window("%s - %s [y/n]", cddb_disc_get_artist(disk),
			cddb_disc_get_title(disk));

		int correct_key = 0;
		while(!correct_key) {
			switch(getch()) {
				case 'y':
					found = 1;
					correct_key = 1;
				break;

				case 'n':
					found = 0;
					correct_key = 1;
				break;
				default:
					found = 0;
					correct_key = 0;
			}
		}

		if(found) {
			ncd_glob_d.cdrom_info.disk = disk;
			return;
		}

		match--;
		if(!cddb_query_next(cddb_conn, disk)) {
			cddb_disc_destroy(disk);
			return;
		}
	}

	if(disk) cddb_disc_destroy(disk);
}

cddb_track_t *cddb_get_track_by_index(int trk)
{
	cddb_track_t *track;
	cddb_disc_t * disk = ncd_glob_d.cdrom_info.disk;

	if(disk) {
		for(track = cddb_disc_get_track_first(disk); track != NULL;
				track = cddb_disc_get_track_next(disk)) {
			if(cddb_track_get_number(track) == trk)
				return (track);
		}
	}
	return (NULL);
}
