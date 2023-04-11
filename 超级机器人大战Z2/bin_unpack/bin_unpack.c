/*
用于解包与解压bin文件
made by Darkness-TX
2023.04.08
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <io.h>
#include <direct.h>
#include <Windows.h>
#include <locale.h>
#include "SRWZ_LZ.h"

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;

void Unpack(char* fname)
{
	FILE *src = NULL, *dst = NULL, *info = NULL;
	unit32 savepos = 0, filesize = 0, startoffset = 0, size = 0;
	unit8 dstname[MAX_PATH], *cdata = NULL, *udata = NULL;
	src = fopen(fname, "rb");
	fseek(src, 0, SEEK_END);
	filesize = ftell(src);
	fseek(src, 0, SEEK_SET);
	printf("%s size:0x%X\n", fname, filesize);
	sprintf(dstname, "%s_info.txt", fname);
	info = fopen(dstname, "wt,ccs=UNICODE");
	sprintf(dstname, "%s_unpack", fname);
	_mkdir(dstname);
	_chdir(dstname);
	cdata = malloc(filesize);
	fread(cdata, filesize, 1, src);
	fclose(src);
	while (savepos < filesize)
	{
		startoffset = get_start_offset(cdata + savepos);
		size = get_size(cdata + savepos);
		fwprintf(info, L"%08d.bin offset:0x%X startoffset:0x%X size:0x%X\n", FileNum, savepos, startoffset, size);
		printf("\tfilenum:%08d offset:0x%X startoffset:0x%X size:0x%X\n", FileNum, savepos, startoffset, size);
		savepos += startoffset;
		udata = malloc(size);
		savepos += LZdecompressmod(cdata + savepos, udata, size);
		sprintf(dstname, "%08d.bin", FileNum);
		FileNum++;
		dst = fopen(dstname, "wb");
		fwrite(udata, size, 1, dst);
		free(udata);
		fclose(dst);
		if (savepos % 0x10 != 0)
			savepos += 0x10 - (savepos % 0x10);
	}
	free(cdata);
	fclose(info);
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-超级机器人大战Z2\n用于解包与解压bin文件。\nby Darkness-TX 2023.04.08\n\n");
	Unpack(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}