/*
用于将多张png合成一张
made by Darkness-TX
2019.10.28
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
	if ((Handle = _findfirst("*.png", &FileInfo)) == -1L)
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
	_chdir("..");
	return FileNum;
}

void png_pack(char *fname)
{
	FILE *src = NULL, *dst = NULL;
	unit32 i = 0, k = 0;
	unit32 width = 0, height = 0;
	unit8 dstname[256], *odata = NULL, *data = NULL;
	sprintf(dstname, "%s.png", fname);
	dst = fopen(dstname, "wb");
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *rows;
	bool isfirst = true;
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
	_chdir(fname);
	for (i = 0; i < FileNum; i++)
	{
		printf("%s\n", Index[i].name);
		src = fopen(Index[i].name, "rb");
		png_structp spng_ptr;
		png_infop sinfo_ptr, send_ptr;
		spng_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (spng_ptr == NULL)
		{
			printf("PNG信息创建失败!\n");
			system("pause");
			exit(0);
		}
		sinfo_ptr = png_create_info_struct(spng_ptr);
		if (sinfo_ptr == NULL)
		{
			printf("info信息创建失败!\n");
			png_destroy_read_struct(&spng_ptr, (png_infopp)NULL, (png_infopp)NULL);
			system("pause");
			exit(0);
		}
		send_ptr = png_create_info_struct(spng_ptr);
		if (send_ptr == NULL)
		{
			printf("end信息创建失败!\n");
			png_destroy_read_struct(&spng_ptr, &sinfo_ptr, (png_infopp)NULL);
			system("pause");
			exit(0);
		}
		png_init_io(spng_ptr, src);
		png_read_info(spng_ptr, sinfo_ptr);
		if (isfirst)
		{
			unit32 bpp = 0, color_type = 0;
			png_get_IHDR(spng_ptr, sinfo_ptr, &width, &height, &bpp, &color_type, NULL, NULL, NULL);
			png_set_IHDR(png_ptr, info_ptr, width, height*FileNum, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			png_write_info(png_ptr, info_ptr);
			odata = malloc(width*height * 4 * FileNum);
			data = odata;
			isfirst = false;
		}
		rows = (png_bytep*)malloc(height * sizeof(BYTE*));
		for (k = 0; k < height; k++)
			rows[k] = (png_bytep)(data + width*k * 4);
		png_read_image(spng_ptr, rows);
		free(rows);
		png_read_end(spng_ptr, sinfo_ptr);
		png_destroy_read_struct(&spng_ptr, &sinfo_ptr, &send_ptr);
		fclose(src);
		data += width*height * 4;
	}
	for (unit32 l = 0; l < height * FileNum; l++)
		png_write_row(png_ptr, odata + l * width * 4);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	free(odata);
	fclose(dst);
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-那由多之轨迹\n用于将多张png合成一张。\n将文件夹拖到程序上。\nby Darkness-TX 2019.10.28\n\n");
	process_dir(argv[1]);
	png_pack(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}