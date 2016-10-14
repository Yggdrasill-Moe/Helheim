#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit8 Header[] = "AFS\0";
unit32 FileNum = 0;//总文件数，初始计数为0

struct index
{
	char FileName[260];//文件名
	unit32 FileSize;//文件大小
	unit32 FileOffset;
}Index[2000];

unit32 process_dir(char *dname)
{
	long Handle;
	unit32 i = 0;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.*", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目。\n");
		system("pause");
		return -1;
	}
	do
	{
		if (FileInfo.name[0] == '.')  //过滤本级目录和父目录
			continue;
		//for (i = 0; i <= 260; i++)
		//Index[FileNum].FileName[i] = FileInfo.name[i];
		//Index[FileNum].FileSize = FileInfo.size;
		FileNum++;
	} while (_findnext(Handle, &FileInfo) == 0);
	return FileNum;
}

void WriteFile(unit32 FileNum, char *dname)
{
	FILE *OpenFile, *WriteFile, *fp;
	unit8 *data;
	unit32 i, offset, j, AFSFileNum;;
	_chdir("..");
	fopen_s(&fp, "etc.afs", "rb");
	if (fp == NULL)
	{
		printf("打开原文件失败\n");
		system("pause");
	}
	fseek(fp, 4, SEEK_SET);
	fread(&AFSFileNum, 4, 1, fp);
	fseek(fp, AFSFileNum * 8 + 8, SEEK_SET);
	fread(&offset, 4, 1, fp);
	fseek(fp, offset, SEEK_SET);
	for (i = 0; i < AFSFileNum; i++)
	{
		fread(&Index[i].FileName, 16, 1, fp);
		fseek(fp, 32, SEEK_CUR);
	}
	fopen_s(&WriteFile, "bg.afs.new", "wb");
	if (WriteFile == NULL)
	{
		printf("打开新文件失败\n");
		system("pause");
	}
	fwrite(Header, 4, 1, WriteFile);
	fwrite(&AFSFileNum, 4, 1, WriteFile);
	offset = 8;
	offset += AFSFileNum * 8;
	offset += 8;
	_chdir(dname);
	offset += ((offset / 0x800 + 1) * 0x800 - offset);
	fseek(WriteFile, offset, SEEK_SET);
	for (i = 0; i < AFSFileNum; i++)
	{
		if (strlen(Index[i].FileName) != 0)
		{
			fopen_s(&OpenFile, Index[i].FileName, "rb");
			if (OpenFile == NULL)
			{
				printf("%s打开失败\n", Index[i].FileName);
				system("pause");
			}
			fseek(OpenFile, 0, SEEK_END);
			Index[i].FileSize = (unit32)ftell(OpenFile);
			fseek(OpenFile, 0, SEEK_SET);
			data = malloc(Index[i].FileSize);
			Index[i].FileOffset = (unit32)ftell(WriteFile);
			fread(data, Index[i].FileSize, 1, OpenFile);
			fwrite(data, Index[i].FileSize, 1, WriteFile);
			offset += Index[i].FileSize;
			offset += ((offset / 0x800 + 1) * 0x800 - offset);
			fseek(WriteFile, offset, SEEK_SET);
			free(data);
			fclose(OpenFile);
		}
		else
		{
			Index[i].FileSize = 0;
			Index[i].FileOffset = 0;
		}
	}
	fseek(WriteFile, 0x8, SEEK_SET);
	for (i = 0; i < AFSFileNum; i++)
	{
		fwrite(&Index[i].FileOffset, 4, 1, WriteFile);
		fwrite(&Index[i].FileSize, 4, 1, WriteFile);
	}
	fwrite(&offset, 4, 1, WriteFile);
	j = AFSFileNum * 32;
	fwrite(&j, 4, 1, WriteFile);
	fseek(WriteFile, offset, SEEK_SET);
	for (i = 0; i < AFSFileNum; i++)
	{
		fwrite(&Index[i].FileName, strlen(Index[i].FileName), 1, WriteFile);
		j = 16 - strlen(Index[i].FileName);
		fseek(WriteFile, j + 32, SEEK_CUR);
	}
	j = ftell(WriteFile);
	j += ((j / 0x800 + 1) * 0x800 - j);
	fseek(WriteFile, j - 1, SEEK_SET);
	fprintf(WriteFile, "%c", '\0');
	j -= offset;
	fseek(WriteFile, AFSFileNum * 8 + 8 + 4, SEEK_SET);
	fwrite(&j, 4, 1, WriteFile);
	fclose(fp);
	fclose(WriteFile);
}

int main(int argc, char *argv[])
{
	process_dir(argv[1]);
	WriteFile(FileNum, argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}