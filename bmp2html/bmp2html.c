#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "bmp.h"

#define RGB_R(r, i) (r[i])->red
#define RGB_G(r, i) (r[i])->green
#define RGB_B(r, i) (r[i])->blue

rgb_t *read_bmp(FILE *bmp, bmpfile_t *file);
void bmpinfo_file(bmphdr_t *hdr);
void bmpinfo_info(bmpinfo_t *info);
unsigned long mkcolor(int r, int g, int b, char *output);

int main(int argc, char *argv[])
{
	if(argc < 3) {
		printf("Usage: %s <picture.bmp> <output.html>\n", argv[0]);
		return 1;
	}

	FILE *bmp = fopen(argv[1], "r");
	if(!bmp) {
		perror(argv[1]);
		return 1;
	}

	bmpfile_t bf;
	bmpinfo_t *info = &bf.info;

	rgb_t *rgb = read_bmp(bmp, &bf);
	if(!rgb) {
		fclose(bmp);
		return 1;
	}

	FILE *htm = fopen(argv[2], "w");
	if(!htm) {
		perror(argv[2]);
		free(rgb);
		fclose(bmp);
		return 1;
	}

	fprintf(htm, "<html><head><title>%s</title><style type=\"text/css\">", argv[1]);
	fprintf(htm, "<!-- td { width: 1px; height: 1px; } --></style></head><body>");
	fprintf(htm, "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\">");

	unsigned int w, h, i = 0;
	char strcolor[32];

	for(h = 0; h < info->biHeight; ++h) {
		fprintf(htm, "<tr>");

		for(w = 0; w < info->biWidth; ++w) {
			mkcolor(RGB_R(&rgb, i), RGB_G(&rgb, i), RGB_B(&rgb, i), strcolor);

			fprintf(htm, "<td bgcolor=\"%s\" />", strcolor);
			++i;
		}
		fprintf(htm, "</tr>");
	}

	fprintf(htm, "</table></body></html>");

	free(rgb);
	fclose(htm);
	fclose(bmp);

	return 0;
}

rgb_t *read_bmp(FILE *bmp, bmpfile_t *file)
{
	bmphdr_t *bh = &file->file;
	bmpinfo_t *bi = &file->info;
	int fd = fileno(bmp);

	if(read(fd, file, sizeof(bmpfile_t)) <= 0) {
		perror("read()");
		return NULL;
	}

	if(bh->bfType != 19778 || bi->biBitCount != 24) {
		fprintf(stderr, "Bitmap file not supported.\n");
		return NULL;
	}

	bmpinfo_file(bh);
	bmpinfo_info(bi);

	fseek(bmp, bh->bfOffBits, SEEK_SET);
	rgb_t *data = (rgb_t *) calloc(sizeof(rgb_t), (bh->bfSize - bh->bfOffBits) + 1);
	if(!data)
		return NULL;

	int len = -1;
	while(read(fd, &data[++len], sizeof(rgb_t)) > 0);

	int i;
	for(i = 0; i < len / 2; ++i) {
		rgb_t temp;

		memcpy(&temp, &data[i], sizeof(rgb_t));
		memcpy(&data[i], &data[len - i - 1], sizeof(rgb_t));
		memcpy(&data[len - i - 1], &temp, sizeof(rgb_t));
	}
	return data;
}

void bmpinfo_file(bmphdr_t *hdr)
{
#define p(x) printf("hdr->bf%s = %u\n", #x, hdr->bf##x)
	p(Type);
	p(Size);
	p(Reserved);
	p(OffBits);
#undef p
}

void bmpinfo_info(bmpinfo_t *info)
{
#define p(x) printf("info->bi%s = %u\n", #x, info->bi##x)
	p(Size);
	p(Width);
	p(Height);
	p(Planes);
	p(BitCount);
	p(Compression);
	p(SizeImage);
	p(XPelsPerMeter);
	p(YPelsPerMeter);
	p(ClrUsed);
	p(ClrImportant);
#undef p
}

unsigned long mkcolor(int r, int g, int b, char *output)
{
	unsigned long rgb = (r << 16) | (g << 8) | (b);

	if(!output)
		return rgb;
#define hi_eq_lo(x)	(((((x) & 0xf0) >> 4) - ((x) & 0x0f)) == 0)
#define lo(x)	((x) & 0x0f)
	if(hi_eq_lo(r) && hi_eq_lo(g) && hi_eq_lo(b)) {
		printf("Found one\n");
		sprintf(output, "#%x%x%x", lo(r), lo(g), lo(b));
	} else
		sprintf(output, "#%06lx", rgb);
#undef hi_eq_lo
#undef lo

	return rgb;
}
