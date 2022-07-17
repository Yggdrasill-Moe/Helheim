/*
用于解压STSC2文件
made by Darkness-TX
2022.07.14
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include <Windows.h>
#include <zlib.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;
typedef unsigned __int64 unit64;

unit32 FileNum = 0;//包文件数，初始计数为0

struct stsc2_header
{
	unit8 magic[16];//STSC2.0.7
	unit32 block_length;//块大小
}STSC2_Header;

struct index
{
	char FileName[MAX_PATH];//文件名
}Index[5000];

unit32 process_dir(char* dname)
{
	long Handle;
	unit32 i = 0;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.bin", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目，请将原文件后缀命名为.bin\n");
		system("pause");
		return -1;
	}
	do
	{
		if (FileInfo.name[0] == '.')  //过滤本级目录和父目录
			continue;
		sprintf(Index[FileNum].FileName, FileInfo.name);
		FileNum++;
	} while (_findnext(Handle, &FileInfo) == 0);
	return FileNum;
}

void Decomp(char* dname)
{
	unit8* data = NULL, *udata = NULL, namebuff[MAX_PATH];
	FILE* src = NULL, * dst = NULL;
	unit32 i = 0, csize = 0, usize = 0;
	_chdir(dname);
	for (i = 0; i < FileNum; i++)
	{
		src = fopen(Index[i].FileName, "rb");
		if (src == NULL)
		{
			printf("打开%s失败", Index[i].FileName);
			system("pause");
			exit(0);
		}
		fread(STSC2_Header.magic, 16, 1, src);
		if (strncmp(STSC2_Header.magic, "STSC2.0.7", 9) != 0)
		{
			printf("%s的文件头不是STSC2.0.7\n", Index[i].FileName);
			system("pause");
			exit(0);
		}
		fread(&STSC2_Header.block_length, 4, 1, src);
		sprintf(namebuff, "%s.dec", Index[i].FileName);
		dst = fopen(namebuff, "wb");
		data = malloc(STSC2_Header.block_length);
		fseek(src, 0, SEEK_SET);
		fread(data, STSC2_Header.block_length, 1, src);
		fwrite(data, STSC2_Header.block_length, 1, dst);
		free(data);
		fseek(src, 0, SEEK_END);
		csize = ftell(src) - STSC2_Header.block_length;
		usize = csize * 2;
		fseek(src, STSC2_Header.block_length, SEEK_SET);
		data = malloc(csize);
		udata = malloc(usize);
		fread(data, csize, 1, src);
		fclose(src);
		uncompress(udata, &usize, data, csize);
		printf("%s block_length:0x%X comp_size:0x%X decomp_size:0x%X\n", Index[i].FileName, STSC2_Header.block_length, csize, usize);
		free(data);
		fwrite(udata, usize, 1, dst);
		free(udata);
		fclose(dst);
	}
}

int main(int argc, char* argv[])
{
	printf("project：Helheim-DATE A LIVE 莲Dystopia\n用于解压STSC2文件，将bin文件放进文件夹，再拖文件夹到程序上\nby Darkness-TX 2022.07.14\n\n");
	process_dir(argv[1]);
	Decomp(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}