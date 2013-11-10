/*
 * just messing around with sd(4)
 */

#include <stdio.h>
#include <string.h>
#include <linux/hdreg.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define DEFAULT_DEVICE		"/dev/sda"

int check_device(char *dev)
{
	struct stat buf;

	if(stat(dev, &buf) < 0) {
		fprintf(stderr, "Failed to stat %s: %s\n",
				dev, strerror(errno));
		return 1;
	}

	if(S_ISBLK(buf.st_mode) == 0) {
		fprintf(stderr, "%s: Not a block device.\n", dev);
		return 1;
	}

	return 0;
}


int main(int argc, char *argv[])
{
	char *device = DEFAULT_DEVICE;
	struct hd_geometry geo;
	long size;
	int fd, e;

	if(argc == 1)
		fprintf(stderr, "No device specified, using "
				DEFAULT_DEVICE "\n");
	else
		device = argv[1];

	if(check_device(device) != 0)
		return 1;

	fd = open(device, O_RDONLY);
	if(fd < 0) {
		fprintf(stderr, "Failed to open: %s (%s)\n",
				device, strerror(errno));
		return errno;
	}

	if(ioctl(fd, HDIO_GETGEO, &geo) < 0) {
		perror("ioctl(2)");

		e = errno;
		close(fd);

		return e;
	}

	if(ioctl(fd, BLKGETSIZE, &size) < 0) {
		perror("ioctl(2)");

		e = errno;
		close(fd);

		return e;
	}

	close(fd);

	printf("Heads: 0x%04x\nSectors: 0x%04x\nCylinders: 0x%04x\n"
			"Start: 0x%08lx\n", geo.heads, geo.sectors,
			geo.cylinders, geo.start);
	printf("Device Size: 0x%04lx (in sectors)\n", size);

	return 0;
}
