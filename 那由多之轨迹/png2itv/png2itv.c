/*
用于将png转换成供Longinus用的自制itv
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

void png2itv(char *fname)
{
	FILE *src = NULL, *dst = NULL;
	unit32 i = 0, k = 0;
	unit32 width = 0, height = 0, bpp = 0, color_type = 0, color_num = 256;
	png_structp png_ptr;
	png_infop info_ptr, end_ptr;
	png_colorp pcolor;
	png_bytep *rows, png_alpha;
	unit8 dstname[256], *data = NULL, *odata = NULL;
	src = fopen(fname, "rb");
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
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
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		system("pause");
		exit(0);
	}
	end_ptr = png_create_info_struct(png_ptr);
	if (end_ptr == NULL)
	{
		printf("end信息创建失败!\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		system("pause");
		exit(0);
	}
	png_init_io(png_ptr, src);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bpp, &color_type, NULL, NULL, NULL);
	png_get_PLTE(png_ptr, info_ptr, &pcolor, &color_num);
	png_get_tRNS(png_ptr, info_ptr, (png_bytep*)&png_alpha,NULL, NULL);
	strcpy(dstname, fname);
	*strrchr(dstname, '.') = '\0';
	sprintf(dstname, "%s.itv", dstname);
	odata = malloc(0x100 * 4);
	for (k = 0; k < 0x100; k++)
	{
		odata[k * 4 + 0] = pcolor[k].red;
		odata[k * 4 + 1] = pcolor[k].green;
		odata[k * 4 + 2] = pcolor[k].blue;
		odata[k * 4 + 3] = png_alpha[k];
	}
	dst = fopen(dstname, "wb");
	fwrite(odata, 0x100 * 4, 1, dst);
	free(odata);
	odata = malloc(width * height);
	data = odata;
	rows = (png_bytep*)malloc(height * sizeof(BYTE*));
	for (k = 0; k < height; k++)
		rows[k] = (png_bytep)(data + width*k);
	png_read_image(png_ptr, rows);
	free(rows);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
	fclose(src);
	fwrite(data, width * height, 1, dst);
	free(odata);
	fclose(dst);
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-falcom\n用于将png转化为供Longinus用的自制itv。\n将png文件拖到程序上。\nby Darkness-TX 2019.10.28\n\n");
	png2itv(argv[1]);
	printf("已完成，总文件数1\n");
	system("pause");
	return 0;
}