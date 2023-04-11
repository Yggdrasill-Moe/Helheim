/*
用于封包与压缩EVENTBIN.BIN
made by Darkness-TX
2023.03.21
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
#include "SRWZ_LZ.h"

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;

struct index
{
	char FileName[260];//文件名
	unit32 FileSize;//文件大小
}Index[2000];

struct EVENTBDY_s
{
	unit32 offset;
	unit32 size;
}EVENTBDY[2000];

struct EVENT_s
{
	unit32 num;
	unit32 header_len;
	unit32 unk;
	unit32 size;
}EVENT_Header;

unit32 process_dir(char* dname)
{
	long Handle;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.bin", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目，请确认目录中是否存在.bin文件\n");
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

void PackBlock(char* dname)
{
	FILE *src = NULL, *dst = NULL, *block = NULL;
	unit32 i = 0, offset = 0, blocksize = 0, filesize = 0, count = 0, endpos = 0, end = 0x444E45, pos = 0;
	unit8 filename[MAX_PATH], blockname[8], eventname[12], zero = 0;
	unit8* data = NULL;
	for (i = 0; i < FileNum; i++)
	{
		_chdir(dname);
		sprintf(filename, "%s\\%s", dname, Index[i].FileName);
		src = fopen(filename, "rb");
		fread(&EVENT_Header, sizeof(EVENT_Header), 1, src);
		fseek(src, EVENT_Header.header_len, SEEK_SET);
		sprintf(filename, "%s_dst", dname);
		_chdir(filename);
		sprintf(filename, "%s_dst\\%s", dname, Index[i].FileName);
		dst = fopen(filename, "wb");
		fwrite(&EVENT_Header, sizeof(EVENT_Header), 1, dst);
		fseek(dst, EVENT_Header.header_len, SEEK_SET);
		EVENT_Header.size = 0;
		printf("%s oldsize:0x%X\n", Index[i].FileName, Index[i].FileSize);
		while (EVENT_Header.num)
		{
			fread(eventname, 12, 1, src);
			fread(&blocksize, 4, 1, src);
			if (strncmp(eventname, "event", 5) == 0)
			{
				pos = ftell(dst);
				printf("\t%s oldblocksize:0x%X\n", eventname, blocksize);
				sprintf(filename, "%04d_%s", i, eventname);
				_chdir(filename);
				count = 0;
				filesize = 0;
				fread(blockname, 8, 1, src);
				fread(&offset, 4, 1, src);
				fread(&blocksize, 4, 1, src);
				fseek(dst, offset + pos, SEEK_SET);
				endpos = offset + EVENT_Header.header_len;
				if (blocksize != 0)
				{
					sprintf(filename, "%04d_%s", count, blockname);
					block = fopen(filename, "rb");
					fseek(block, 0, SEEK_END);
					blocksize = ftell(block);
					fseek(block, 0, SEEK_SET);
					offset = ftell(dst) - pos;
					printf("\t\t%s offset:0x%X blocksize:0x%X\n", blockname, offset, blocksize);
					data = malloc(blocksize);
					fread(data, blocksize, 1, block);
					fwrite(data, blocksize, 1, dst);
					free(data);
					fclose(block);
					fseek(dst, ftell(src) - 0x10, SEEK_SET);
					fwrite(blockname, 8, 1, dst);
					fwrite(&offset, 4, 1, dst);
					fwrite(&blocksize, 4, 1, dst);
					fseek(dst, 0, SEEK_END);
					filesize += blocksize;
				}
				else
				{
					fseek(dst, pos + 0x10, SEEK_SET);
					fwrite(blockname, 8, 1, dst);
					fwrite(&offset, 4, 1, dst);
					fwrite(&blocksize, 4, 1, dst);
					fseek(dst, 0, SEEK_END);
				}
				count++;
				while ((unit32)ftell(src) < endpos)
				{
					fread(blockname, 8, 1, src);
					fread(&offset, 4, 1, src);
					fread(&blocksize, 4, 1, src);
					if (blocksize == 0)
					{
						offset = 0;
						fseek(dst, ftell(src) - 0x10, SEEK_SET);
						fwrite(blockname, 8, 1, dst);
						fwrite(&offset, 4, 1, dst);
						fwrite(&blocksize, 4, 1, dst);
						fseek(dst, 0, SEEK_END);
					}
					else
					{
						sprintf(filename, "%04d_%s", count, blockname);
						block = fopen(filename, "rb");
						fseek(block, 0, SEEK_END);
						blocksize = ftell(block);
						fseek(block, 0, SEEK_SET);
						offset = ftell(dst) - pos;
						printf("\t\t%s offset:0x%X blocksize:0x%X\n", blockname, offset, blocksize);
						data = malloc(blocksize);
						fread(data, blocksize, 1, block);
						fwrite(data, blocksize, 1, dst);
						free(data);
						fclose(block);
						fseek(dst, ftell(src) - 0x10, SEEK_SET);
						fwrite(blockname, 8, 1, dst);
						fwrite(&offset, 4, 1, dst);
						fwrite(&blocksize, 4, 1, dst);
						fseek(dst, 0, SEEK_END);
						filesize += blocksize;
					}
					count++;
				}
				fwrite(eventname, 12, 1, dst);
				fwrite(&end, 4, 1, dst);
				filesize += count * 0x10 + 0x10 + 0x10;
				fseek(dst, pos, SEEK_SET);
				fwrite(eventname, 12, 1, dst);
				fwrite(&filesize, 4, 1, dst);
				EVENT_Header.size += filesize;
				_chdir("..");
			}
			else if (strncmp(eventname, "MTFLz2_2", 8) == 0)
			{
				sprintf(filename, "%04d", i);
				_chdir(filename);
				block = fopen("MTFLz2_2.bin", "rb");
				fseek(block, 0, SEEK_END);
				EVENT_Header.size += ftell(block);
				fseek(block, 0, SEEK_SET);
				printf("\t%s blocksize:0x%X\n", eventname, EVENT_Header.size);
				data = malloc(EVENT_Header.size);
				fread(data, EVENT_Header.size, 1, block);
				fwrite(data, EVENT_Header.size, 1, dst);
				free(data);
				fclose(block);
				_chdir("..");
			}
			else
			{
				printf("\t未知的块标识:%s\n", eventname);
				system("pause");
			}
			fseek(dst, 0, SEEK_END);
			EVENT_Header.num--;
		}
		if ((ftell(dst) % 0x10) != 0)
		{
			EVENT_Header.size += 0x10 - ((unit32)ftell(dst) % 0x10);
			count = 0x10 - ((unit32)ftell(dst) % 0x10);
			while (count--)
				fwrite(&zero, 1, 1, dst);
		}
		fseek(dst, 0x0C, SEEK_SET);
		fwrite(&EVENT_Header.size, 4, 1, dst);
		if (EVENT_Header.header_len == 0x20)
			fwrite(&EVENT_Header.size, 4, 1, dst);
		fseek(dst, 0, SEEK_END);
		Index[i].FileSize = ftell(dst);
		fclose(dst);
		fclose(src);
	}
	_chdir("..");
}

void Pack(char* dname)
{
	FILE *src = NULL, *dst = NULL, *bdy = NULL, *hed = NULL;
	unit32 i = 0, offset = 0, count = 0, j = 0;
	unit8 dstname[MAX_PATH], sizebuff[4], zero = 0;
	unit8 *cdata = NULL, *udata = NULL;
	PackBlock(dname);
	dst = fopen("EVENTBIN.new", "wb");
	bdy = fopen("EVENTBINBDY.new", "wb");
	hed = fopen("EVENTBINHED.new", "wb");
	sprintf(dstname, "%s_dst", dname);
	_chdir(dstname);
	for (i = 0; i < FileNum; i++)
	{
		src = fopen(Index[i].FileName, "rb");
		udata = malloc(Index[i].FileSize);
		fwrite(&Index[i].FileSize, 4, 1, hed);
		fread(udata, Index[i].FileSize, 1, src);
		fclose(src);
		cdata = malloc(Index[i].FileSize * 2);
		offset = ftell(dst);
		printf("%s offset:0x%X decsize:0x%X ", Index[i].FileName, offset, Index[i].FileSize);
		count = make_size(sizebuff, Index[i].FileSize);
		for (j = 4 - count; j < 4; j++)
			fwrite(&sizebuff[j], 1, 1, dst);
		count = make_buff_size(sizebuff, Index[i].FileSize);
		for (j = 4 - count; j < 4; j++)
			fwrite(&sizebuff[j], 1, 1, dst);
		fputc(1, dst);
		fwrite(&offset, 4, 1, bdy);
		Index[i].FileSize = LZcompress(cdata, udata, Index[i].FileSize);
		fwrite(cdata, Index[i].FileSize, 1, dst);
		printf("compsize:0x%X\n", Index[i].FileSize);
		if ((ftell(dst) % 0x10) != 0)
		{
			count = 0x10 - ((unit32)ftell(dst) % 0x10);
			while (count--)
				fwrite(&zero, 1, 1, dst);
		}
		free(cdata);
		free(udata);
	}
	offset = ftell(dst);
	fwrite(&offset, 4, 1, bdy);
	count = 4;
	while (count--)
		fwrite(&zero, 1, 1, hed);
	fclose(dst);
	fclose(bdy);
	fclose(hed);
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-超级机器人大战Z2\n用于封包与压缩EVENTBIN.BIN。\nby Darkness-TX 2023.03.21\n\n");
	process_dir(argv[1]);
	Pack(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}