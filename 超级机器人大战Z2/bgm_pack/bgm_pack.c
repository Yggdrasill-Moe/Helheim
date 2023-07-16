/*
用于不压缩封包文件并生成索引
made by Darkness-TX
2023.06.20
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
	if ((Handle = _findfirst("*.*", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目，请确认目录中是否存在文件\n");
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
	unit32 i = 0, j = 0, offset = 0, count = 0;
	unit8  *data = NULL;
	dst = fopen("DATA001.MOD", "wb");
	info = fopen("BGM.BIN", "wb");
	process_dir(fname);
	_chdir(fname);
	for (i = 0; i < FileNum; i++)
	{
		src = fopen(Index[i].FileName, "rb");
		data = malloc(Index[i].FileSize);
		fread(data, Index[i].FileSize, 1, src);
		fclose(src);
		offset = ftell(dst);
		fwrite(&offset, 4, 1, info);
		printf("%s offset:0x%X size:0x%X\n", Index[i].FileName, offset, Index[i].FileSize);
		fwrite(data, Index[i].FileSize, 1, dst);
		free(data);
		if ((ftell(dst) % 0x10) != 0)
		{
			count = 0x10 - ((unit32)ftell(dst) % 0x10);
			while (count--)
				fputc(0, dst);
		}
	}
	offset = ftell(dst);
	fwrite(&offset, 4, 1, info);
	fclose(info);
	fclose(dst);
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-超级机器人大战Z2\n用于不压缩封包文件并生成索引。\nby Darkness-TX 2023.06.20\n\n");
	Pack(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}