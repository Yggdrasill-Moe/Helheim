#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#define DataOffset 0x500//图片数据块地址

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;//总文件数，初始计数为0

struct index
{
	char FileName[260];//文件名
	unit32 FileSize;//文件大小
	unit32 Buffer;//所需的32字节组数
}Index[10000];

unit32 process_dir(char *dname)
{
	long Handle;
	unit32 i = 0;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.lz", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目，请将后缀命名为.lz\n");
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
		Index[FileNum].Buffer = (Index[FileNum].FileSize - 1280) / 32;//减去不用处理的前0x500字节
		FileNum++;
	} while (_findnext(Handle, &FileInfo) == 0);
	return FileNum;
}

void WriteFile(unit32 FileNum, char *dname)
{
	unit32 i = 0, j = 0, a = 0;
	FILE *OpenFile, *WriteFile;
	unit8* data;
	char Dispose[32];//处理用
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
			fopen_s(&WriteFile, strcat(Index[i].FileName, ".bin"), "wb");
			if (WriteFile == NULL)
			{
				printf("写入文件失败\n");
				system("pause");
			}
			data = malloc(1280);
			fread(data, 1, 1280, OpenFile);
			fwrite(data, 1, 1280, WriteFile);
			fseek(OpenFile, DataOffset, SEEK_SET);
			for (j = 0; j < Index[i].Buffer; j++)
			{
				if (j % 256 == 0)
				{
					j += 16;
					fseek(OpenFile, 512, SEEK_CUR);
				}
				fread(Dispose, 1, 32, OpenFile);
				for (a = 0; a < 31; a++)
					Dispose[a] = Dispose[a + 1];
				Dispose[31] = '\0';
				fwrite(Dispose, 1, 30, WriteFile);
			}
			free(data);
			fclose(OpenFile);
			fclose(WriteFile);
		}
		printf("Size:0x%X Buffer:%d %s \n", Index[i].FileSize, Index[i].Buffer, Index[i].FileName);
	}
}

int main(int argc, char *argv[])
{
		process_dir(argv[1]);
		WriteFile(FileNum, argv[1]);
		printf("已完成，总文件数%d\n", FileNum);
		system("pause");
		return 0;
}