/*
用于将Longinus.prx导出的位图数据转换为png
made by Darkness-TX
2019.10.16
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <Windows.h>
#include <locale.h>
#include <stdbool.h>
#include <png.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;//总文件数，初始计数为0

struct index
{
	char name[MAX_PATH];//文件名
	unit32 FileSize;//文件大小
}Index[1000];

unit32 process_dir(char *dname)
{
	long Handle;
	unit32 i = 0;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.bin", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目\n");
		system("pause");
		return -1;
	}
	do
	{
		if (FileInfo.name[0] == '.')  //过滤本级目录和父目录
			continue;
		sprintf(Index[FileNum].name, FileInfo.name);
		Index[FileNum].FileSize = FileInfo.size;
		FileNum++;
	} while (_findnext(Handle, &FileInfo) == 0);
	return FileNum;
}

void bin2png(char *fname)
{
	FILE *src = NULL, *dst = NULL;
	unit32 i = 0, k = 0;
	unit16 width = 0, height = 0;
	png_colorp pcolor = NULL;
	unit8 *png_alpha = NULL, *data = NULL, dstname[MAX_PATH];
	for (i = 0; i < FileNum; i++)
	{
		src = fopen(Index[i].name, "rb");
		sprintf(dstname, "%s.png", Index[i].name);
		pcolor = (png_colorp)malloc(0x100 * sizeof(png_color));
		png_alpha = (unit8 *)malloc(0x100 * sizeof(unit8));
		data = malloc(0x100 * 4);
		fread(&width, 2, 1, src);
		fread(&height, 2, 1, src);
		fread(data, 0x100 * 4, 1, src);
		for (k = 0; k < 0x100; k++)
		{
			pcolor[k].red = data[k * 4 + 0];
			pcolor[k].green = data[k * 4 + 1];
			pcolor[k].blue = data[k * 4 + 2];
			png_alpha[k] = data[k * 4 + 3];
		}
		free(data);
		data = malloc(width * height);
		fread(data, width * height, 1, src);
		fclose(src);
		dst = fopen(dstname, "wb");
		png_structp png_ptr;
		png_infop info_ptr;
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr == NULL)
		{
			printf("PNG信息创建失败!\n");
			system("pause");
			exit(0);
		}
		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL)
		{
			printf("info信息创建失败!\n");
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			system("pause");
			exit(0);
		}
		png_init_io(png_ptr, dst);
		png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_set_PLTE(png_ptr, info_ptr, pcolor, 0x100);
		png_set_tRNS(png_ptr, info_ptr, (png_bytep)png_alpha, 0x100, (png_color_16p)0);
		png_write_info(png_ptr, info_ptr);
		for (unit32 l = 0; l < height; l++)
			png_write_row(png_ptr, data + l * width);
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(dst);
		free(data);
		free(pcolor);
		free(png_alpha);
	}
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-那由多之轨迹\n用于将Longinus.prx导出的位图数据转换为png。\n将文件夹拖到程序上。\nby Darkness-TX 2019.10.16\n\n");
	process_dir(argv[1]);
	bin2png(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}