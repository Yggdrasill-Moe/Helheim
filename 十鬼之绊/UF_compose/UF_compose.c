/*
用于组合文件头为UF的字库文件
made by Darkness-TX
2021.01.05
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
#include <png.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

struct compose_info
{
	unit32 width;
	unit32 height;
	unit8 *data;
}Base_Info, Outline_Info;

void tilec(int twidth, int thigh, unit8 *indata, unit8 *outdata, int width, int high, unit32 bpp)
{
	int w, h, tw, th, ofs, b;
	int wlen;
	unit8 *tempin, *tempout;
	tempin = indata;
	tempout = outdata;
	wlen = width;
	if (bpp == 8)
		b = 1;
	else if (bpp == 4)
		b = 2;
	for (h = 0; h < high; h += thigh)
	{
		for (w = 0; w < width; w += twidth)
		{
			for (th = 0, ofs = 0; th < thigh; th++)
			{
				for (tw = 0; tw < twidth; tw += b)
				{
					if (bpp == 8)
						tempout[0] = tempin[ofs + tw];
					else if (bpp == 4)
						tempout[0] = tempin[ofs + tw / 2];
					tempout++;
				}
				ofs += wlen*bpp / 8;
			}
			tempin += twidth*bpp / 8;
		}
		tempin += wlen*bpp / 8 * (thigh - 1);
	}
}

unit8* ReadPng(FILE *pngfile, unit32 *width, unit32 *height)
{
	png_structp png_ptr;
	png_infop info_ptr, end_ptr;
	png_bytep *rows = NULL;
	unit32 i = 0, format = 0, bpp = 0;
	unit8 *data = NULL;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		printf("PNG信息创建失败!\n");
		exit(0);
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		printf("info信息创建失败!\n");
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		exit(0);
	}
	end_ptr = png_create_info_struct(png_ptr);
	if (end_ptr == NULL)
	{
		printf("end信息创建失败!\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		exit(0);
	}
	png_init_io(png_ptr, pngfile);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)width, (png_uint_32*)height, &bpp, &format, NULL, NULL, NULL);
	if (format != PNG_COLOR_TYPE_PALETTE || bpp != 4)
	{
		printf("两张原始图片需为4位索引图！format:%d bpp:%d\n", format, bpp);
		system("pause");
		exit(0);
	}
	data = malloc((*width)*(*height) / 2);
	rows = (png_bytep*)malloc(*height * sizeof(char*));
	for (i = 0; i < *height; i++)
		rows[i] = (png_bytep)(data + (*width)*i / 2);
	png_read_image(png_ptr, rows);
	free(rows);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
	return data;
}

void Compose(char *fname)
{
	FILE *src = NULL, *dst = NULL;
	unit8 *indata = NULL, *outdata = NULL, temp = 0;
	unit32 i = 0, filesize = 0, ofilesize = 0;
	src = fopen("base.png", "rb");
	Base_Info.data = ReadPng(src, &Base_Info.width, &Base_Info.height);
	fclose(src);
	src = fopen("outline.png", "rb");
	Outline_Info.data = ReadPng(src, &Outline_Info.width, &Outline_Info.height);
	fclose(src);
	if (Base_Info.width != Outline_Info.width || Base_Info.height != Outline_Info.height)
	{
		printf("两张原始图片的长宽不一致！\nBase:\twidth:%d height:%d\nOutline:\twidth:%d height:%d\n", Base_Info.width, Base_Info.height, Outline_Info.width, Outline_Info.height);
		system("pause");
		exit(0);
	}
	filesize = Base_Info.width * Base_Info.height / 2;
	indata = malloc(filesize);
	for (i = 0; i < filesize; i++)
	{
		if (Base_Info.data[i] == Outline_Info.data[i])
			indata[i] = Base_Info.data[i];
		else
		{
			temp = 0;
			if ((Base_Info.data[i] & 0xF0) >> 4 >= 5)
				temp = Base_Info.data[i] & 0xF0;
			else
				temp = Outline_Info.data[i] & 0xF0;
			if ((Base_Info.data[i] & 0x0F) >= 5)
				temp += Base_Info.data[i] & 0x0F;
			else
				temp += Outline_Info.data[i] & 0x0F;
			indata[i] = temp;
		}
	}
	src = fopen(fname, "rb");
	fseek(src, 0, SEEK_END);
	ofilesize = ftell(src);
	fseek(src, 0, SEEK_SET);
	outdata = malloc(ofilesize);
	fread(outdata, ofilesize, 1, src);
	fclose(src);
	dst = fopen("UF.bin", "wb");
	fwrite(outdata, ofilesize, 1, dst);
	fseek(dst, 0, SEEK_SET);
	free(outdata);
	outdata = malloc(filesize);
	memset(outdata, 0, filesize);
	tilec(22, 22, indata, outdata, Base_Info.width, Base_Info.height, 4);
	free(indata);
	fwrite(outdata, filesize, 1, dst);
	free(outdata);
	fclose(dst);
	free(Base_Info.data);
	free(Outline_Info.data);
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-十鬼之绊\n用于组合文件头为UF的字库文件。\n将字库文件拖到程序上。\nby Darkness-TX 2021.01.05\n\n");
	Compose(argv[1]);
	printf("已完成。\n");
	system("pause");
	return 0;
}