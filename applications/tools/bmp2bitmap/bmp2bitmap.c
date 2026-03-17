#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum _OSD_COLOR_FMT_E {
	OSD_COLOR_FMT_RGB444 = 0,
	OSD_COLOR_FMT_RGB4444 = 1,
	OSD_COLOR_FMT_RGB555 = 2,
	OSD_COLOR_FMT_RGB565 = 3,
	OSD_COLOR_FMT_RGB1555 = 4,
	OSD_COLOR_FMT_RGB888 = 6,
	OSD_COLOR_FMT_RGB8888 = 7,
	OSD_COLOR_FMT_BUTT
} OSD_COLOR_FMT_E;

typedef struct _OSD_RGB_S {
	uint8_t u8B;
	uint8_t u8G;
	uint8_t u8R;
	uint8_t u8Reserved;
} OSD_RGB_S;

typedef struct _OSD_SURFACE_S {
	OSD_COLOR_FMT_E enColorFmt; /* color format */
	uint16_t u16Height; /* operation height */
	uint16_t u16Width; /* operation width */
	uint16_t u16Stride; /* surface stride */
	uint16_t u16Reserved;
} OSD_SURFACE_S;

typedef struct _Logo {
	uint32_t width; /* out */
	uint32_t height; /* out */
	uint32_t stride; /* in */
	uint8_t *pRGBBuffer; /* in/out */
} OSD_LOGO_T;

typedef struct _BITMAPINFOHEADER {
	uint32_t biSize;
	uint32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
} OSD_BITMAPINFOHEADER;

typedef struct _BITMAPFILEHEADER {
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
} OSD_BITMAPFILEHEADER;

typedef struct _RGBQUAD {
	uint8_t rgbBlue;
	uint8_t rgbGreen;
	uint8_t rgbRed;
	uint8_t rgbReserved;
} OSD_RGBQUAD;

typedef struct _BITFIELD {
	uint32_t r_mask;
	uint32_t g_mask;
	uint32_t b_mask;
	uint32_t a_mask;
} OSD_BITFIELD;

typedef struct _BITMAPINFO {
	OSD_BITMAPINFOHEADER bmiHeader;
	union {
		OSD_RGBQUAD bmiColors[1];
		OSD_BITFIELD bitfield;
	};
} OSD_BITMAPINFO;

typedef struct _OSD_COMPONENT_INFO_S {
	int32_t alen;
	int32_t rlen;
	int32_t glen;
	int32_t blen;
} OSD_COMP_INFO;

OSD_COMP_INFO s_OSDCompInfo[OSD_COLOR_FMT_BUTT] = {
	{ 0, 4, 4, 4 }, /*RGB444*/
	{ 4, 4, 4, 4 }, /*ARGB4444*/
	{ 0, 5, 5, 5 }, /*RGB555*/
	{ 0, 5, 6, 5 }, /*RGB565*/
	{ 1, 5, 5, 5 }, /*ARGB1555*/
	{ 0, 0, 0, 0 }, /*RESERVED*/
	{ 0, 8, 8, 8 }, /*RGB888*/
	{ 8, 8, 8, 8 } /*ARGB8888*/
};
static int32_t bmp_width = 0, bmp_height = 0;

uint16_t OSD_MAKECOLOR_U16(uint8_t r, uint8_t g, uint8_t b, OSD_COMP_INFO compinfo)
{
	uint8_t r1, g1, b1;
	uint16_t pixel = 0;
	uint32_t tmp = 15;

	r1 = g1 = b1 = 0;
	r1 = r >> (8 - compinfo.rlen);
	g1 = g >> (8 - compinfo.glen);
	b1 = b >> (8 - compinfo.blen);
	while (compinfo.alen) {
		pixel |= (1 << tmp);
		tmp--;
		compinfo.alen--;
	}

	pixel |= (b1 | (g1 << compinfo.blen) | (r1 << (compinfo.blen + compinfo.glen)));
	return pixel;
}

