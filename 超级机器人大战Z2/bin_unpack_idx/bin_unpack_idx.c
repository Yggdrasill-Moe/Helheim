/*
用于基于索引的解包bin文件
made by Darkness-TX
2023.04.10
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

unit32 FileNum = 0;

unit32 CheckString(char *buff)
{
	if (strncmp(buff, "0x", 2) == 0 || strncmp(buff, "0X", 2) == 0)
		return strtoul(buff + 2, NULL, 16);
	else
		return strtoul(buff, NULL, 10);
}

void Unpack(char *fname, char *idxname)
{
	FILE *src = NULL, *dst = NULL, *info = NULL, *idx = NULL;
	unit32 filesize = 0, startoffset = 0, size = 0, offset = 0;
	unit8 dstname[MAX_PATH], *data = NULL;
	char dirPath[MAX_PATH];
	char iniPath[MAX_PATH];
	char buff[30];
	GetCurrentDirectoryA(MAX_PATH, dirPath);
	sprintf(iniPath, "%s\\%s", dirPath, "index.ini");
	if (_access(iniPath, 4) == -1)
	{
		printf("初始化失败，请确认目录下是否含有index.ini\n");
		system("pause");
		exit(0);
	}
	GetPrivateProfileStringA(idxname, "offset", "0", buff, MAX_PATH, iniPath);
	startoffset = CheckString(buff);
	GetPrivateProfileStringA(idxname, "idxfile", "BOOT.BIN", buff, MAX_PATH, iniPath);
	idx = fopen(buff, "rb");
	if (idx == NULL)
	{
		printf("初始化失败，请确认目录下是否含有%s\n", buff);
		system("pause");
		exit(0);
	}
	fseek(idx, startoffset, SEEK_SET);
	src = fopen(fname, "rb");
	fseek(src, 0, SEEK_END);
	filesize = ftell(src);
	fseek(src, 0, SEEK_SET);
	printf("%s size:0x%X idx_startoffset:0x%X\n", fname, filesize,startoffset);
	sprintf(dstname, "%s_info.txt", fname);
	info = fopen(dstname, "wt,ccs=UNICODE");
	sprintf(dstname, "%s_unpack", fname);
	_mkdir(dstname);
	_chdir(dstname);
	fread(&offset, 4, 1, idx);
	while (offset != filesize)
	{
		startoffset = offset;
		fread(&offset, 4, 1, idx);
		size = offset - startoffset;
		fwprintf(info, L"%08d.bin offset:0x%X size:0x%X\n", FileNum, startoffset, size);
		printf("\tfilenum:%08d offset:0x%X size:0x%X\n", FileNum, startoffset, size);
		data = malloc(size);
		fseek(src, startoffset, SEEK_SET);
		fread(data, size, 1, src);
		sprintf(dstname, "%08d.bin", FileNum);
		FileNum++;
		dst = fopen(dstname, "wb");
		fwrite(data, size, 1, dst);
		free(data);
		fclose(dst);
	}
	fclose(idx);
	fclose(info);
	fclose(src);
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-超级机器人大战Z2\n用于基于索引的解包bin文件。\nby Darkness-TX 2023.04.10\n\n");
	Unpack(argv[1], argv[2]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}