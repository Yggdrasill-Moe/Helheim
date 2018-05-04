/*
用于解包文件头为HFU2的文件
made by Darkness-TX
2018.05.03
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;//总文件数，初始计数为0

struct hfu2_header
{
	char magic[4];
	unit32 filenum;
	unit32 fileoffset;
	unit32 count;//为1
}HFU2_Header;

struct index
{
	unit32 offset;
	unit32 comp_size;
	unit32 uncomp_size;
	unit32 crc;//大概
}Index[4000];

void ReadIndex(FILE *src)
{
	unit32 i = 0;
	fread(HFU2_Header.magic, 4, 1, src);
	fread(&HFU2_Header.filenum, 4, 1, src);
	fread(&HFU2_Header.fileoffset, 4, 1, src);
	fread(&HFU2_Header.count, 4, 1, src);
	if (strncmp(HFU2_Header.magic, "HFU2", 4) != 0)
	{
		printf("文件头不是HFU2\n");
		system("pause");
		exit(0);
	}
	printf("filenum:0x%X fileoffset:0x%X count:0x%X\n", HFU2_Header.filenum, HFU2_Header.fileoffset, HFU2_Header.count);
	for (i = 0; i < HFU2_Header.filenum; i++)
	{
		fread(&Index[i].offset, 4, 1, src);
		Index[i].offset += HFU2_Header.fileoffset;
		fread(&Index[i].comp_size, 4, 1, src);
		fread(&Index[i].uncomp_size, 4, 1, src);
		fread(&Index[i].crc, 4, 1, src);
	}
	FileNum = HFU2_Header.filenum;
}

void UnpackFile(char *fname)
{
	FILE *src, *dst;
	char dstname[260];
	unit8 *data = NULL;
	src = fopen(fname, "rb");
	sprintf(dstname, "%s_unpack", fname);
	printf("%s ", fname);
	ReadIndex(src);
	_mkdir(dstname);
	_chdir(dstname);
	for (unit32 i = 0; i < FileNum; i++)
	{
		sprintf(dstname, "%08d.UFFA", i);
		printf("%s offset:0x%X size:0x%X crc:0x%X\n", dstname, Index[i].offset, Index[i].comp_size, Index[i].crc);
		fseek(src, Index[i].offset, SEEK_SET);
		data = malloc(Index[i].comp_size);
		fread(data, Index[i].comp_size, 1, src);
		dst = fopen(dstname, "wb");
		fwrite(data, Index[i].comp_size, 1, dst);
		free(data);
		fclose(dst);
	}
	fclose(src);
}

int main(int argc, char *argv[])
{
	printf("project：Helheim-魔力家族\n用于解包文件头为HFU2的文件，将文件拖到程序上。\nby Darkness-TX 2018.05.03\n");
	UnpackFile(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}