uint16_t OSD_MAKECOLOR_U16_A(uint8_t a, uint8_t r, uint8_t g, uint8_t b, OSD_COMP_INFO compinfo)
{
	uint8_t a1, r1, g1, b1;
	uint16_t pixel = 0;

	a1 = r1 = g1 = b1 = 0;
	a1 = a >> (8 - compinfo.alen);
	r1 = r >> (8 - compinfo.rlen);
	g1 = g >> (8 - compinfo.glen);
	b1 = b >> (8 - compinfo.blen);

	pixel = (b1 | (g1 << compinfo.blen) | (r1 << (compinfo.blen + compinfo.glen))
		| (a1 << (compinfo.rlen + compinfo.glen + compinfo.blen)));
	return pixel;
}

uint8_t find_bitshift(uint32_t mask)
{
	uint8_t j;

	for (j = 0; !((mask >> j) & 1) && j < 32; ++j)
		;

	return j;
}

int32_t GetBmpInfo(const char *filename, OSD_BITMAPFILEHEADER *pBmpFileHeader, OSD_BITMAPINFO *pBmpInfo)
{
	FILE *pFile;
	uint16_t bfType;

	if (filename == NULL) {
		printf("OSD_LoadBMP: filename=NULL\n");
		return -1;
	}

	pFile = fopen((char *)filename, "rb");
	if (pFile == NULL) {
		printf("Open file failed:%s!\n", filename);
		return -1;
	}

	fread(&bfType, 1, sizeof(bfType), pFile);
	if (bfType != 0x4d42) {
		printf("not bitmap file\n");
		fclose(pFile);
		return -1;
	}

	fread(pBmpFileHeader, 1, sizeof(OSD_BITMAPFILEHEADER), pFile);
	fread(pBmpInfo, 1, sizeof(OSD_BITMAPINFO), pFile);
	fclose(pFile);

	printf("bmp width(%d) height(%d) bpp(%d) compression(%d)\n"
		, pBmpInfo->bmiHeader.biWidth, pBmpInfo->bmiHeader.biHeight, pBmpInfo->bmiHeader.biBitCount
		, pBmpInfo->bmiHeader.biCompression);

	bmp_height = pBmpInfo->bmiHeader.biHeight;
	bmp_width = pBmpInfo->bmiHeader.biWidth;

	if (pBmpInfo->bmiHeader.biCompression == 3)
		printf("bitmask a(%#x) r(%#x) g(%#x) b(%#x)\n", pBmpInfo->bitfield.a_mask
			, pBmpInfo->bitfield.r_mask, pBmpInfo->bitfield.g_mask, pBmpInfo->bitfield.b_mask);
	return 0;
}

int32_t LoadBMP(const char *filename, OSD_LOGO_T *pVideoLogo, OSD_COLOR_FMT_E enFmt, bool update_logo)
{
	FILE *pFile;
	uint16_t i, j;

	uint32_t w, h;
	uint16_t Bpp;

	OSD_BITMAPFILEHEADER bmpFileHeader;
	OSD_BITMAPINFO bmpInfo;

	uint8_t *pOrigBMPBuf;
	uint8_t *pRGBBuf;
	uint32_t stride;
	uint8_t r, g, b, a;
	uint8_t *pStart;
	uint16_t *pDst;
	uint32_t pxl;
	uint8_t color_shifts[4] = { 0 };

	if (filename == NULL) {
		printf("OSD_LoadBMP: filename=NULL\n");
		return -1;
	}

	if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0) {
		return -1;
	}

	Bpp = bmpInfo.bmiHeader.biBitCount / 8;
	if (Bpp < 2) {
		/* only support 1555.8888  888 bitmap */
		printf("bitmap format not supported!\n");
		return -1;
	}
	if (bmpInfo.bmiHeader.biCompression == 3) {
		if ((Bpp != 2) && (Bpp != 4)) {
			printf("bitmap bitfiled format bpp(%d) not supported!\n", Bpp);
			return -1;
		}

		color_shifts[0] = find_bitshift(bmpInfo.bitfield.a_mask);
		color_shifts[1] = find_bitshift(bmpInfo.bitfield.r_mask);
		color_shifts[2] = find_bitshift(bmpInfo.bitfield.g_mask);
		color_shifts[3] = find_bitshift(bmpInfo.bitfield.b_mask);
	}

	if ((bmpInfo.bmiHeader.biCompression != 0) && (bmpInfo.bmiHeader.biCompression != 3)) {
		printf("only support non-compressed or bitfile bitmap file!\n");
		return -1;
	}

	if (bmpInfo.bmiHeader.biHeight < 0) {
		printf("bmpInfo.bmiHeader.biHeight < 0\n");
		return -1;
	}

	pFile = fopen((char *)filename, "rb");
	if (pFile == NULL) {
		printf("Open file failed:%s!\n", filename);
		return -1;
	}

	w = bmpInfo.bmiHeader.biWidth;
	h = ((bmpInfo.bmiHeader.biHeight > 0) ? bmpInfo.bmiHeader.biHeight : (-bmpInfo.bmiHeader.biHeight));

	stride = w * Bpp;
