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
}Index[12000];

struct findex
{
	char FileName[260];//文件名
	unit32 FileNameOff;//文件名位置
	unit32 FileOffset;//文件偏移
}FIndex[300];

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
		exit(0);
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

void decompression(unit32 FileNum, char *dname)
{
	FILE *OpenFile, *WriteFile;
	unit8 *buf, *ubuf, filename[260];
	unit32 i, hflag, filen, noffset, foffset, j;
	unit16 flag;
	uLong flen, ulen;
	_chdir(dname);
	for (i = 0; i < FileNum; i++)
	{
		fopen_s(&OpenFile, Index[i].FileName, "rb");
		if (OpenFile == NULL)
		{
			printf("打开文件失败\n");
			system("pause");
			exit(0);
		}
		else
		{
			fread(&hflag, 4, 1, OpenFile);
			if (hflag == 0x200)
			{
				fseek(OpenFile, 0x20, SEEK_SET);
				fread(&filen, 4, 1, OpenFile);//文件数，之后为文件名偏移区起始位置，文件偏移区起始位置，都要加0x20
				fread(&noffset, 4, 1, OpenFile);
				noffset += 0x20;
				fread(&foffset, 4, 1, OpenFile);
				foffset += 0x20;
				printf("%s flag:0x%X nameoffset:0x%X fileoffset:0x%X\n", Index[i].FileName, hflag, noffset, foffset);
				fseek(OpenFile, noffset, SEEK_SET);
				for (j = 0; j < filen; j++)
				{
					fread(&FIndex[j].FileNameOff, 4, 1, OpenFile);
					FIndex[j].FileNameOff += 0x20;
				}
				fseek(OpenFile, foffset, SEEK_SET);
				for (j = 0; j < filen; j++)
				{
					fread(&FIndex[j].FileOffset, 4, 1, OpenFile);
					FIndex[j].FileOffset += 0x20;
				}
				for (j = 0; j < filen; j++)
				{
					fseek(OpenFile, FIndex[j].FileNameOff, SEEK_SET);
					fread(&FIndex[j].FileName, 16, 1, OpenFile);
					if (FIndex[j].FileName[15] != '\0')
					{
						fseek(OpenFile, -0x10, SEEK_CUR);
						fread(&FIndex[j].FileName, 32, 1, OpenFile);
						if (FIndex[j].FileName[31] != '\0')
						{
							fseek(OpenFile, -0x20, SEEK_CUR);
							fread(&FIndex[j].FileName, 48, 1, OpenFile);
						}
					}
				}
				sprintf(filename, "%s_unpack", Index[i].FileName);
				_mkdir(filename);
				_chdir(filename);
				for (j = 0; j < filen; j++)
				{
					fseek(OpenFile, FIndex[j].FileOffset, SEEK_SET);
					fread(&hflag, 4, 1, OpenFile);
					if (hflag == 0x10300)
					{
						fread(&flen, 4, 1, OpenFile);
						fread(&ulen, 4, 1, OpenFile);
						fseek(OpenFile, 0x14, SEEK_CUR);
						fopen_s(&WriteFile, FIndex[j].FileName, "wb");
						buf = malloc(flen);
						ubuf = malloc(ulen);
						fread(buf, flen, 1, OpenFile);
						uncompress(ubuf, &ulen, buf, flen);
						fwrite(ubuf, ulen, 1, WriteFile);
						free(buf);
						free(ubuf);
						fclose(WriteFile);
						printf("\t%s flag:0x%X 文件大小：0x%X 解压大小：0x%X\n", FIndex[j].FileName, hflag, flen + 0x20, ulen);
					}
					else
					{
						printf("\t%s\n", FIndex[j].FileName);
						system("pause");
					}
				}
				_chdir("..");
			}
			else// if (hflag == 0x10300 || hflag == 0x10600)
			{
				fread(&flen, 4, 1, OpenFile);
				fread(&ulen, 4, 1, OpenFile);
				fseek(OpenFile, 0x20, SEEK_SET);
				fread(&flag, 2, 1, OpenFile);
				if (flag == 0xDA78)
				{
					fseek(OpenFile, 0x20, SEEK_SET);
					fopen_s(&WriteFile, strcat(Index[i].FileName, ".dec"), "wb");
					buf = malloc(flen);
					ubuf = malloc(ulen);
					fread(buf, flen, 1, OpenFile);
					uncompress(ubuf, &ulen, buf, flen);
					fwrite(ubuf, ulen, 1, WriteFile);
					free(buf);
					free(ubuf);
					fclose(WriteFile);
					printf("%s 文件大小：0x%X 解压大小：0x%X\n", Index[i].FileName, Index[i].FileSize, ulen);
				}
				else
				{
					printf("%s 文件大小：0x%X\n", Index[i].FileName, Index[i].FileSize);
					system("pause");
				}
			}
		}
		fclose(OpenFile);
	}
}

int main(int argc, char *argv[])
{
	printf("project：Helheim-3DS WE 2014\n用于解压或解包zlib压缩的文件，将文件夹拖到程序上，\n已挂载可处理一万二千个文件的缓存区，文件夹中的文件请不要多余一万两千。\nby Darkness-TX 2016.3.11\n");
	process_dir(argv[1]);
	decompression(FileNum, argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}