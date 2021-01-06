/*
用于压缩文件头为UF的字库文件
made by Darkness-TX
2019.05.26
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

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FontNum = 0;

struct uf_header
{
	char magic[2];//UF
	unit16 version;
	unit32 count;
	unit8 type;
	unit8 width;
	unit8 height;
	unit8 flag;
	unit32 alpha;
	unit32 filesize;
	unit32 sjis_offset;
	unit32 index_offset;
	unit32 font_offset;
}UF_Header;

struct index
{
	unit8 width;
	unit8 height;
	unit16 size;
	unit32 offset;
}Index[10000];

void ReadIndex(FILE *src)
{
	fread(&UF_Header, sizeof(UF_Header), 1, src);
	if (strncmp(UF_Header.magic, "UF", 2) != 0)
	{
		printf("不支持的类型，文件头不是UF！\n");
		system("pause");
		exit(0);
	}
	fseek(src, UF_Header.index_offset, SEEK_SET);
	for (unit32 i = 0; i < UF_Header.count; i++)
	{
		fread(&Index[i], 8, 1, src);
		Index[i].offset += UF_Header.font_offset;
	}
}

void Compress(char *fname)
{
	FILE *src = NULL, *dsrc = NULL, *dst = NULL;
	unit32 i = 0, filesize = 0, buff = 0;
	unit8 *data = NULL;
	src = fopen(fname, "rb");
	dsrc = fopen("UF.bin", "rb");
	ReadIndex(src);
	data = malloc(UF_Header.font_offset);
	fseek(src, 0, SEEK_SET);
	fread(data, UF_Header.font_offset, 1, src);
	fclose(src);
	dst = fopen("UFn.bin", "wb");
	fwrite(data, UF_Header.font_offset, 1, dst);
	free(data);
	fseek(dsrc, 0, SEEK_END);
	filesize = ftell(dsrc);
	fseek(dsrc, 0, SEEK_SET);
	data = malloc(filesize);
	fread(data, filesize, 1, dsrc);
	fclose(dsrc);
	fwrite(data, filesize, 1, dst);
	fseek(dst, UF_Header.index_offset, SEEK_SET);
	for (i = 0; i < UF_Header.count; i++)
	{
		fwrite(&Index[i].width, 1, 1, dst);
		fwrite(&Index[i].height, 1, 1, dst);
		buff = Index[i].width*Index[i].height / 2;
		fwrite(&buff, 2, 1, dst);
		buff *= i;
		fwrite(&buff, 4, 1, dst);
	}
	fseek(dst, 0, SEEK_END);
	filesize = ftell(dst);
	fseek(dst, 0x10, SEEK_SET);
	fwrite(&filesize, 4, 1, dst);
	fclose(dst);
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-十鬼之绊\n用于压缩文件头为UF的字库文件。\n将字库文件拖到程序上。\nby Darkness-TX 2019.05.26\n\n");
	Compress(argv[1]);
	printf("已完成，总字模数%d\n", UF_Header.count);
	system("pause");
	return 0;
}