#if 1
	if (stride % 4) {
		stride = (stride & 0xfffc) + 4;
	}
#endif

	if (update_logo) {
		pVideoLogo->width = w;
		pVideoLogo->height = h;
		pVideoLogo->stride = (enFmt >= OSD_COLOR_FMT_RGB888) ? w << 2 : w << 1;
	}

	/* RGB8888 or RGB1555 */
	pOrigBMPBuf = (uint8_t *)malloc(h * stride);
	if (pOrigBMPBuf == NULL) {
		printf("not enough memory to malloc!\n");
		fclose(pFile);
		return -1;
	}

	pRGBBuf = pVideoLogo->pRGBBuffer;

	if (h > pVideoLogo->height) {
		printf("Bitmap's height(%d) is bigger than canvas's height(%d). Load bitmap error!\n", h,
		       pVideoLogo->height);
		free(pOrigBMPBuf);
		fclose(pFile);
		return -1;
	}

	if (w > pVideoLogo->width) {
		printf("Bitmap's width(%d) is bigger than canvas's width(%d). Load bitmap error!\n", w,
		       pVideoLogo->width);
		free(pOrigBMPBuf);
		fclose(pFile);
		return -1;
	}

	fseek(pFile, bmpFileHeader.bfOffBits, 0);
	if (fread(pOrigBMPBuf, 1, h * stride, pFile) != (h * stride)) {
		printf("fread (%d*%d)error!line:%d\n", h, stride, __LINE__);
		perror("fread:");
	}

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			if (Bpp == 3) {
				switch (enFmt) {
				case OSD_COLOR_FMT_RGB444:
				case OSD_COLOR_FMT_RGB555:
				case OSD_COLOR_FMT_RGB565:
				case OSD_COLOR_FMT_RGB1555:
				case OSD_COLOR_FMT_RGB4444:
					/* start color convert */
					pStart = pOrigBMPBuf + ((h - 1) - i) * stride + j * Bpp;
					pDst = (uint16_t *)(pRGBBuf + i * pVideoLogo->stride + j * 2);
					b = *(pStart);
					g = *(pStart + 1);
					r = *(pStart + 2);
					*pDst = OSD_MAKECOLOR_U16(r, g, b, s_OSDCompInfo[enFmt]);

					break;

				case OSD_COLOR_FMT_RGB888:
				case OSD_COLOR_FMT_RGB8888:
					memcpy(pRGBBuf + i * pVideoLogo->stride + j * 4,
					       pOrigBMPBuf + ((h - 1) - i) * stride + j * Bpp, Bpp);
					*(pRGBBuf + i * pVideoLogo->stride + j * 4 + 3) = 0xff; /*alpha*/
					break;

				default:
					printf("file(%s), line(%d), no such format!\n", __FILE__, __LINE__);
					break;
				}
			} else if ((Bpp == 2) || (Bpp == 4)) {
				pStart = pOrigBMPBuf + ((h - 1) - i) * stride + j * Bpp;
				pDst = (uint16_t *)(pRGBBuf + i * pVideoLogo->stride + j * 2);
				pxl = *(uint32_t *)pStart;
				a = (pxl & bmpInfo.bitfield.a_mask) >> color_shifts[0];
				r = (pxl & bmpInfo.bitfield.r_mask) >> color_shifts[1];
				g = (pxl & bmpInfo.bitfield.g_mask) >> color_shifts[2];
				b = (pxl & bmpInfo.bitfield.b_mask) >> color_shifts[3];
				if (i == 0 && j == 0)
				printf("Func: %s, line:%d, Bpp: %d, bmp stride: %d, Canvas stride: %d, h:%d, w:%d.\n",
				    __func__, __LINE__, Bpp, stride, pVideoLogo->stride, i, j);
				*pDst = OSD_MAKECOLOR_U16_A(a, r, g, b, s_OSDCompInfo[enFmt]);
			}
		}
	}

	free(pOrigBMPBuf);
	pOrigBMPBuf = NULL;

	fclose(pFile);
	return 0;
}

