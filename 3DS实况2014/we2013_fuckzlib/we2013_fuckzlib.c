#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <zlib.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 EndianChange(unit32 Unit32Endian)
{
	unit8 *p;
	p = &Unit32Endian;
	return (unit32)(p[0] << 24) + (unit32)(p[1] << 16) +
		(unit32)(p[2] << 8) + (unit32)p[3];
}

int main(int argc, char *argv[])
{
	unit32 flen, ulen, hflag;
	FILE *OpenFile, *WriteFile;
	unit8 *buf, *ubuf, filename[260];
	printf("project：Helheim-Wii WE 2013\n用于解压zlib压缩的文件，将文件拖到程序上。\nby Darkness-TX 2016.6.25\n");
	fopen_s(&OpenFile, argv[1], "rb");
	if (OpenFile == NULL)
	{
		printf("打开文件失败\n");
		system("pause");
		exit(0);
	}
	sprintf(filename, "%s.unc", argv[1]);
	fread(&hflag, 4, 1, OpenFile);
	if (hflag == 0x10104)
	{
		fread(&flen, 4, 1, OpenFile);
		flen = EndianChange(flen);
		fread(&ulen, 4, 1, OpenFile);
		ulen = EndianChange(ulen);
		fseek(OpenFile, 0x14, SEEK_CUR);
		fopen_s(&WriteFile, filename, "wb");
		buf = malloc(flen);
		ubuf = malloc(ulen);
		fread(buf, flen, 1, OpenFile);
		uncompress(ubuf, &ulen, buf, flen);
		fwrite(ubuf, ulen, 1, WriteFile);
		free(buf);
		free(ubuf);
		fclose(WriteFile);
		printf("\t%s flag:0x%X 文件大小：0x%X 解压大小：0x%X\n", filename, hflag, flen + 0x20, ulen);
	}
	else
		printf("\t%s\n", argv[1]);
	system("pause");
	return 0;
}