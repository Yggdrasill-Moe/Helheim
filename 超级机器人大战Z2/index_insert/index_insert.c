/*
用于导入索引
made by Darkness-TX
2023.06.21
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

unit32 CheckString(char* buff)
{
	if (strncmp(buff, "0x", 2) == 0 || strncmp(buff, "0X", 2) == 0)
		return strtoul(buff + 2, NULL, 16);
	else
		return strtoul(buff, NULL, 10);
}

void IndexInsert(char* idxname, char* info)
{
	FILE *dst = NULL, *idx = NULL;
	unit32 filesize = 0, startoffset = 0;
	unit8 *data = NULL;
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
	fseek(idx, 0, SEEK_END);
	filesize = ftell(idx);
	fseek(idx, 0, SEEK_SET);
	data = malloc(filesize);
	fread(data, filesize, 1, idx);
	fclose(idx);
	dst = fopen("EBOOT.NEW", "wb");
	fwrite(data, filesize, 1, dst);
	free(data);
	fseek(dst, startoffset, SEEK_SET);
	idx = fopen(info, "rb");
	fseek(idx, 0, SEEK_END);
	filesize = ftell(idx);
	fseek(idx, 0, SEEK_SET);
	data = malloc(filesize);
	fread(data, filesize, 1, idx);
	fclose(idx);
	fwrite(data, filesize, 1, dst);
	free(data);
	fclose(dst);
}

int main(int argc, char* argv[])
{
	FILE* src = NULL, * dst = NULL, * info = NULL;
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-超级机器人大战Z2\n用于导入索引。\nby Darkness-TX 2023.06.21\n\n");
	IndexInsert(argv[1], argv[2]);
	system("pause");
	return 0;
}