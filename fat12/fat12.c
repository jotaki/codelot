#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "fat12.h"

int main(int argc, char *argv[])
{
	int fd, opti, dump_info = 0;
	short sector = 0;
	char *buffer, *appname, *ptr;
	char *imgfile = "a.img";
	struct stat img_status;
	struct fat12_cluster_entry *root_clus_entry;
	struct fat12_dir_entry *path;

	if((appname = strrchr(argv[0], '/')))
		++appname;
	else
		appname = argv[0];

	if(argc > 1) {
		for(opti = 1; opti < argc; ++opti) {
			if(strcmp(argv[opti], "-help") == 0)
				return usage(appname, 0);
			else if(strcmp(argv[opti], "-image") == 0) {
				if((opti + 1) > argc)
					return usage(appname, 1);

				imgfile = argv[++opti];
			} else if(strcmp(argv[opti], "-read") == 0) {
				if((opti + 1) > argc || argv[opti+1][0] == '-') {
					dump_info = DI_ALL;
					continue;
				}
				dump_info = parse_distr(argv[++opti]);
			} else if(strcmp(argv[opti], "-sector") == 0) {
				if((opti + 1) > argc)
					return usage(appname, 1);

				sector = atoi(argv[++opti]);
			} else if(strcmp(argv[opti], "-unlink") == 0) {
				unlink(imgfile);
			}
		}
	}

	if(dump_info == 0) {
		if(stat(imgfile, &img_status) == 0 || errno != ENOENT) {
			fprintf(stderr, "%s: It appears either that file exists; ", appname);
			fprintf(stderr, "or I do not have permission to access it. :(\n");
			return 1;
		}

		fd = open(imgfile, O_RDWR | O_CREAT, 0644);
	} else {
		if(stat(imgfile, &img_status) != 0) {
			fprintf(stderr, "%s: Error reading %s. sorry :(\n", appname, imgfile);
			return 1;
		}
		fd = open(imgfile, O_RDWR);
	}

	if(fd < 0) {
		fprintf(stderr, "%s: couldn't open %s\n", appname, imgfile);
		return 1;
	}

	if(dump_info == 0) {
		lseek(fd, FAT12_IMAGE_SIZE - 1, SEEK_SET);
		if(write(fd, "\0", 1) < 0) {
			fprintf(stderr, "%s: couldn't size image file to 1.4M :(\n", appname);
			close(fd);
			unlink(imgfile);
			return 1;
		}
		lseek(fd, 0, SEEK_SET);
	}

	buffer = mmap(NULL, FAT12_IMAGE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	if(buffer == MAP_FAILED) {
		close(fd);
		unlink(imgfile);
		fprintf(stderr, "%s: mmap failed, out of memory?\n", appname);
		perror(appname);
		return errno;
	}

	if(dump_info != 0) {
		report_info(buffer, (dump_info << 8) | sector);

		munmap(buffer, FAT12_IMAGE_SIZE);
		close(fd);

		return 0;
	}

	root_clus_entry = ROOT_CLUSTER(buffer);
	path = ROOT_DIR(buffer);

	build_superblock((struct fat12_superblock *) buffer);

	/* Build a cluster root @ sector 1 */
	build_cluster_root(root_clus_entry);

	/* Build some files... */

	build_file(
		/* location of root cluster. */
		root_clus_entry,
		path,	/* the directory location */
		ATTR_ARCHIVE,	/* attributes */
		"readme.txt",	/* the name of the file */
		/* the text to be found _IN_ the file */
		"This image was built with a pathetic little image building tool :D\n"
	);

	build_file(
		root_clus_entry,
		path,
		ATTR_ARCHIVE,
		"hi.txt",
		"It's me! Wazzup???\n"
	);

	/* Backup tree??? ..*shrug* this is how fs.msdos handles it
	 according to all my hexedit-age.. so this is how I'm gonna
	 do it. :) */
	memcpy(NEXT_CLUSTER(buffer), (void *) root_clus_entry,
		sizeof(struct fat12_cluster_entry));

	munmap(buffer, FAT12_IMAGE_SIZE);
	close(fd);

	fprintf(stderr, "%s: image built.. enjoy :)\n", appname);
	return 0;
}

int usage(const char *app, int code)
{
	fprintf(stderr, "Usage:\n\t%s [options]\n\n", app);
	fprintf(stderr, "Where [options] is one of:\n");
	fprintf(stderr, "\t-help\tprints this help, and exits.\n");
	fprintf(stderr, "\t-image <file>\tThe image <file> to use.\n");
	fprintf(stderr, "\t-read\treads some header info from the image file.\n");
	fprintf(stderr, "\t-unlink\tdeletes the image file first.\n");

	fputc('\n', stderr);
	return code;
}

void build_superblock(struct fat12_superblock *sprblk)
{
	char code[] = 
		"\x31\xc0"	/* xor ax, ax */
		"\x8e\xd8"	/* mov ds, ax */
		"\xbe\x36\x7c"	/* mov si, message ; boot memory is @ 0x7C00 */
		"\xb4\x0e"	/* mov ah, 0x0e */
		"\x30\xff"	/* xor bh, bh */
		"\xb3\x07"	/* mov bl, 0x07 */
		"\xac"	/* lodsb */
		"\x08\xc0"	/* or al, al */
		"\x74\x04"	/* jz short +4 */
		"\xcd\x10"	/* int 0x10 */
		"\xeb\xf7"	/* jmp short -9 ; back to lodsb */
		"\xeb\xfe"	/* jmp $ */
		"This disk is not a Bootable Disk.\n";	/* message db ... */

	memset(sprblk, 0, sizeof(struct fat12_superblock));

	/* jmp short +30 : nop */
	strcpy(sprblk->jump_code, "\xeb\x1c\x90");
	strcpy(sprblk->oem_id, "  FAT12  ");

	sprintf(sprblk->bootstrap, code);
	sprblk->signature = 0xaa55;

	sprblk->bytes_per_sector = 512;	/* 512 bytes per sector */
	sprblk->sectors_per_cluster = 1;	/* only 1 sector per cluster */
	sprblk->reserved_clusters = 1;	/* Only 1, and that's the boot loader/fat12 header */
	sprblk->fat_copies = 2;		/* ... */
	sprblk->root_dir_entries = 224;		/* ... */
	sprblk->sector_count = 2880;	/* number of sectors in our fs */
	sprblk->media_type = 0xf0;	/* 3.5" floppy */
	sprblk->sectors_per_fat = 9;	/* 9 sectors per fat: log2(512) */
	sprblk->sectors_per_track = 12;	/* 12 sectors per track */
	sprblk->heads = 2;	/* a 3.5" floppy as 2 heads :) */
	sprblk->hidden_sectors = 0;		/* number of hidden sectors, we aint got none! */
}

void build_cluster_root(struct fat12_cluster_entry *root)
{
	root->media_type = 0xf0;
	root->ignored = 0xff;
	root->eof = 0xff;
}

void report_info(const char *buffer, int di_flags)
{
	short s, sector = (di_flags & 0xff);
	struct fat12_superblock *sprblk = (struct fat12_superblock *) buffer;
	struct fat12_cluster_entry *cluster = ROOT_CLUSTER(buffer);
	struct fat12_dir_entry *dir = ROOT_DIR(buffer);

	di_flags = ((di_flags & 0xff00) >> 8);

	if((di_flags & DI_DIRENTRY)) {
		dir = (struct fat12_dir_entry *) (buffer + FAT_SECTOR(sector));

		while(dir->name[0] != 0x00) {
			if(dir->name[0] == FLAG_DELETED) {
				++dir;
				continue;
			}

			fprintf(stderr, "dir->name: ");
			s = 0;
			while(dir->name[s] != ' ' && s < sizeof(dir->name))
				fputc(dir->name[s++], stderr);

			s = 0;
			if(dir->ext[s] != ' ') {
				fputc('.', stderr);
				while(dir->ext[s] != ' ' && s < sizeof(dir->ext))
					fputc(dir->ext[s++], stderr);
			}

			fprintf(stderr, "\ndir->attr: ");

			if(dir->attr == ATTR_NONE)
				fprintf(stderr, "ATTR_NONE ");
			else if(IS_RDONLY(dir->attr))
				fprintf(stderr, "ATTR_RDONLY ");
			else if(IS_HIDDEN(dir->attr))
				fprintf(stderr, "ATTR_HIDDEN ");
			else if(IS_SYSTEM(dir->attr))
				fprintf(stderr, "ATTR_SYSTEM ");
			else if(IS_VLABEL(dir->attr))
				fprintf(stderr, "ATTR_VLABLE ");
			else if(IS_SUBDIR(dir->attr))
				fprintf(stderr, "ATTR_SUBDIR ");
			else if(IS_ARCHIVE(dir->attr))
				fprintf(stderr, "ATTR_ARCHIVE ");

			fprintf(stderr, "\ndir->time: %d:%d:%d",
				(dir->time >> 11),
				((dir->time >> 5) & 0x3f),
				((dir->time) & 0x1f));

			fprintf(stderr, "\ndir->date: %d-%d-%d",
				(dir->date >> 9) + 1980,
				((dir->date >> 5) & 0x0f),
				((dir->date) & 0x1f));

			fprintf(stderr, "\ndir->start_cluster: %d", dir->start_cluster);
			fprintf(stderr, "\ndir->size: %d\n", dir->size);

			++dir;
		}
	}
}

void build_file(struct fat12_cluster_entry *table,
		struct fat12_dir_entry *file, unsigned char attributes,
		char *filename, char *data)
{
	short free_cluster, i, data_offset = 0x4200;
	char *ptr;

	free_cluster = find_free_cluster(table);

	if(free_cluster < 0)
		return;

	/* this is a hack.. */
	if(table->cluster[1] != 0xf0) {
		table->cluster[1] = 0xf0;
		table->cluster[2] = 0xff;
	}

	if(free_cluster > 3) {
		file += (free_cluster - 3);
		data_offset += (0x200 * (free_cluster - 3));
	}

	memset(file->name, ' ', sizeof(file->name));
	memset(file->ext, ' ', sizeof(file->ext));

	ptr = strchr(filename, '.');
	if(ptr != NULL) ++ptr;

	for(i = 0; filename[i] && filename[i] != '.' && i < sizeof(file->name); ++i)
		file->name[i] = toupper(filename[i]);

	for(i = 0; ptr[i] && i < sizeof(file->ext); ++i)
		file->ext[i] = toupper(ptr[i]);

	file->attr = attributes;
	file->time = 0xb860;	/* 23:03:00 */
	file->date = 0x362f;	/* 2007-09-15 */
	file->start_cluster = free_cluster;
	file->size = (data? strlen(data): 0);

	if(IS_SUBDIR(attributes)) {
		struct fat12_dir_entry *dir = (struct fat12_dir_entry *) table + data_offset;

		dir->attr = ATTR_SUBDIR;
		dir->time = file->time;
		dir->date = file->date;
		dir->start_cluster = ROOT_DIR_CLUSTER;
	}

	strcpy((char *) table + data_offset, data);

	/* cheap hack... */
	table->cluster[free_cluster] = 0xff;
}

int parse_distr(char *typ)
{
	int r = 0;
	char *ptr = typ;

	while(ptr) {
		ptr = strchr(typ, ',');
		if(ptr) *ptr++ = 0;

		r |= lookup_distr(typ);
		typ = ptr;
	}
	return r;	
}

int lookup_distr(char *ds)
{
	if(strcmp(ds, "superblock") == 0)
		return DI_SUPERBLOCK;
	else if(strcmp(ds, "direntry") == 0)
		return DI_DIRENTRY;
	else if(strcmp(ds, "all") == 0)
		return DI_ALL;

	return 0;
}

short find_free_cluster(struct fat12_cluster_entry *table)
{
	short free_cluster = 3;	/* cheap hack! */
	int max = sizeof(table->cluster);

	while(free_cluster < max) {
		if(table->cluster[free_cluster] == CLUSTER_FREE)
			break;

		++free_cluster;
	}

	if(free_cluster >= max)
		return -1;

	return free_cluster;
}
