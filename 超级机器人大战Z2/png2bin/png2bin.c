/*
用于导入图片到bin文件
made by Darkness-TX
2023.04.10
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

unit32 FileNum = 0;//总文件数，初始计数为0

unit32 CheckString(char* buff)
{
	if (strncmp(buff, "0x", 2) == 0 || strncmp(buff, "0X", 2) == 0)
		return strtoul(buff + 2, NULL, 16);
	else
		return strtoul(buff, NULL, 10);
}

void tilec(int twidth, int thigh, unit8* indata, unit8* outdata, int width, int high, unit32 bpp)
{
	int w, h, tw, th, ofs, b;
	int wlen;
	unit8* tempin, * tempout;
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
				ofs += wlen * bpp / 8;
			}
			tempin += twidth * bpp / 8;
		}
		tempin += wlen * bpp / 8 * (thigh - 1);
	}
}

void ReadPng(FILE* Pngname, unit32 Width, unit32 Height, unit8* PaletteData, unit8* data, unit32 bpp)
{
	png_structp png_ptr;
	png_infop info_ptr, end_ptr;
	png_colorp pcolor;
	png_bytep png_alpha;
	png_bytep* rows;
	unit32 i = 0;
	unit32 nplt;//调色板数
	if (bpp == 8)
		nplt = 256;
	else if (bpp == 4)
		nplt = 16;
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
	png_init_io(png_ptr, Pngname);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	//png_set_IHDR(png_ptr, info_ptr, Width, Height, bpp, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_get_PLTE(png_ptr, info_ptr, &pcolor, &nplt);
	for (i = 0; i < nplt; i++)
	{
		PaletteData[i * 4 + 0] = pcolor[i].red;
		PaletteData[i * 4 + 1] = pcolor[i].green;
		PaletteData[i * 4 + 2] = pcolor[i].blue;
		PaletteData[i * 4 + 3] = 0xFF;
	}
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_get_tRNS(png_ptr, info_ptr, (png_bytep*)&png_alpha, NULL, NULL);
		for (i = 0; i < nplt; i++)
			PaletteData[i * 4 + 3] = png_alpha[i];
	}
	rows = (png_bytep*)malloc(Height * sizeof(char*));
	if (bpp == 8)
		for (i = 0; i < Height; i++)
			rows[i] = (png_bytep)(data + i * Width);
	else if (bpp == 4)
		for (i = 0; i < Height; i++)
			rows[i] = (png_bytep)(data + i * Width / 2);
	png_read_image(png_ptr, rows);
	free(rows);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
}

void png2bin(char* fname, char* idxname)
{
	FILE *src = NULL, *dst = NULL, *org = NULL;
	unit8 *data = NULL, *palette = NULL, *udata = NULL, dstname[MAX_PATH];
	unit32 height = 0, width = 0, theight = 0, twidth = 0, bpp = 0, data_offset = 0, palette_offset = 0, count = 0, withpalette = 1, filesize = 0, i = 0;
	char dirPath[MAX_PATH];
	char iniPath[MAX_PATH];
	char buff[30], keybuff[50];
	GetCurrentDirectoryA(MAX_PATH, dirPath);
	sprintf(iniPath, "%s\\%s", dirPath, "png.ini");
	if (_access(iniPath, 4) == -1)
	{
		printf("初始化失败，请确认目录下是否含有png.ini\n");
		system("pause");
		exit(0);
	}
	count = GetPrivateProfileIntA(idxname, "count", 1, iniPath);
	sprintf(dstname, "%s.new", fname);
	dst = fopen(dstname, "wb");
	org = fopen(fname, "rb");
	fseek(org, 0, SEEK_END);
	filesize = ftell(org);
	fseek(org, 0, SEEK_SET);
	data = malloc(filesize);
	fread(data, filesize, 1, org);
	fwrite(data, filesize, 1, dst);
	fclose(org);
	free(data);
	for (i = 1; i <= count; i++)
	{
		if (i == 1)
		{
			GetPrivateProfileStringA(idxname, "data_offset", "0", buff, MAX_PATH, iniPath);
			data_offset = CheckString(buff);
			GetPrivateProfileStringA(idxname, "palette_offset", "0", buff, MAX_PATH, iniPath);
			palette_offset = CheckString(buff);
			GetPrivateProfileStringA(idxname, "height", "0", buff, MAX_PATH, iniPath);
			height = CheckString(buff);
			GetPrivateProfileStringA(idxname, "width", "0", buff, MAX_PATH, iniPath);
			width = CheckString(buff);
			GetPrivateProfileStringA(idxname, "theight", "0", buff, MAX_PATH, iniPath);
			theight = CheckString(buff);
			GetPrivateProfileStringA(idxname, "twidth", "0", buff, MAX_PATH, iniPath);
			twidth = CheckString(buff);
			GetPrivateProfileStringA(idxname, "bpp", "0", buff, MAX_PATH, iniPath);
			bpp = CheckString(buff);
			withpalette = GetPrivateProfileIntA(idxname, "withpalette", 1, iniPath);
			sprintf(dstname, "%s.png", fname);
		}
		else
		{
			sprintf(keybuff, "data_offset%d", i - 1);
			GetPrivateProfileStringA(idxname, keybuff, "0", buff, MAX_PATH, iniPath);
			data_offset = CheckString(buff);
			sprintf(keybuff, "palette_offset%d", i - 1);
			GetPrivateProfileStringA(idxname, keybuff, "0", buff, MAX_PATH, iniPath);
			palette_offset = CheckString(buff);
			sprintf(keybuff, "height%d", i - 1);
			GetPrivateProfileStringA(idxname, keybuff, "0", buff, MAX_PATH, iniPath);
			height = CheckString(buff);
			sprintf(keybuff, "width%d", i - 1);
			GetPrivateProfileStringA(idxname, keybuff, "0", buff, MAX_PATH, iniPath);
			width = CheckString(buff);
			sprintf(keybuff, "theight%d", i - 1);
			GetPrivateProfileStringA(idxname, keybuff, "0", buff, MAX_PATH, iniPath);
			theight = CheckString(buff);
			sprintf(keybuff, "twidth%d", i - 1);
			GetPrivateProfileStringA(idxname, keybuff, "0", buff, MAX_PATH, iniPath);
			twidth = CheckString(buff);
			sprintf(keybuff, "bpp%d", i - 1);
			GetPrivateProfileStringA(idxname, keybuff, "0", buff, MAX_PATH, iniPath);
			bpp = CheckString(buff);
			sprintf(keybuff, "withpalette%d", i - 1);
			withpalette = GetPrivateProfileIntA(idxname, keybuff, 1, iniPath);
			sprintf(dstname, "%s.%d.png", fname, i - 1);
		}
		src = fopen(dstname, "rb");
		if (bpp == 8)
		{
			data = malloc(height * width);
			udata = malloc(height * width);
			palette = malloc(256 * 4);
			ReadPng(src, width, height, palette, data, bpp);
			tilec(twidth, theight, data, udata, width, height, bpp);
			free(data);
			fseek(dst, data_offset, SEEK_SET);
			fwrite(udata, height * width, 1, dst);
			if (withpalette)
			{
				fseek(dst, palette_offset, SEEK_SET);
				fwrite(palette, 256 * 4, 1, dst);
			}
			free(palette);
			free(udata);
		}
		else if (bpp == 4)
		{
			data = malloc(height * width / 2);
			udata = malloc(height * width / 2);
			palette = malloc(16 * 4);
			ReadPng(src, width, height, palette, data, bpp);
			for (unit32 j = 0; j < height * width / 2; j++)
				data[j] = (data[j] & 0x0F) << 4 | (data[j] & 0xF0) >> 4;
			tilec(twidth, theight, data, udata, width, height, bpp);
			free(data);
			fseek(dst, data_offset, SEEK_SET);
			fwrite(udata, height * width / 2, 1, dst);
			if (withpalette)
			{
				fseek(dst, palette_offset, SEEK_SET);
				fwrite(palette, 16 * 4, 1, dst);
			}
			free(palette);
			free(udata);
		}
		else
		{
			printf("不支持的bpp类型！bpp:%d\n", bpp);
			system("pause");
		}
		fclose(src);
		FileNum++;
	}
	fclose(dst);
}

int main(int argc, char* argv[])
{
	printf("project：Helheim-超级机器人大战Z2\n用于导入图片到bin文件。\nby Darkness-TX 2023.04.10\n\n");
	png2bin(argv[1], argv[2]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}