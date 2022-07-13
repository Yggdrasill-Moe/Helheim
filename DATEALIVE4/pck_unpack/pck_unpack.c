/*
用于解包pck文件
made by Darkness-TX
2022.07.05
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include <zlib.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;
typedef unsigned __int64 unit64;

unit32 PackNum = 0;//包文件数，初始计数为0

struct FS
{
	unit8 signature[20];
	unit32 Length;//索引长度
}Fsignature;

struct PS
{
	unit8 signature[20];
	unit32 Length;//索引长度
}Psignature;

struct AS
{
	unit8 signature[20];
	unit32 Length;//索引长度
}Asignature;

struct packindex
{
	char FileName[100];//文件名
	unit32 FileSize;//文件大小
	unit32 FileOffset;//文件偏移
}PackIndex[20000];

int CreateDir(const char* sPathName)
{
	char DirName[256];
	strcpy(DirName, sPathName);
	int i, len = strlen(DirName);
	if (DirName[len - 1] != '/')
		strcat(DirName, "/");
	len = strlen(DirName);
	for (i = 1; i < len; i++)
	{
		if (DirName[i] == '/')
		{
			DirName[i] = 0;
			if (_access(DirName, 0) != 0)
			{
				if (_mkdir(DirName) == -1)
				{
					perror("mkdir   error");
					return   -1;
				}
			}
			DirName[i] = '/';
		}
	}
	return   0;
}

void ReadIndex(char* FileName)
{
	unit32 i = 0, s = 0, namestart, namenext, saveoff = 0;
	FILE* fp;
	fopen_s(&fp, FileName, "rb");
	fread(Fsignature.signature, 20, 1, fp);
	if (strncmp("Filename", Fsignature.signature, 8) != 0)//"Filename"为文件名索引标示
	{
		printf("不支持的文件类型，请检查文件头标示是否为FileName\n");
		system("pause");
		exit(0);
	}
	else
	{
		fread(&Fsignature.Length, 4, 1, fp);
		Fsignature.Length = Fsignature.Length - sizeof(Fsignature.Length) - sizeof(Fsignature.signature) + 7 & ~7;
		fseek(fp, Fsignature.Length, SEEK_CUR);
		fread(Psignature.signature, 20, 1, fp);
		if (strncmp("Pack", Psignature.signature, 4) != 0)//"Pack"为包索引标示
		{
			printf("不支持的文件类型，请检查包标示是否为Pack\n");
			system("pause");
			exit(0);
		}
		else
		{
			fread(&Psignature.Length, 4, 1, fp);
			Psignature.Length = Psignature.Length - sizeof(Psignature.Length) - sizeof(Psignature.signature) + 7 & ~7;
			fread(&PackNum, 4, 1, fp);
			for (s = 0; s < PackNum; s++)
			{
				fread(&PackIndex[s].FileOffset, 4, 1, fp);
				fread(&PackIndex[s].FileSize, 4, 1, fp);
			}
			fseek(fp, 0x18, SEEK_SET);
			fread(&namestart, 4, 1, fp);
			for (s = 0; s < PackNum; s++)
			{
				fread(&namenext, 4, 1, fp);
				fseek(fp, 0x18 + PackNum * 4 + saveoff, SEEK_SET);
				if (s != PackNum - 1)
				{
					fread(PackIndex[s].FileName, 1, namenext - namestart, fp);
					saveoff += namenext - namestart;
					namestart = namenext;
				}
				else
					fread(PackIndex[s].FileName, 1, Fsignature.Length - namestart, fp);
				fseek(fp, 0x18 + s * 4 + 8, SEEK_SET);
			}
		}
	}
	fclose(fp);
}

void WriteFile(char* FileName)
{
	unit8* data, dir[260], buff[400], path[100], * decdata, zlib[4];
	unit32 i, s, namesize, fsize, dsize;
	FILE* OpenFile, * WriteFile;
	fopen_s(&OpenFile, FileName, "rb");
	sprintf(dir, "%s_unpack", FileName);
	_mkdir(dir);
	_chdir(dir);
	_getcwd(buff, sizeof(buff));
	for (s = 0; s < PackNum; s++)
	{
		namesize = strlen(PackIndex[s].FileName);
		char* p;
		p = strrchr(PackIndex[s].FileName, '/');
		if (p == NULL)
		{
			if (PackIndex[s].FileName[0] == '\\')
			{
				for (i = 0; i < namesize; i++)
					PackIndex[s].FileName[i] = PackIndex[s].FileName[i + 1];
				PackIndex[s].FileName[namesize] = '\0';
			}
			fseek(OpenFile, PackIndex[s].FileOffset, SEEK_SET);
			fread(zlib, 4, 1, OpenFile);
			if (strncmp("ZLIB", zlib, 4) != 0)
			{
				fseek(OpenFile, -4, SEEK_CUR);
				data = malloc(PackIndex[s].FileSize);
				fread(data, PackIndex[s].FileSize, 1, OpenFile);
				fopen_s(&WriteFile, PackIndex[s].FileName, "wb");
				fwrite(data, PackIndex[s].FileSize, 1, WriteFile);
				fclose(WriteFile);
				free(data);
				printf("%s offset:0x%X size:0x%X\n", PackIndex[s].FileName, PackIndex[s].FileOffset, PackIndex[s].FileSize);
			}
			else
			{
				data = malloc(PackIndex[s].FileSize - 0xC);
				fread(&dsize, 4, 1, OpenFile);
				fread(&fsize, 4, 1, OpenFile);
				fread(data, PackIndex[s].FileSize - 0xC, 1, OpenFile);
				decdata = malloc(dsize);
				uncompress(decdata, &dsize, data, fsize);
				fopen_s(&WriteFile, PackIndex[s].FileName, "wb");
				fwrite(decdata, dsize, 1, WriteFile);
				fclose(WriteFile);
				free(data);
				free(decdata);
				printf("%s offset:0x%X fsize:0x%X desize:0x%X\n", PackIndex[s].FileName, PackIndex[s].FileOffset, PackIndex[s].FileSize, dsize);
			}
		}
		else
		{
			strncpy(path, PackIndex[s].FileName, p - &PackIndex[s].FileName + 1);
			path[p - &PackIndex[s].FileName + 1] = '\0';
			CreateDir(path);
			_chdir(buff);
			namesize = strlen(PackIndex[s].FileName);
			for (i = namesize; i > 0; i--)
			{
				PackIndex[s].FileName[i + 1] = PackIndex[s].FileName[i - 1];
			}
			PackIndex[s].FileName[0] = '.';
			PackIndex[s].FileName[1] = '/';
			fseek(OpenFile, PackIndex[s].FileOffset, SEEK_SET);
			fread(zlib, 4, 1, OpenFile);
			if (strncmp("ZLIB", zlib, 4) != 0)
			{
				fseek(OpenFile, -4, SEEK_CUR);
				data = malloc(PackIndex[s].FileSize);
				fread(data, PackIndex[s].FileSize, 1, OpenFile);
				fopen_s(&WriteFile, PackIndex[s].FileName, "wb");
				fwrite(data, PackIndex[s].FileSize, 1, WriteFile);
				fclose(WriteFile);
				free(data);
				printf("%s offset:0x%X size:0x%X\n", PackIndex[s].FileName, PackIndex[s].FileOffset, PackIndex[s].FileSize);
			}
			else
			{
				data = malloc(PackIndex[s].FileSize - 0xC);
				fread(&dsize, 4, 1, OpenFile);
				fread(&fsize, 4, 1, OpenFile);
				fread(data, PackIndex[s].FileSize - 0xC, 1, OpenFile);
				decdata = malloc(dsize);
				uncompress(decdata, &dsize, data, fsize);
				fopen_s(&WriteFile, PackIndex[s].FileName, "wb");
				fwrite(decdata, dsize, 1, WriteFile);
				fclose(WriteFile);
				free(data);
				free(decdata);
				printf("%s offset:0x%X fsize:0x%X desize:0x%X\n", PackIndex[s].FileName, PackIndex[s].FileOffset, PackIndex[s].FileSize, dsize);
			}
		}
	}
	fclose(OpenFile);
}

int main(int argc, char* argv[])
{
	printf("project：Helheim-DATE A LIVE 莲Dystopia\n用于解包pck文件，将pck文件拖到程序上\nby Darkness-TX 2022.07.05\n\n");
	ReadIndex(argv[1]);
	WriteFile(argv[1]);
	printf("已完成，总文件数%d\n", PackNum);
	system("pause");
	return 0;
}