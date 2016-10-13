/*
用于导出fmg文件中的文本
by Darkness-TX
2015.12.5
*/
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
}Index[100];

unit32 process_dir(char *dname)
{
	long Handle;
	unit32 i = 0;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.fmg", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目，请将后缀命名为.fmg\n");
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

void WriteTxt(unit32 FileNum, char *dname)
{
	unit8 dumptxt[512];
	wchar_t writetxt[512], number[16];
	unit32 i, j, k, txtoffset = 0, point, s = 0, num, TXTOffsetStart = 0, TXTOffsetEnd = 0;
	FILE *ofp, *wfp;
	_chdir(dname);
	for (i = 0; i < FileNum; i++)
	{
		num = 1;
		ofp = fopen(Index[i].FileName, "rb");
		if (ofp == NULL)
		{
			printf("打开文件失败\n");
			system("pause");
			exit(0);
		}
		wfp = fopen(strcat(Index[i].FileName, ".txt"), "wb");
		fputc(0xff, wfp);
		fputc(0xfe, wfp);
		fseek(ofp, 0x14, SEEK_SET);
		fread(&TXTOffsetStart, 4, 1, ofp);
		fseek(ofp, TXTOffsetStart, SEEK_SET);
		do {
			fread(&TXTOffsetEnd, 4, 1, ofp);
		} while (TXTOffsetEnd == 0);
		k = (TXTOffsetEnd - TXTOffsetStart) / 4;
		fseek(ofp, TXTOffsetStart, SEEK_SET);
		for (j = 0;j < k;j++)
		{
			fread(&txtoffset, 4, 1, ofp);
			point = ftell(ofp);
			if (txtoffset != 0)
			{
				fseek(ofp, txtoffset, SEEK_SET);
				do
				{
					fread(&dumptxt[s], 1, 1, ofp);
					s++;
					if (dumptxt[s - 1] == 0xA)
					{
						dumptxt[s - 1] = 0xD;
						dumptxt[s] = 0xA;
						s++;
					}
				} while (dumptxt[s - 1] != '\0');
				s = 0;
				MultiByteToWideChar(932, 0, dumptxt, -1, writetxt, strlen(dumptxt));
				writetxt[strlen(dumptxt)] = L'\0';
				swprintf(number, 16, L"%08d", num);
				fwprintf(wfp, L"####%s####\r\n", number);
				fwprintf(wfp, writetxt);
				fwprintf(wfp, L"\r\n\r\n");
				num++;
			}
			fseek(ofp, point, SEEK_SET);
		}
		fclose(ofp);
		fclose(wfp);
		printf("%s %d段文本\n", Index[i].FileName, num - 1);
	}
}

int main(int argc, char *argv[])
{
	//setlocale(LC_ALL, "CHS");
	printf("project：Helheim-九怨\n用于导出.fmg文件中的文本，将解包文件夹拖到程序上\nby Darkness-TX 2015.12.5\nEmail：1545492547@qq.com\n");
	process_dir(argv[1]);
	WriteTxt(FileNum, argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}