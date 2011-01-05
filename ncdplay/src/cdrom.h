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

#ifndef CDROM_H
# define CDROM_H

# define CDROM_LBA		0x01
# define CDROM_MSF		0x02

# define CD_MINS		74
# define CD_SECS		60
# define CD_FRAMES		75

# define CDROM_LEADOUT		0xAA

# define CDROMPAUSE		0x5301
# define CDROMRESUME		0x5302
# define CDROMPLAYMSF		0x5303
# define CDROMPLAYTRKIND	0x5304
# define CDROMREADTOCHDR	0x5305
# define CDROMREADTOCENTRY	0x5306
# define CDROMSTOP		0x5307
# define CDROMSUBCHNL		0x530b
# define CDROMCLOSETRAY		0x5319
# define CDROMEJECT		0x5309

# define CDROM_AUDIO_PLAY	0x11
# define CDROM_AUDIO_PAUSED	0x12
# define CDROM_AUDIO_COMPLETED	0x13

# define CDROM_DRIVE_STATUS	0x5326
# define CDROM_MEDIA_CHANGED	0x5325

# define CDS_TRAY_OPEN		0x02

struct cdrom_msf0
{
	unsigned char minute;
	unsigned char second;
	unsigned char frame;
};

union cdrom_addr
{
	struct cdrom_msf0 msf;
	int lba;
};

struct cdrom_msf
{
	unsigned char cdmsf_min0;
	unsigned char cdmsf_sec0;
	unsigned char cdmsf_frame0;
	unsigned char cdmsf_min1;
	unsigned char cdmsf_sec1;
	unsigned char cdmsf_frame1;
};

struct cdrom_ti
{
	unsigned char cdti_trk0;
	unsigned char cdti_ind0;
	unsigned char cdti_trk1;
	unsigned char cdti_ind1;
};

struct cdrom_tochdr
{
	unsigned char cdth_trk0;
	unsigned char cdth_trk1;
};

struct cdrom_tocentry
{
	unsigned char cdte_track;
	unsigned char cdte_adr:4;
	unsigned char cdte_ctrl:4;
	unsigned char cdte_format;
	union cdrom_addr cdte_addr;
	unsigned char cdte_datamode;
};

struct cdrom_subchnl
{
	unsigned char cdsc_format;
	unsigned char cdsc_audiostatus;
	unsigned char cdsc_adr: 4;
	unsigned char cdsc_ctrl: 4;
	unsigned char cdsc_trk;
	unsigned char cdsc_ind;
	union cdrom_addr cdsc_absaddr;
	union cdrom_addr cdsc_reladdr;
};

#endif	/* !CDROM_H */