char *GetExtName(char *filename)
{
	char *pret = NULL;
	uint32_t fnLen;

	if (filename == NULL) {
		printf("filename can't be null!");
		return NULL;
	}

	fnLen = strlen(filename);
	while (fnLen) {
		pret = filename + fnLen;
		if (*pret == '.')
			return (pret + 1);

		fnLen--;
	}

	return pret;
}

int32_t LoadImageEx(const char *filename, OSD_LOGO_T *pVideoLogo, OSD_COLOR_FMT_E enFmt)
{
	char *ext = GetExtName((char *)filename);

	if (ext == NULL) {
		printf("LoadImageEx error!\n");
		return -1;
	}

	if (strcmp(ext, "bmp") == 0) {
		if (LoadBMP(filename, pVideoLogo, enFmt, true) != 0) {
			printf("OSD_LoadBMP error!\n");
			return -1;
		}
	} else {
		printf("not supported image file!\n");
		return -1;
	}

	return 0;
}

int32_t LoadCanvasEx(const char *filename, OSD_LOGO_T *pVideoLogo, OSD_COLOR_FMT_E enFmt)
{
	char *ext = GetExtName((char *)filename);

	if (ext == NULL) {
		printf("LoadCanvasEx error!\n");
		return -1;
	}

	if (strcmp(ext, "bmp") == 0) {
		if (LoadBMP(filename, pVideoLogo, enFmt, false) != 0) {
			printf("OSD_LoadBMP error!\n");
			return -1;
		}
	} else {
		printf("not supported image file!\n");
		return -1;
	}

	return 0;
}

int32_t CreateSurfaceByBitMap(const char *pszFileName, OSD_SURFACE_S *pstSurface, uint8_t *pu8Virt)
{
	OSD_LOGO_T stLogo;

	stLogo.pRGBBuffer = pu8Virt;
	if (LoadImageEx(pszFileName, &stLogo, pstSurface->enColorFmt) < 0) {
		printf("load bmp error!\n");
		return -1;
	}

	pstSurface->u16Height = stLogo.height;
	pstSurface->u16Width = stLogo.width;
	pstSurface->u16Stride = stLogo.stride;

	return 0;
}

int32_t CreateSurfaceByCanvas(const char *pszFileName, OSD_SURFACE_S *pstSurface, uint8_t *pu8Virt
			     , uint32_t u32Width, uint32_t u32Height, uint32_t u32Stride)
{
	OSD_LOGO_T stLogo;

	stLogo.pRGBBuffer = pu8Virt;
	stLogo.width = u32Width;
	stLogo.height = u32Height;
	stLogo.stride = u32Stride;
	if (LoadCanvasEx(pszFileName, &stLogo, pstSurface->enColorFmt) < 0) {
		printf("load bmp error!\n");
		return -1;
	}

	pstSurface->u16Height = u32Height;
	pstSurface->u16Width = u32Width;
	pstSurface->u16Stride = u32Stride;

	return 0;
}

static void usage()
{
    printf("==== Command ====\n");
	printf("./bmp2bitmap -f logo.bmp -b 0xffff hide white color\n");
    printf("-b: + color pixelformat argb 1555\n");
}

