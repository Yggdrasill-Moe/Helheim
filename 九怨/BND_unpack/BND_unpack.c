/*
用于解包ALL.BND，解包后的BND无法处理，但是解包后的BND结构大同小异
by Darkness-TX
2015.12.5
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;//总文件数，初始计数为0
char Head[] = "BND\0";

struct FileInfo
{
	unit32 FileID;//文件ID
	unit32 FileOffset;//文件偏移
	unit32 FileSize;//文件大小
	unit32 NameOffset;//文件名所在偏移
}Index;

void WriteFile(char *fname)
{
	unit8* data;
	unit8 signature[4];
	char FileName[32];
	unit32 i, point = 0;
	FILE *OpenFile, *WriteFile;
	OpenFile = fopen(fname, "rb");
	if (OpenFile == NULL)
	{
		printf("无法打开文件！");
		system("pause");
		exit(0);
	}
	_mkdir(strcat(fname, "_unpack"));
	_chdir(fname);
	fread(signature, 4, 1, OpenFile);
	if (strcmp(signature, Head) != 0)
	{
		printf("文件头不是BND！");
		system("pause");
		exit(0);
	}
	else
	{
		fseek(OpenFile, 0xC, SEEK_SET);
		fread(&FileNum, 4, 1, OpenFile);
	}
	for (i = 0; i < FileNum; i++)
	{
		fread(&Index.FileID, 4, 1, OpenFile);
		fread(&Index.FileOffset, 4, 1, OpenFile);
		fread(&Index.FileSize, 4, 1, OpenFile);
		fread(&Index.NameOffset, 4, 1, OpenFile);
		point = ftell(OpenFile);
		fseek(OpenFile, Index.FileOffset, SEEK_SET);
		data = malloc(Index.FileSize);
		fread(data, Index.FileSize, 1, OpenFile);
		fseek(OpenFile, Index.NameOffset, SEEK_SET);
		fscanf(OpenFile, "%32s",FileName);//fscanf可以检测以/0结尾的字符串
		printf("FileID:0x%X FileOffset:0x%X FileSize:0x%X NameOffset:0x%X %s\n", Index.FileID, Index.FileOffset, Index.FileSize, Index.NameOffset, FileName);
		WriteFile = fopen(FileName, "wb");
		fwrite(data, Index.FileSize, 1, WriteFile);
		free(data);
		fclose(WriteFile);
		fseek(OpenFile, point, SEEK_SET);
	}
	fclose(OpenFile);
}

int main(int argc, char *argv[])
{
	printf("project：Helheim-九怨\n用于解包ALL.BND，拖动原文件至程序上\nby Darkness-TX 2015.12.5\nEmail：1545492547@qq.com\n");
	WriteFile(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}