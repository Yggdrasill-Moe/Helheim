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

struct unpackindex
{
	char FileName[260];//文件名
	unit32 FileOffset;//文件偏移
	unit32 FileSize;//文件大小
};

struct index
{
	char FileName[260];//文件名
	unit32 FileSize;//文件大小
	unit32 Num;//含文本数
	struct unpackindex UnpackIndex[100];
}Index[7000];

unit32 EndianChange(unit32 Unit32Endian)
{
	unit8 *p;
	p = &Unit32Endian;
	return (unit32)(p[0] << 24) + (unit32)(p[1] << 16) +
		(unit32)(p[2] << 8) + (unit32)p[3];
}

unit32 process_dir(char *dname)
{
	long Handle;
	unit32 i = 0;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.bin", &FileInfo)) == -1L)
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

void ReadIndex(unit32 FileNum)
{
	unit32 i = 0, j = 1;
	FILE *fp;
	for (i = 0; i < FileNum; i++)
	{
		fopen_s(&fp, Index[i].FileName, "rb");
		j = 1;
		while (1)
		{
			fread(&Index[i].UnpackIndex[j].FileOffset, 4, 1, fp);
			if (Index[i].UnpackIndex[j].FileOffset == Index[i].FileSize)
				break;
			fread(&Index[i].UnpackIndex[j].FileName, 24, 1, fp);
			Index[i].Num = j;
			j++;
		}
	}
}

void WriteFile(unit32 FileNum, char *dname)
{
	unit32 i = 0, s = 1;
	FILE *OpenFile, *WriteFile;
	unit8* data;
	_chdir(dname);
	for (i = 0; i < FileNum; i++)
	{
		fopen_s(&OpenFile, Index[i].FileName, "rb");
		if (OpenFile == NULL)
		{
			printf("打开文件失败\n");
			system("pause");
		}
		else
		{
			for (s = 1; s <= Index[i].Num; s++)
			{
				fopen_s(&WriteFile, Index[i].UnpackIndex[s].FileName, "wb");
				fseek(OpenFile, Index[i].UnpackIndex[s].FileOffset, SEEK_SET);
				fread(&Index[i].UnpackIndex[s].FileSize, 4, 1, OpenFile);
				Index[i].UnpackIndex[s].FileSize = EndianChange(Index[i].UnpackIndex[s].FileSize);
				Index[i].UnpackIndex[s].FileSize += 8;;
				data = malloc(Index[i].UnpackIndex[s].FileSize);
				fseek(OpenFile, Index[i].UnpackIndex[s].FileOffset, SEEK_SET);
				fread(data, Index[i].UnpackIndex[s].FileSize, 1, OpenFile);
				fwrite(data, Index[i].UnpackIndex[s].FileSize, 1, WriteFile);
				free(data);
				fclose(WriteFile);
			}
			fclose(OpenFile);
		}
	}
}

int main(int argc, char *argv[])
{
	process_dir(argv[1]);
	ReadIndex(FileNum);
	WriteFile(FileNum, argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}