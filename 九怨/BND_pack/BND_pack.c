/*
用于打包ALL.BND
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
	unit32 i, j, point = 0, FileStart, dummy;
	FILE *OpenFile, *WriteFile, *PackFile;
	OpenFile = fopen(fname, "rb");
	if (OpenFile == NULL)
	{
		printf("无法打开文件！");
		system("pause");
		exit(0);
	}
	PackFile = fopen("ALL.BND.NEW","wb");
	if (PackFile == NULL)
	{
		printf("无法打开写入文件！");
		system("pause");
		exit(0);
	}
	_chdir(strcat(fname, "_unpack"));
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
	fseek(OpenFile, 0x14, SEEK_SET);
	fread(&FileStart, 4, 1, OpenFile);//第一个文件所在偏移
	fseek(OpenFile, 0, SEEK_SET);
	data = malloc(FileStart);
	fread(data, FileStart, 1, OpenFile);
	fwrite(data, FileStart, 1, PackFile);
	free(data);
	fseek(OpenFile, 0x1C, SEEK_SET);
	for (i = 0; i < FileNum; i++)
	{
		fread(&Index.NameOffset, 4, 1, OpenFile);
		point = ftell(OpenFile);
		fseek(OpenFile, Index.NameOffset, SEEK_SET);
		fscanf(OpenFile, "%32s", FileName);//fscanf可以检测以/0结尾的字符串
		WriteFile = fopen(FileName, "rb");
		if (WriteFile == NULL)
		{
			printf("无法%s！\n",FileName);
			system("pause");
			exit(0);
		}
		fseek(WriteFile, 0, SEEK_END);
		Index.FileSize = ftell(WriteFile);
		fseek(WriteFile, 0, SEEK_SET);
		data = malloc(Index.FileSize);
		fread(data, Index.FileSize, 1, WriteFile);
		Index.FileOffset = ftell(PackFile);
		fwrite(data, Index.FileSize, 1, PackFile);
		free(data);
		fseek(PackFile, 16 + i * 16 + 4, SEEK_SET);
		fwrite(&Index.FileOffset, 4, 1, PackFile);
		fwrite(&Index.FileSize, 4, 1, PackFile);
		fseek(PackFile, 0, SEEK_END);
		printf("FileID:0x%X FileOffset:0x%X FileSize:0x%X NameOffset:0x%X %s\n", i, Index.FileOffset, Index.FileSize, Index.NameOffset, FileName);
		dummy = (ftell(PackFile) / 0x800 + 1) * 0x800 - ftell(PackFile);
		if (dummy == 0x800 || i == FileNum - 1)
			dummy = 0;
		for (j = 0;j < dummy;j++)
			fprintf(PackFile, "%c", '\0');//0x800字节对齐，填充字节
		fclose(WriteFile);
		fseek(OpenFile, point + 0xC, SEEK_SET);
	}
	fclose(OpenFile);
}

int main(int argc, char *argv[])
{
	printf("project：Helheim-九怨\n用于打包ALL.BND，将解包文件夹与原文件放在一个目录，拖动原文件至程序上\nby Darkness-TX 2015.12.5\nEmail：1545492547@qq.com\n");
	WriteFile(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}