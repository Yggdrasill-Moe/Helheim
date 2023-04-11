/*
用于解包与解压EVENTBIN.BIN
made by Darkness-TX
2023.03.06
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

void UnpackBlock(unit8* data, unit32 size, unit32 number)
{
	unit32 pos = 0, count = 0, offset = 0, blocksize = 0, endpos = 0, readsize = 0;
	unit8 blockname[8], eventname[12], dstname[MAX_PATH];
	FILE* dst = NULL;
	memcpy(&EVENT_Header, data, 0x10);
	pos = EVENT_Header.header_len;
	//EVENT_Header.num基本都是1，少量0，没发现有1以上的
	while (EVENT_Header.num)
	{
		memcpy(eventname, data + pos, 12);
		memcpy(&blocksize, data + pos + 12, 4);
		pos += 0x10;
		if (strncmp(eventname, "event", 5) == 0)
		{
			printf("\t%s blocksize:0x%X\n", eventname, blocksize);
			sprintf(dstname, "%04d_%s", number, eventname);
			_mkdir(dstname);
			_chdir(dstname);
			count = 0;
			memcpy(blockname, data + pos, 8);
			memcpy(&offset, data + pos + 8, 4);
			memcpy(&blocksize, data + pos + 12, 4);
			readsize += blocksize;
			if (blocksize != 0)
			{
				printf("\t\t%s offset:0x%X blocksize:0x%X\n", blockname, offset + EVENT_Header.header_len, blocksize);
				sprintf(dstname, "%04d_%s", count, blockname);
				dst = fopen(dstname, "wb");
				fwrite(data + offset + EVENT_Header.header_len, blocksize, 1, dst);
				fclose(dst);
			}
			endpos = offset + EVENT_Header.header_len;
			pos += 0x10;
			count++;
			while (pos < endpos)
			{
				memcpy(blockname, data + pos, 8);
				memcpy(&offset, data + pos + 8, 4);
				memcpy(&blocksize, data + pos + 12, 4);
				readsize += blocksize;
				if (blocksize != 0)
				{
					printf("\t\t%s offset:0x%X blocksize:0x%X\n", blockname, offset + EVENT_Header.header_len, blocksize);
					sprintf(dstname, "%04d_%s", count, blockname);
					dst = fopen(dstname, "wb");
					fwrite(data + offset + EVENT_Header.header_len, blocksize, 1, dst);
					fclose(dst);
				}
				pos += 0x10;
				count++;
			}
			pos += readsize;
			//然后会跟一串eventname
			if (strncmp(eventname, data + pos, 12) != 0)
			{
				printf("\t此块可能有读取错误！pos:0x%X filesize:0x%X\n", pos, size);
				system("pause");
			}
			pos += 12;
			//然后跟END\0，最后0x10对齐
			if (strncmp(data + pos, "END\0", 4) != 0)
			{
				printf("\t此块可能有读取错误！pos:0x%X filesize:0x%X\n", pos, size);
				system("pause");
			}
			pos += 4;
			if ((pos % 0x10) != 0)
				pos += 0x10 - (pos % 0x10);
			_chdir("..");
		}
		else if (strncmp(eventname, "MTFLz2_2", 8) == 0)
		{
			pos -= 0x10;
			printf("\t%s blocksize:0x%X\n", eventname, EVENT_Header.size);
			sprintf(dstname, "%04d", number);
			_mkdir(dstname);
			_chdir(dstname);
			dst = fopen("MTFLz2_2.bin", "wb");
			fwrite(data + pos, EVENT_Header.size, 1, dst);
			fclose(dst);
			pos += EVENT_Header.size;
			_chdir("..");
		}
		else
		{
			printf("\t未知的块标识:%s\n", eventname);
			system("pause");
		}
		EVENT_Header.num--;
	}
	//最终长度应该是相等的，判断一下
	if (pos != size)
	{
		printf("\t长度不对！pos:0x%X size:0x%X\n", pos, size);
		system("pause");
	}
}

void Unpack()
{
	FILE *src = NULL, *dst = NULL;
	unit32 i = 0, filesize = 0, startoffset;
	unit8 filename[MAX_PATH], *cdata = NULL, *udata = NULL;
	src = fopen("EVENTBINBDY.BIN", "rb");
	if (src == NULL)
	{
		printf("未在目录下找到EVENTBINBDY.BIN！\n");
		system("pause");
		exit(0);
	}
	fseek(src, 0, SEEK_END);
	filesize = ftell(src);
	fseek(src, 0, SEEK_SET);
	for (i = 0; i < filesize / 4; i++)
	{
		fread(&EVENTBDY[i].offset, 4, 1, src);
		FileNum++;
	}
	FileNum--;
	for (i = 1; i < filesize / 4; i++)
		EVENTBDY[i - 1].size = EVENTBDY[i].offset - EVENTBDY[i - 1].offset;
	fclose(src);
	src = fopen("EVENTBIN.BIN", "rb");
	if (src == NULL)
	{
		printf("未在目录下找到EVENTBIN.BIN！\n");
		system("pause");
		exit(0);
	}
	_mkdir("EVENT");
	_chdir("EVENT");
	for (i = 0; i < FileNum; i++)
	{
		cdata = malloc(EVENTBDY[i].size);
		fseek(src, EVENTBDY[i].offset, SEEK_SET);
		fread(cdata, EVENTBDY[i].size, 1, src);
		startoffset = get_start_offset(cdata);
		EVENTBDY[i].size = get_size(cdata);
		udata = malloc(EVENTBDY[i].size);
		LZdecompress(cdata + startoffset, udata, EVENTBDY[i].size);
		sprintf(filename, "%04d.bin", i);
		printf("%s offset:0x%X size:0x%X start:0x%X\n", filename, EVENTBDY[i].offset, EVENTBDY[i].size, startoffset);
		free(cdata);
		dst = fopen(filename, "wb");
		fwrite(udata, EVENTBDY[i].size, 1, dst);
		fclose(dst);
		UnpackBlock(udata, EVENTBDY[i].size, i);
		free(udata);
	}
	fclose(src);
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-超级机器人大战Z2\n用于解包与解压EVENTBIN.BIN。\nby Darkness-TX 2023.03.06\n\n");
	Unpack();
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}