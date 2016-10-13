#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include <png.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;
typedef unsigned int   unit32;

int TileOrder[] =
{
	0,   1,   4,   5,   16,   17,   20,   21,
	2,   3,   6,   7,   18,   19,   22,   23,

	8,   9,  12,  13,   24,   25,   28,   29,
	10,  11,  14,  15,   26,   27,   30,   31
};

void WriteFile8(char *fname)
{
	unit8 *data, *pixeldata, buff;
	FILE *OpenFile, *fp;
	unit32 i,pixel[256];
	fopen_s(&OpenFile, fname, "rb");
	printf("%s\n", fname);
	fseek(OpenFile, 0x80, SEEK_SET);
	for (i = 0; i < 256; i++)
		fread(&pixel[i], 4, 1, OpenFile);
	fopen_s(&fp, "32.bin", "wb");
	for (i = 0; i < 0x10000; i++)
	{
		fread(&buff, 1, 1, OpenFile);
		fwrite(&pixel[buff], 4, 1, fp);
	}
}

void WriteFile4(char *fname)
{
	unit8 *data, *pixeldata, buff;
	FILE *OpenFile, *fp;
	unit32 i, pixel[16];
	fopen_s(&OpenFile, fname, "rb");
	printf("%s\n", fname);
	fseek(OpenFile, 0x80, SEEK_SET);
	fopen_s(&fp, "fex.bin", "wb");
	for (i = 0; i < 16; i++)
		fread(&pixel[i], 4, 1, OpenFile);
	fopen_s(&fp, "fex.bin", "wb");
	for (i = 0; i < 0x10000; i++)
	{
		fread(&buff, 1, 1, OpenFile);
		fwrite(&pixel[buff], 4, 1, fp);
	}
}
int main(int argc, char *argv[])
{
	printf("project：Helheim-3DS WE 2014\n用于将文件转换为png图片，文件放在一个文件夹里，将文件夹拖到程序上\nby Darkness-TX 2016.4.21\nEmail：1545492547@qq.com\n\n");
	WriteFile8(argv[1]);
	printf("已完成，总文件数1\n");
	system("pause");
	return 0;
}