#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <zlib.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;//总文件数，初始计数为0

struct index
{
	char FileName[260];//文件名
	unit32 FileSize;//文件大小
}Index[7000];

unit32 process_dir(char *dname)
{
	long Handle;
	unit32 i = 0;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.zip", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目。\n");
		system("pause");
		return -1;
	}
	do
	{
		if (FileInfo.name[0] == '.')  //过滤本级目录和父目录
			continue;
		for (i = 0; i <= 260; i++)
			Index[FileNum].FileName[i] = FileInfo.name[i];
		Index[FileNum].FileSize = FileInfo.size;
		FileNum++;
	} while (_findnext(Handle, &FileInfo) == 0);
	return FileNum;
}

unit32 EndianChange(unit32 Unit32Endian)
{
	unit8 *p;
	p = &Unit32Endian;
	return (unit32)(p[0] << 24) + (unit32)(p[1] << 16) +
		(unit32)(p[2] << 8) + (unit32)p[3];
}

void decompression(unit32 FileNum, char *dname)
{
	FILE *OpenFile, *WriteFile;
	unit8 *buf, *ubuf;
	unit32 i;
	uLong flen, ulen;
	_chdir(dname);
	for (i = 0; i < FileNum; i++)
	{
		fopen_s(&OpenFile, Index[i].FileName, "rb");
		fopen_s(&WriteFile, strcat(Index[i].FileName, ".tm2"), "wb");
		if (OpenFile == NULL)
		{
			printf("打开文件失败\n");
			system("pause");
		}
		else
		{
			fread(&flen, 4, 1, OpenFile);
			fread(&ulen, 4, 1, OpenFile);
			flen = EndianChange(flen);
			ulen = EndianChange(ulen);
			buf = malloc(flen);
			ubuf = malloc(ulen);
			fread(buf, flen, 1, OpenFile);
			uncompress(ubuf, &ulen, buf, flen);
			fwrite(ubuf, ulen, 1, WriteFile);
			free(buf);
			free(ubuf);
			fclose(OpenFile);
			fclose(WriteFile);
		}
	}
}

void compression(unit32 FileNum, char *dname)
{
	FILE *OpenFile, *WriteFile;
	unit8 *buf;
	unit32 i;
	unit32 flen, ulen;
	for (i = 0; i < FileNum; i++)
	{
		fopen_s(&OpenFile, Index[i].FileName, "rb");
		fopen_s(&WriteFile, strcat(Index[i].FileName, ".tm2"), "wb");
		if (OpenFile == NULL)
		{
			printf("打开文件失败\n");
			system("pause");
		}
		else
		{
			fread(&flen, 4, 1, OpenFile);
			fread(&ulen, 4, 1, OpenFile);
			buf = malloc(flen);
			fread(buf, 1, flen, OpenFile);
			uncompress(WriteFile, &ulen, buf, flen);
		}

	}
}

int main(int argc, char *argv[])
{
	unit32 s;
	printf("解压输入1，压缩输入2。解压需文件后缀为.zip，压缩需文件后缀为.tm2\n");
	scanf("%d", &s);
	process_dir(argv[1]);
	if (s == 1)
		decompression(FileNum, argv[1]);
	else if (s == 2)
		compression(FileNum, argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}