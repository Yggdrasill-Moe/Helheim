#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include <png.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;
typedef unsigned int   unit32;

unit32 FileNum = 0;//总文件数，初始计数为0

struct index
{
	char FileName[100];//文件名
	unit32 FileSize;//文件大小
}Index[300];

int TileOrder8[] =
{
	0,   1,   4,   5,  16,  17,   20,   21,
	2,   3,   6,   7,  18,  19,   22,   23,
	8,   9,  12,  13,  24,  25,   28,   29,
	10,  11,  14,  15, 26,  27,   30,   31
};

int TileOrder4[] =
{
	0,   2,   8,   10,
	1,   3,   9,   11,
	4,   6,  12,   14,
	5,   7,  13,   15
};

unit32 process_dir(char *dname)
{
	long Handle;
	unit32 i = 0;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.png", &FileInfo)) == -1L)
	{
		printf("没有找到匹配的项目。\n");
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

void ReadPng(FILE *OpenPng, unit32 Width, unit32 Height, unit8* PixelData, unit8* TexData, unit32 bpp)
{
	png_structp png_ptr;
	png_infop info_ptr, end_ptr;
	png_colorp pcolor;
	png_bytep *rows, ptrans;
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
	png_init_io(png_ptr, OpenPng);
	png_read_info(png_ptr, info_ptr);
	png_set_IHDR(png_ptr, info_ptr, Width, Height, bpp, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_get_PLTE(png_ptr, info_ptr, &pcolor, &nplt);
	for (i = 0; i < nplt; i++)
	{
		PixelData[i * 4 + 3] = pcolor[i].red;
		PixelData[i * 4 + 2] = pcolor[i].green;
		PixelData[i * 4 + 1] = pcolor[i].blue;
	}
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_get_tRNS(png_ptr, info_ptr, (png_bytep*)&ptrans, NULL, NULL);
		for (i = 0; i < nplt; i++)
		{
			PixelData[i * 4 + 0] = ptrans[i];
		}
	}
	rows = (png_bytep*)malloc(Height * sizeof(char*));
	if (bpp == 8)
		for (i = 0; i < Height; i++)
			rows[i] = (png_bytep)(TexData + Width*i);
	else if (bpp == 4)
		for (i = 0; i < Height; i++)
			rows[i] = (png_bytep)(TexData + Width*i / 2);
	png_read_image(png_ptr, rows);
	free(rows);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
}

void WriteFile(unit32 FileNum, char *dname)
{
	unit8 *data, *tempdata, *PixelData, buff;
	unit32 i, s, j, k, TexSize, PixelSize, bpp;
	unit16 Width, Height, texoffset, pixeloffset;
	FILE *OpenFile, *WriteFile;
	_chdir(dname);
	for (i = 0; i < FileNum; i++)
	{
		Index[i].FileName[strchr(Index[i].FileName, '.') - Index[i].FileName] = '\0';
		fopen_s(&OpenFile, Index[i].FileName, "rb");
		printf("%s ", Index[i].FileName);
		fseek(OpenFile, 0, SEEK_END);
		Index[i].FileSize = ftell(OpenFile);
		fseek(OpenFile, 0x19, SEEK_SET);
		fread(&buff, 1, 1, OpenFile);
		if (buff == 0x10)
		{
			bpp = 8;
			PixelSize = 0x400;
			printf("bpp:%d ", bpp);
		}
		else if (buff == 0xF)
		{
			bpp = 4;
			PixelSize = 0x40;
			printf("bpp:%d ", bpp);
		}
		else if (buff == 0xC)
		{
			//bpp = 4;
			printf("buff:0xC 疑似bpp4");
			system("pause");
		}
		else
		{
			printf("buff:0x%X 未知bpp类型", buff);
			system("pause");
		}
		fseek(OpenFile, 0x28, SEEK_SET);
		fread(&Width, 2, 1, OpenFile);
		fread(&Height, 2, 1, OpenFile);
		fseek(OpenFile, 0x3C, SEEK_SET);
		fread(&texoffset, 2, 1, OpenFile);
		fread(&pixeloffset, 2, 1, OpenFile);
		PixelData = malloc(PixelSize);
		TexSize = Index[i].FileSize - texoffset;
		tempdata = malloc(TexSize);
		printf("width:%d height:%d texoffset:0x%X pixeloffset:0x%X \n", Width, Height, texoffset, pixeloffset);
		data = malloc(pixeloffset);
		fseek(OpenFile, 0, SEEK_SET);
		fread(data, pixeloffset, 1, OpenFile);
		fclose(OpenFile);
		sprintf(Index[i].FileName, "%s.new", Index[i].FileName);
		fopen_s(&WriteFile, Index[i].FileName, "wb");
		Index[i].FileName[strchr(Index[i].FileName, '.') - Index[i].FileName] = '\0';
		fwrite(data, pixeloffset, 1, WriteFile);
		free(data);
		sprintf(Index[i].FileName, "%s.png", Index[i].FileName);
		fopen_s(&OpenFile, Index[i].FileName, "rb");
		ReadPng(OpenFile, Width, Height, PixelData, tempdata, bpp);
		fwrite(PixelData, PixelSize, 1, WriteFile);
		free(PixelData);
		data = malloc(TexSize);
		tilec(8, 8, tempdata, data, Width, Height, bpp);
		if(bpp == 8)
			for (s = 0, j = 0, k = 0; s < TexSize; s++)
			{
				tempdata[TileOrder8[j] + k] = data[s];
				if (j == 31)
				{
					j = 0;
					k += 32;
				}
				else
					j++;
			}
		else if(bpp == 4)
			for (s = 0, j = 0, k = 0; s < TexSize; s++)
			{
				tempdata[TileOrder4[j] + k] = data[s];
				if (j == 15)
				{
					j = 0;
					k += 16;
				}
				else
					j++;
			}
		fwrite(tempdata, TexSize, 1, WriteFile);
		free(tempdata);
		free(data);
		fclose(WriteFile);
	}
}

int main(int argc, char *argv[])
{
	printf("project：Helheim-3DS WE 2014\n用于将png图片转换为原始文件，将文件夹拖到程序上，文件夹中要有原文件和对应的png文件\nby Darkness-TX 2016.7.8\nEmail：1545492547@qq.com\n\n");
	process_dir(argv[1]);
	WriteFile(FileNum, argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}