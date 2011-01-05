#ifndef H_BMP_
# define H_BMP_

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

typedef struct bmphdr_t
{
	unsigned short bfType;
	unsigned int bfSize;
	unsigned int bfReserved;
	unsigned int bfOffBits;
} __attribute__((packed)) bmphdr_t;

typedef struct bmpinfo_t
{
	unsigned int biSize;
	unsigned int biWidth;
	unsigned int biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	unsigned int biXPelsPerMeter;
	unsigned int biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
} __attribute__((packed)) bmpinfo_t;

typedef struct rgb_t
{
	unsigned char blue, green, red;
} __attribute__((packed)) rgb_t;

typedef struct rgbquad_t
{
	unsigned char rgbBlue;
	unsigned char rbgGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
} __attribute__((packed)) rgbquad_t;

typedef struct bmpfile_t
{
	bmphdr_t file;
	bmpinfo_t info;
} __attribute__((packed)) bmpfile_t;

# ifdef __cplusplus
}
# endif /* __cplusplus */
#endif	/* !H_BMP_ */
