#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <png.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;//总文件数，初始计数为0

struct index
{
	char FileName[100];//文件名
}Index[5000];

unit32 process_dir(char *dname)
{
	long Handle;
	unit32 i = 0;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.tex", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目，请将原文件后缀命名为.tex\n");
		system("pause");
		return -1;
	}
	do
	{
		if (FileInfo.name[0] == '.')  //过滤本级目录和父目录
			continue;
		sprintf(Index[FileNum].FileName, FileInfo.name);
		FileNum++;
	} while (_findnext(Handle, &FileInfo) == 0);
	return FileNum;
}

void WritePng4bit(FILE *Pngname, unit32 Width, unit32 Height, unit8* PixelData, unit8* data)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp pcolor;
	unit8 *png_alpha;
	unit32 i = 0, bpp = 4;//4位索引图
	unit32 nplt = 16;//调色板色数
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
	png_init_io(png_ptr, Pngname);
	png_set_IHDR(png_ptr, info_ptr, Width, Height, bpp, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	pcolor = (png_colorp)malloc(nplt*sizeof(png_color));
	png_alpha = (unit8 *)malloc(nplt * sizeof(unit8));
	for (i = 0; i<nplt; i++)
	{
		pcolor[i].red = PixelData[i * 4 + 0];
		pcolor[i].green = PixelData[i * 4 + 1];
		pcolor[i].blue = PixelData[i * 4 + 2];
		png_alpha[i] = PixelData[i * 4 + 3];
	}
	png_set_PLTE(png_ptr, info_ptr, pcolor, nplt);
	png_set_tRNS(png_ptr, info_ptr, (png_bytep)png_alpha, nplt, (png_color_16p)0);
	free(pcolor);
	free(png_alpha);
	png_write_info(png_ptr, info_ptr);
	for (i = 0; i < Height; i++)
		png_write_row(png_ptr, data + i*Width / 2);//4位索引图两像素占一字节，所以除以2
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

void WritePng8bit(FILE *Pngname, unit32 Width, unit32 Height, unit8* PixelData, unit8* data)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp pcolor;
	unit8 *png_alpha;
	unit32 i = 0, bpp = 8;
	unit32 nplt = 256;//调色板数
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		printf("PNG信息创建失败!\n");
		return -1;;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		printf("info信息创建失败!\n");
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return -1;;
	}
	png_init_io(png_ptr, Pngname);
	png_set_IHDR(png_ptr, info_ptr, Width, Height, bpp, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	pcolor = (png_colorp)malloc(nplt*sizeof(png_color));
	png_alpha = (unit8 *)malloc(nplt * sizeof(unit8));
	for (i = 0; i<nplt; i++)
	{
		pcolor[i].red = PixelData[i * 4 + 0];
		pcolor[i].green = PixelData[i * 4 + 1];
		pcolor[i].blue = PixelData[i * 4 + 2];
		png_alpha[i] = PixelData[i * 4 + 3];
	}
	png_set_PLTE(png_ptr, info_ptr, pcolor, nplt);
	png_set_tRNS(png_ptr, info_ptr, (png_bytep)png_alpha, nplt, (png_color_16p)0);
	free(pcolor);
	free(png_alpha);
	png_write_info(png_ptr, info_ptr);
	for (i = 0; i < Height; i++)
	{
		png_write_row(png_ptr, data + i*Width);
	}
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

void WritePng32bit(FILE *Pngname, unit32 Width, unit32 Height, unit8* data)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unit32 i = 0, bpp = 8;
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		printf("PNG信息创建失败!\n");
		return -1;;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		printf("info信息创建失败!\n");
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return -1;;
	}
	png_init_io(png_ptr, Pngname);
	png_set_IHDR(png_ptr, info_ptr, Width, Height, bpp, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	for (i = 0; i < Height; i++)
	{
		png_write_row(png_ptr, data + i*Width * 4);
	}
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

void WriteFile(unit32 FileNum, char *dname)
{
	unit8 buf[8];
	unit8 *PixelData, *data;
	unit32 i, j, Width, Height, flag, datasize;
	FILE *OpenFile, *WriteFile;
	_chdir(dname);
	for (i = 0; i < FileNum; i++)
	{
		fopen_s(&OpenFile, Index[i].FileName, "rb");
		if (OpenFile == NULL)
		{
			printf("打开%s失败", Index[i].FileName);
			system("pause");
			exit(0);
		}
		fread(buf, 8, 1, OpenFile);
		if (strncmp(buf, "Texture ", 8) != 0)
			fseek(OpenFile, 0, SEEK_SET);
		else
			fseek(OpenFile, 4, SEEK_CUR);
		fread(&flag, 4, 1, OpenFile);
		fread(&datasize, 4, 1, OpenFile);
		fread(&Width, 4, 1, OpenFile);
		fread(&Height, 4, 1, OpenFile);
		if (flag == 0x11000100)//4bit
		{
			datasize -= 0x40;
			data = malloc(datasize);
			fread(data, datasize, 1, OpenFile);
			PixelData = malloc(0x40);
			fread(PixelData, 0x40, 1, OpenFile);
			for (j = 0; j < datasize; j++)
				data[j] = (data[j] << 4) | (data[j] >> 4);//4位图这方面pc与掌机家用机有点不同，所以要高地位互换
			printf("%s width:%d height:%d bpp:4\n", Index[i].FileName, Width, Height);
			sprintf(Index[i].FileName, "%s.png", Index[i].FileName);
			fopen_s(&WriteFile, Index[i].FileName, "wb");
			WritePng4bit(WriteFile, Width, Height, PixelData, data);
			free(data);
			free(PixelData);
			fclose(WriteFile);
		}
		else if (flag == 0x21000200)//8bit
		{
			datasize -= 0x400;
			data = malloc(datasize);
			fread(data, datasize, 1, OpenFile);
			PixelData = malloc(0x400);
			fread(PixelData, 0x400, 1, OpenFile);
			printf("%s width:%d height:%d bpp:8\n", Index[i].FileName, Width, Height);
			sprintf(Index[i].FileName, "%s.png", Index[i].FileName);
			fopen_s(&WriteFile, Index[i].FileName, "wb");
			WritePng8bit(WriteFile, Width, Height, PixelData, data);
			free(data);
			free(PixelData);
			fclose(WriteFile);
		}
		else if (flag == 0x81004000)//32bit
		{
			data = malloc(datasize);
			fread(data, datasize, 1, OpenFile);
			printf("%s width:%d height:%d size:0x%X bpp:32\n", Index[i].FileName, Width, Height, datasize);
			sprintf(Index[i].FileName, "%s.png", Index[i].FileName);
			fopen_s(&WriteFile, Index[i].FileName, "wb");
			WritePng32bit(WriteFile, Width, Height, data);
			free(data);
			fclose(WriteFile);
		}
		else if (flag == 0x81014000)//png
		{
			data = malloc(datasize);
			fread(data, datasize, 1, OpenFile);
			printf("%s width:%d height:%d size:0x%X\n", Index[i].FileName, Width, Height, datasize);
			sprintf(Index[i].FileName, "%s.png", Index[i].FileName);
			fopen_s(&WriteFile, Index[i].FileName, "wb");
			fwrite(data, datasize, 1, WriteFile);
			free(data);
			fclose(WriteFile);
		}
		else
		{
			printf("%s width:%d height:%d size:0x%X flag:0x%X 未知的图片文件类型\n", Index[i].FileName, Width, Height, datasize,flag);
		}
		fclose(OpenFile);
	}
}

int main(int argc, char *argv[])
{
	printf("project：Helheim-DATE A LIVE 凛绪Reincarnation ver0.7\n用于转换.tex文件，将tex文件放进文件夹下后拖动文件夹到程序上，\n文件夹中不得有其他文件夹\nby Darkness-TX 2016.3.3\nEmail：1545492547@qq.com\n");
	process_dir(argv[1]);
	WriteFile(FileNum, argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}