int32_t main(int32_t argc, char *argv[])
{
	if (argc != 5) {
        usage();
        exit(-1);
    }
    int32_t c;
    int32_t bFil = 0;
    char filename[256] = {0};
    char outfilename[32] = "bitmap.h";
    uint8_t *pData = NULL;
    int32_t len = 0;
    opterr = 0;

	while ((c = getopt(argc, argv, "f:b:")) != -1) {
        switch (c) {
            case 'f':
                snprintf(filename, sizeof(filename), "%s", optarg);
                break;
            case 'b':
                sscanf(optarg, "%x", &bFil);
                break;
            default:
                printf("Invalid option : %c\n", c);
                usage();
                abort();
        }
    }

	if(strlen(filename) == 0)
    {
        usage();
        abort();
    }
	OSD_SURFACE_S Surface;
	OSD_BITMAPFILEHEADER bmpFileHeader;
	OSD_BITMAPINFO bmpInfo;
	int32_t bpp = 2;

	if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0) {
		printf("GetBmpInfo err!\n");
		return -1;
	}

	if(bmp_height == 0 || bmp_width == 0 ) {
		printf("bmp_width: %d, bmp_height: %d\n", bmp_width, bmp_height);
		printf("bmp_height and bmp_width cannot be 0!\n");
		return -1;
	}
    Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
    len = bpp * (bmp_height) * (bmp_width);
	pData = malloc(len);
	if (pData == NULL) {
		printf("malloc osd memroy err!\n");
		return -1;
	}

	CreateSurfaceByBitMap(filename, &Surface, (uint8_t *)(pData));
    int32_t i, j;
    FILE *fp;
    uint16_t *pu16Temp;
    char buf[256];
    fp = fopen(outfilename, "w+");

	fprintf(fp, "#ifndef __CVI_BITMAP_H__\n");
    fprintf(fp, "#define __CVI_BITMAP_H__\n\n");

	fprintf(fp, "#ifdef __cplusplus\n");
	fprintf(fp, "#if __cplusplus\n");
	fprintf(fp, "extern \"C\" {\n");
	fprintf(fp, "#endif\n");
	fprintf(fp, "#endif /* End of #ifdef __cplusplus */\n\n");

	fprintf(fp, "typedef struct bitMap_SIZE_S\n");
	fprintf(fp, "{\n");
	fprintf(fp, "    uint32_t Width;\n");
	fprintf(fp, "	uint32_t Height;\n");
	fprintf(fp, "} BitMapSize;\n\n");
	fprintf(fp, "BitMapSize stBitMapSize = {\n");
	fprintf(fp, "    .Width = %d,\n", bmp_width);
	fprintf(fp, "    .Height = %d\n", bmp_height);
	fprintf(fp, "};\n\n");

    fprintf(fp, "unsigned short pdata[] = {\n");
    pu16Temp = (uint16_t *)pData;
    for (i = 0; i < bmp_height; i++) {
        for (j = 0; j < bmp_width; j++) {
            if (bFil != 0 && bFil == *pu16Temp) {
                *pu16Temp &= 0x7FFF;
            }
            if(j == bmp_width - 1 && i == bmp_height - 1) {
                snprintf(buf, sizeof(buf), "0x%02X\n", *pu16Temp);
            } else {
                if(j == bmp_height - 1) {
                    snprintf(buf, sizeof(buf), "0x%02X,\n", *pu16Temp);
                } else {
                    snprintf(buf, sizeof(buf), "0x%02X,", *pu16Temp);
                }
            }
            fwrite(buf, strlen(buf), 1, fp);
            pu16Temp++;
        }
    }
	fprintf(fp, "};\n\n");

	fprintf(fp, "#ifdef __cplusplus\n");
	fprintf(fp, "#if __cplusplus\n");
	fprintf(fp, "}\n");
	fprintf(fp, "#endif\n");
	fprintf(fp, "#endif /* End of #ifdef __cplusplus */\n\n");

    fprintf(fp, "#endif /* End of #ifndef HEADER_H */\n");

    fclose(fp);

	return 0;
}