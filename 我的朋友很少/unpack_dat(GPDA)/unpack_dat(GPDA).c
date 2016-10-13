/*
用于解包dat(GPDA)
by Darkness-TX
2016.9.16
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
char Head[] = "GPDA";

struct FileInfo
{
	unit32 FileID;//文件ID
	unit32 FileOffset;//文件偏移
	unit32 FileSize;//文件大小
	unit32 NameOffset;//文件名所在偏移
	unit32 NameSize;//文件名长度
	char FileName[32];//文件名
}Index;

void WriteFile(char *fname)
{
	unit8* data;
	unit8 signature[4];
	char FileName[260];
	unit32 i, point = 0;
	FILE *OpenFile, *WriteFile;
	OpenFile = fopen(fname, "rb");
	if (OpenFile == NULL)
	{
		printf("无法打开文件%s！",fname);
		system("pause");
		exit(0);
	}
	_mkdir(strcat(fname, "_unpack"));
	_chdir(fname);
	fread(signature, 4, 1, OpenFile);
	if (strncmp(signature, Head,4) != 0)
	{
		printf("文件头不是GPDA！");
		system("pause");
		exit(0);
	}
	else
	{
		fseek(OpenFile, 8, SEEK_CUR);
		fread(&FileNum, 4, 1, OpenFile);
	}
	for (i = 0; i < FileNum; i++)
	{
		Index.FileID = i;
		fread(&Index.FileOffset, 4, 1, OpenFile);
		fseek(OpenFile, 4, SEEK_CUR);
		fread(&Index.FileSize, 4, 1, OpenFile);
		fread(&Index.NameOffset, 4, 1, OpenFile);
		point = ftell(OpenFile);
		fseek(OpenFile, Index.FileOffset, SEEK_SET);
		data = malloc(Index.FileSize);
		fread(data, Index.FileSize, 1, OpenFile);
		fseek(OpenFile, Index.NameOffset, SEEK_SET);
		fread(&Index.NameSize, 4, 1, OpenFile);
		fread(Index.FileName, Index.NameSize, 1, OpenFile);
		Index.FileName[Index.NameSize] = '\0';//读取文件名后无结束符，自己加上
		printf("FileID:%d FileOffset:0x%X FileSize:0x%X NameOffset:0x%X %s\n", Index.FileID, Index.FileOffset, Index.FileSize, Index.NameOffset, Index.FileName);
		sprintf(FileName, "%d_%s", Index.FileID, Index.FileName);//因为有些封包中文件名居然有相同的，所以解包时后面解包的文件会覆盖掉前面相同名字的文件，所以添加个序号
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
	printf("project：Helheim-友少\n用于解包dat(GPDA)，拖动原文件至程序上\nby Darkness-TX 2016.9.16\nEmail：1545492547@qq.com\n");
	WriteFile(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}