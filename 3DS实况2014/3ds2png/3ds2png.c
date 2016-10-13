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
}Index[3000];

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
	if ((Handle = _findfirst("*.*", &FileInfo)) == -1L)
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
						tempout[ofs + tw] = tempin[0];
					else if (bpp == 4)
						tempout[ofs + tw / 2] = tempin[0];
					tempin++;
				}
				ofs += wlen*bpp / 8;
			}
			tempout += twidth*bpp / 8;
		}
		tempout += wlen*bpp / 8 * (thigh - 1);
	}
}

void WritePng(FILE *Pngname, unit32 Width, unit32 Height, unit8* PixelData, unit8* data, unit32 bpp)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp pcolor;
	unit8 *png_alpha;
	unit32 i = 0;
	unit32 nplt;//调色板数
	if (bpp == 8)
		nplt = 256;
	else if (bpp == 4)
		nplt = 16;
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
	pcolor = (png_colorp)malloc(nplt * sizeof(png_color));
	png_alpha = (unit8 *)malloc(nplt * sizeof(unit8));
	for (i = 0; i<nplt; i++)
	{
		pcolor[i].red = PixelData[i * 4 + 3];
		pcolor[i].green = PixelData[i * 4 + 2];
		pcolor[i].blue = PixelData[i * 4 + 1];
		png_alpha[i] = PixelData[i * 4 + 0];
	}
	png_set_PLTE(png_ptr, info_ptr, pcolor, nplt);
	png_set_tRNS(png_ptr, info_ptr, (png_bytep)png_alpha, nplt, (png_color_16p)0);
	free(pcolor);
	free(png_alpha);
	png_write_info(png_ptr, info_ptr);
	if (bpp == 8)
		for (i = 0; i < Height; i++)
			png_write_row(png_ptr, data + i*Width);
	else if (bpp == 4)
		for (i = 0; i < Height; i++)
			png_write_row(png_ptr, data + i*Width / 2);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

void WriteFile(unit32 FileNum, char *dname)
{
	unit8 *data, *tempdata, *PixelData, buff, buff2;
	unit32 i, s, j, k, TexSize, PixelSize, flag, bpp;
	unit16 Width, Height, texoffset, pixeloffset;
	FILE *OpenFile, *WriteFile;
	_chdir(dname);
	for (i = 0; i < FileNum; i++)
	{
		fopen_s(&OpenFile, Index[i].FileName, "rb");
		printf("%s ", Index[i].FileName);
		fseek(OpenFile, 4, SEEK_SET);
		fread(&flag, 4, 1, OpenFile);
		if (flag != 0x33647301)//.sd3
			printf("flag不是.sd3，跳过\n");
		else
		{
			fseek(OpenFile, 0x18, SEEK_SET);
			fread(&buff2, 1, 1, OpenFile);
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
			fseek(OpenFile, pixeloffset, SEEK_SET);
			PixelData = malloc(PixelSize);
			fread(PixelData, PixelSize, 1, OpenFile);
			fseek(OpenFile, texoffset, SEEK_SET);
			TexSize = Index[i].FileSize - texoffset;
			tempdata = malloc(TexSize);
			fread(tempdata, TexSize, 1, OpenFile);
			printf("width:%d height:%d texoffset:0x%X pixeloffset:0x%X\n", Width, Height,texoffset,pixeloffset);
			data = malloc(TexSize);
			sprintf(Index[i].FileName, "%s.png", Index[i].FileName);
			if (bpp == 8)
			{
				if (buff2 == 2 || buff2 == 4 || buff2 == 8)
				{
					fopen_s(&WriteFile, Index[i].FileName, "wb");
					for (s = 0, j = 0, k = 0; s < TexSize; s++)
					{
						data[s] = tempdata[TileOrder8[j] + k];
						if (j == 31)
						{
							j = 0;
							k += 32;
						}
						else
							j++;
					}
					//fwrite(data, TexSize, 1, WriteFile);
					//fwrite(PixelData, PixelSize, 1, WriteFile);
					tilec(8, 8, data, tempdata, Width, Height, bpp);
					WritePng(WriteFile, Width, Height, PixelData, tempdata, bpp);
					fclose(WriteFile);
				}
				else
				{
					printf("buff2:%d", buff2);
					system("pause");
					/*fopen_s(&WriteFile, Index[i].FileName, "wb");
					//fwrite(data, TexSize, 1, WriteFile);
					//fwrite(PixelData, PixelSize, 1, WriteFile);
					tilec(8, 8, tempdata, data, Width, Height, bpp);
					WritePng(WriteFile, Width, Height, PixelData, data, bpp);
					fclose(WriteFile);*/
				}
			}
			else if (bpp == 4)
			{
				if (buff2 == 2||buff2==4||buff2 == 8)
				{
					fopen_s(&WriteFile, Index[i].FileName, "wb");
					for (s = 0, j = 0, k = 0; s < TexSize; s++)
					{
						data[s] = tempdata[TileOrder4[j] + k];
						if (j == 15)
						{
							j = 0;
							k += 16;
						}
						else
							j++;
					}
					tilec(8, 8, data, tempdata, Width, Height, bpp);
					//fwrite(data, TexSize, 1, WriteFile);
					//fwrite(PixelData, PixelSize, 1, WriteFile);
					WritePng(WriteFile, Width, Height, PixelData, tempdata, bpp);
					fclose(WriteFile);
				}
				else
				{
					printf("buff2:%d", buff2);
					system("pause");
					/*fopen_s(&WriteFile, Index[i].FileName, "wb");
					//fwrite(data, TexSize, 1, WriteFile);
					//fwrite(PixelData, PixelSize, 1, WriteFile);
					tilec(8, 8, tempdata, data, Width, Height, bpp);
					WritePng(WriteFile, Width, Height, PixelData, data, bpp);
					fclose(WriteFile);*/
				}
			}
			free(data);
			free(PixelData);
			free(tempdata);
		}
		fclose(OpenFile);
	}
}

int main(int argc, char *argv[])
{
	printf("project：Helheim-3DS WE 2014\n用于将文件转换为png图片，文件放在一个文件夹里，将文件夹拖到程序上\nby Darkness-TX 2016.7.5\nEmail：1545492547@qq.com\n\n");
	process_dir(argv[1]);
	WriteFile(FileNum, argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	//system("pause");
	return 0;
}