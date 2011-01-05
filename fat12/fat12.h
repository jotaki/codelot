#ifndef FAT12_H
# define FAT12_H

/* filesystem definitions */

struct fat12_superblock {
	unsigned char jump_code[3];	/* the jump code */
	unsigned char oem_id[8];	/* the oem id */

	unsigned short bytes_per_sector;	/* number of bytes per sector */
	unsigned char sectors_per_cluster;	/* number of sectors per cluster */
	unsigned short reserved_clusters;	/* number of reserved clusters */

	unsigned char fat_copies;	/* number of FATs */

	unsigned short root_dir_entries;	/* root directory entries */

	unsigned short sector_count;	/* number of sectors */

	unsigned char media_type;	/* The media type */

	unsigned short sectors_per_fat;	/* number of sectors per fat */
	unsigned short sectors_per_track;	/* number of sectors per track */
	unsigned short heads;	/* number heads on this disk. */
	unsigned short hidden_sectors;	/* the hidden sectors */

	unsigned char bootstrap[480];	/* the bootstrap code */
	unsigned short signature;	/* boot signature. (0xaa55) */
} __attribute__ ((packed));

struct fat12_cluster_entry {
	unsigned char media_type;	/* our media type */
	unsigned char ignored;	/* no concern to us. */
	unsigned char eof;	/* eof char */
	unsigned char cluster[509];	/* marks the cluster... */
} __attribute__ ((packed));
	

struct fat12_dir_entry {
	unsigned char name[8];	/* the filename */
	unsigned char ext[3];	/* the extension */

	unsigned char attr; 	/* attributes */

	unsigned char reserved[10]; 	/* reserved */

	unsigned short time;	/* Time
			bits 11-16 = hour, bits 5-11 = minutes, bits 0-4 = doubleseconds */
	unsigned short date;	/* Date
			bits 9-16 = year-since-1980, bits 5-8 = month, bits 0-4 = day */

	unsigned short start_cluster;	/* The starting cluster */
	unsigned int size;
} __attribute__ ((packed));

# define ATTR_NONE  	0x00
# define ATTR_RDONLY	0x01
# define ATTR_HIDDEN	0x02
# define ATTR_SYSTEM	0x04
# define ATTR_VLABEL	0x08
# define ATTR_SUBDIR	0x10
# define ATTR_ARCHIVE	0x20

# define IS_RDONLY(attr)	(((attr) & ATTR_RDONLY) == ATTR_RDONLY)
# define IS_HIDDEN(attr)	(((attr) & ATTR_HIDDEN) == ATTR_HIDDEN)
# define IS_SYSTEM(attr)	(((attr) & ATTR_SYSTEM) == ATTR_SYSTEM)
# define IS_VLABEL(attr)	(((attr) & ATTR_VLABEL) == ATTR_VLABEL)
# define IS_SUBDIR(attr)  	(((attr) & ATTR_SUBDIR) == ATTR_SUBDIR)
# define IS_ARCHIVE(attr)	(((attr) & ATTR_ARCHIVE) == ATTR_ARCHIVE)

# define CLUSTER_FREE	0x000
# define CLUSTER_BAD	0xff7

# define FLAG_DELETED	0xe5

/* misc definitions */

# define FAT12_IMAGE_SIZE	0x168000
# define FAT_SECTOR(x)  	(x) * 512
# define ROOT_DIR(b)	(struct fat12_dir_entry *) ((b) + 0x2600)
# define ROOT_DIR_CLUSTER	0
# define ROOT_CLUSTER(b)	(struct fat12_cluster_entry *) ((b) + 512)
# define NEXT_CLUSTER(b) \
	(struct fat12_cluster_entry *) ((b) + FAT_SECTOR(10))

# define SECTOR_OFFSET(x)	FAT_SECTOR((x) + 0xc)
# define NEXT_DIR_ENTRY(d)	++(d)

# define DOT		".        "
# define DOTDOT 	"..       "

# define DI_SUPERBLOCK	0x01
# define DI_DIRENTRY	0x02

# define DI_ALL 	0xff

/* program definitions */
int usage(const char *app, int code);
void build_superblock(struct fat12_superblock *sprblk);
void build_cluster_root(struct fat12_cluster_entry *root);
void report_info(const char *buffer, int di_flags);
void build_file(struct fat12_cluster_entry *table,
		struct fat12_dir_entry *file, unsigned char attributes,
		char *filename, char *data);

int parse_distr(char *typ);
int lookup_distr(char *ds);
short find_free_cluster(struct fat12_cluster_entry *table);

#endif	/* !FAT12_H */
