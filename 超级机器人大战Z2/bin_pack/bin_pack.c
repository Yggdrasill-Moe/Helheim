/*
用于封包与压缩bin文件
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

struct index
{
	char FileName[260];//文件名
	unit32 FileSize;//文件大小
}Index[20000];

unit32 process_dir(char* dname)
{
	long Handle;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.bin", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目，请确认目录中是否存在.bin文件\n");
		system("pause");
		return -1;
	}
	do
	{
		if (FileInfo.name[0] == '.')  //过滤本级目录和父目录
			continue;
		sprintf(Index[FileNum].FileName, FileInfo.name);
		Index[FileNum].FileSize = FileInfo.size;
		FileNum++;
	} while (_findnext(Handle, &FileInfo) == 0);
	return FileNum;
}

void Pack(char* fname)
{
	FILE *src = NULL, *dst = NULL, *info = NULL;
	unit8 dstname[MAX_PATH], *cdata = NULL, *udata = NULL, sizebuff[4];
	unit32 i = 0, j = 0, count = 0;
	sprintf(dstname, "%s.new", fname);
	dst = fopen(dstname, "wb");
	sprintf(dstname, "%s.new_info.txt", fname);
	info = fopen(dstname, "wt,ccs=UNICODE");
	sprintf(dstname, "%s_unpack", fname);
	process_dir(dstname);
	_chdir(dstname);
	for (i = 0; i < FileNum; i++)
	{
		src = fopen(Index[i].FileName, "rb");
		udata = malloc(Index[i].FileSize);
		fread(udata, Index[i].FileSize, 1, src);
		fclose(src);
		cdata = malloc(Index[i].FileSize * 2);
		fwprintf(info, L"%08d|%08X|%08X|", i, ftell(dst), Index[i].FileSize);
		printf("%s offset:0x%X decsize:0x%X ", Index[i].FileName, ftell(dst), Index[i].FileSize);
		count = make_size(sizebuff, Index[i].FileSize);
		for (j = 4 - count; j < 4; j++)
			fwrite(&sizebuff[j], 1, 1, dst);
		count = make_buff_size(sizebuff, Index[i].FileSize);
		for (j = 4 - count; j < 4; j++)
			fwrite(&sizebuff[j], 1, 1, dst);
		fputc(1, dst);
		Index[i].FileSize = LZcompress(cdata, udata, Index[i].FileSize);
		fwrite(cdata, Index[i].FileSize, 1, dst);
		fwprintf(info, L"%08X\n", Index[i].FileSize);
		printf("compsize:0x%X\n", Index[i].FileSize);
		if ((ftell(dst) % 0x10) != 0)
		{
			count = 0x10 - ((unit32)ftell(dst) % 0x10);
			while (count--)
				fputc(0, dst);
		}
		free(udata);
		free(cdata);
	}
	fwprintf(info, L"FFFFFFFF|%08X|%08X|%08X\n", ftell(dst), 0, 0);
	fclose(info);
	fclose(dst);
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-超级机器人大战Z2\n用于封包与压缩bin文件。\nby Darkness-TX 2023.04.08\n\n");
	Pack(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}