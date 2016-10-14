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

unit32 process_dir(char *dname)
{
	long Handle;
	unit32 i = 0;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.lz", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目，请将后缀命名为.LZ\n");
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

void ImportBip(unit32 FileNum, char *dname)
{
	FILE *Openbin, *Opentxt, *WriteFile, *Tbl;
	unit32 i, j, k = 0, slen, txtoffset, txtbinoffset, tnum, m = 6, n, l = 0, hlong, newtxtoffset;
	wchar_t tbl[4000], hex[4000][5], data[256], *find, buff[10],*buff1, buff2[2];
	unit8 *data1, hexbuff, zero = '\0', flag, talkflag = 0;
	_chdir("..");
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
	_chdir(dname);
	for (i = 0; i < FileNum; i++)
	{
		fopen_s(&Openbin, Index[i].FileName, "rb");
		if (Openbin == NULL)
		{
			printf("打开原文件失败\n");
			system("pause");
		}
		fopen_s(&Opentxt, strcat(Index[i].FileName,".txt"), "rt,ccs=UNICODE");
		if (Opentxt == NULL)
		{
			printf("打开文本文件失败\n");
			system("pause");
		}
		fopen_s(&WriteFile, strcat(Index[i].FileName, ".new"), "wb");
		if (WriteFile == NULL)
		{
			printf("打开导入文件失败\n");
			system("pause");
		}
		j = 1;
		printf("%s\n", Index[i].FileName);

		while (fgetws(data, 256, Opentxt) != NULL)
		{
			if (j % 4 == 0)
			{
				find = wcschr(data, L'[');
				cutstr(buff, data, 10, find - data + 3);
				txtoffset = (int)wcstoul(buff, &buff1, 16);
				if (j == 4)
				{
					data1 = malloc(txtoffset);
					fread(data1, txtoffset, 1, Openbin);
					fwrite(data1, txtoffset, 1, WriteFile);
					free(data1);
				}
			}
			if (j == m)
			{
				slen = wcslen(data);
				if (data[slen - 2] == L'P')
					talkflag = 1;
				else
					talkflag = 0;
				fseek(Openbin, 4, SEEK_SET);
				while (!feof(Openbin))
				{
					fread(&tnum, 4, 1, Openbin);
					if (tnum == txtoffset)
					{
						fseek(Openbin, -8, SEEK_CUR);
						fread(&flag, 1, 1, Openbin);
						fseek(Openbin, 7, SEEK_CUR);
						if (flag != 0x69 && talkflag == 1)
							continue;
						txtbinoffset = (int)ftell(Openbin) - 4;
						break;
					}
				}
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
							if(hlong / 2 == 1)
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
		fseek(Openbin, -24, SEEK_END);
		data1 = malloc(24);
		fread(data1, 24, 1, Openbin);
		fwrite(data1, 24, 1, WriteFile);
		free(data1);
		m = 6;
		fclose(Openbin);
		fclose(Opentxt);
		fclose(WriteFile);
	}
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "CHS");
	process_dir(argv[1]);
	ImportBip(FileNum, argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}