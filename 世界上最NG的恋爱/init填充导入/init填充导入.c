#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <locale.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;//总文件数，初始计数为0

struct index
{
	char FileName[260];//文件名
	unit32 FileSize;//文件大小
}Index[300];

wchar_t *cutstr(wchar_t *dst, wchar_t *src, int n, int m) /*n为长度，m为位置*/
{
	wchar_t *p = src;
	wchar_t *q = dst;
	int len = wcslen(src);
	if (n>len)
		n = len - m;    /*从第m个到最后*/
	if (m<0)
		m = 0;    /*从第一个开始*/
	if (m>len)
		return NULL;
	p += m;
	while (n--)
		*(q++) = *(p++);
	*(q++) = '\0'; /*有必要吗？很有必要*/
	return dst;
}

void Insert()
{
	FILE *Openbin, *Opentxt, *WriteFile, *Tbl;
	wchar_t tbl[4000], hex[4000][5], data[256], *find, buff[10], *buff1, buff2[2];
	unit32 j, k = 0, slen, txtoffset, txtbinoffset, tnum, m = 6, n, l = 0, hlong, newtxtoffset;
	unit8 *data1, hexbuff, zero = '\0';
	fopen_s(&Tbl, "汉化码表.TXT", "rt,ccs=UNICODE");
	if (Tbl == NULL)
	{
		printf("打开码表文件失败\n");
		system("pause");
	}
	while (fgetws(data, 256, Tbl) != NULL)
	{
		slen = wcslen(data);
		find = wcschr(data, L'=');
		cutstr(hex[k], data, find - data, 0);
		tbl[k] = data[find - data + 1];
		k++;
	}
	fclose(Tbl);
	fopen_s(&Openbin, "init.bin.lz", "rb");
	if (Openbin == NULL)
	{
		printf("打开原文件失败\n");
		system("pause");
	}
	fopen_s(&Opentxt, "initlog.txt", "rt,ccs=UNICODE");
	if (Opentxt == NULL)
	{
		printf("打开文本文件失败\n");
		system("pause");
	}
	fopen_s(&WriteFile, "init.bin.lz.new", "wb");
	if (WriteFile == NULL)
	{
		printf("打开导入文件失败\n");
		system("pause");
	}
	fseek(Openbin, 0, SEEK_END);
	j = ftell(Openbin);
	fseek(Openbin, 0, SEEK_SET);
	data1 = malloc(j);
	fread(data1, j, 1, Openbin);
	fwrite(data1, j, 1, WriteFile);
	free(data1);
	fseek(Openbin, 0, SEEK_SET);
	j = 1;
	while (fgetws(data, 256, Opentxt) != NULL)
	{
		if (j % 4 == 0)
		{
			find = wcschr(data, L'[');
			cutstr(buff, data, 10, find - data + 3);
			txtoffset = (int)wcstoul(buff, &buff1, 16);
			fseek(Openbin, 0, SEEK_SET);
			while (!feof(Openbin))
			{
				fread(&tnum, 4, 1, Openbin);
				if (tnum == txtoffset)
				{
					txtbinoffset = (int)ftell(Openbin) - 4;
				}
			}
		}
		if (j == m)
		{
			slen = wcslen(data);
			newtxtoffset = (int)ftell(WriteFile);
			fseek(WriteFile, txtbinoffset, SEEK_SET);
			fwrite(&newtxtoffset, 4, 1, WriteFile);
			fseek(WriteFile, 0, SEEK_END);
			for (n = 0; n < slen; n++)
			{
				for (l = 0;l < k; l++)
					if (data[n] == tbl[l])
					{
						hlong = wcslen(hex[l]);
						if (hlong / 2 == 1)
						{
							buff2[0] = hex[l][0];
							buff2[1] = hex[l][1];
							hexbuff = (unit8)wcstoul(buff2, &buff1, 16);
							fwrite(&hexbuff, 1, 1, WriteFile);
						}
						if (hlong / 2 == 2)
						{
							buff2[0] = hex[l][0];
							buff2[1] = hex[l][1];
							hexbuff = (unit8)wcstoul(buff2, &buff1, 16);
							fwrite(&hexbuff, 1, 1, WriteFile);
							buff2[0] = hex[l][2];
							buff2[1] = hex[l][3];
							hexbuff = (unit8)wcstoul(buff2, &buff1, 16);
							fwrite(&hexbuff, 1, 1, WriteFile);
						}
					}
			}
			fwrite(&zero, 1, 1, WriteFile);
			m += 4;
		}
		j++;
	}
	fclose(Openbin);
	fclose(Opentxt);
	fclose(WriteFile);
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "CHS");
	Insert();
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}