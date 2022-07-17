/*
用于解包tex文件
made by Darkness-TX
2022.07.12
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include <Windows.h>
#include <png.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;
typedef unsigned __int64 unit64;

unit32 FileNum = 0;//包文件数，初始计数为0

typedef struct texture_s
{
	unit8 magic[20];
	unit32 block_length;//块大小
	unit64 flag;//图片类型
	unit32 pic_length;//图片大小
	unit16 width;
	unit16 height;
}texture_t;

typedef struct parts_s
{
	unit8 magic[20];
	unit32 length;//长度
	unit32 frame_num;//块数
}parts_t;

typedef struct frame_s
{
	unit32 unk1;
	unit32 unk2;
	unit32 frameWidth;
	unit32 frameHeight;
	unit32 frameXScale;
	unit32 frameYScale;
	unit32 frameWidthScale;
	unit32 frameHeightScale;
}frame_t;

typedef struct anime_s
{
	unit8 magic[20];
	unit32 length;//长度
	unit32 info_num;//块数
}anime_t;

struct index
{
	char FileName[MAX_PATH];//文件名
}Index[5000];

unit32 process_dir(char* dname)
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

void WritePng8bit(FILE* Pngname, unit32 Width, unit32 Height, unit8* data)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unit32 i = 0, bpp = 8;
	unit8 *udata = NULL;
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
	png_set_IHDR(png_ptr, info_ptr, Width, Height, bpp, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	udata = malloc(Width * Height * 4);
	memset(udata, 0xFF, Width * Height * 4);
	for (i = 0; i < Width * Height; i++)
		udata[i * 4 + 3] = data[i];
	for (i = 0; i < Height; i++)
	{
		png_write_row(png_ptr, udata + i * Width * 4);
	}
	free(udata);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

void WritePng32bit(FILE* Pngname, unit32 Width, unit32 Height, unit8* data)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unit32 i = 0, bpp = 8;
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
	png_set_IHDR(png_ptr, info_ptr, Width, Height, bpp, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	for (i = 0; i < Height; i++)
	{
		png_write_row(png_ptr, data + i * Width * 4);
	}
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

void Unpack(char *dname)
{
	unit8 *data = NULL, namebuff[MAX_PATH];
	FILE *src = NULL, *dst = NULL, *ini = NULL;
	unit32 i = 0, j = 0;
	_chdir(dname);
	for (i = 0; i < FileNum; i++)
	{
		src = fopen(Index[i].FileName, "rb");
		if (src == NULL)
		{
			printf("打开%s失败", Index[i].FileName);
			system("pause");
			exit(0);
		}
		texture_t Texture;
		fread(Texture.magic, 20, 1, src);
		sprintf(namebuff, "%s.ini", Index[i].FileName);
		ini = fopen(namebuff, "w");
		fputs("[Texture]\n", ini);
		if (strncmp(Texture.magic, "Texture ", 8) != 0)
		{
			fputs("Head=0\n",ini);
			fseek(src, 0, SEEK_SET);
			Texture.block_length = 0;
		}
		else
		{
			fputs("Head=1\n", ini);
			fread(&Texture.block_length, 4, 1, src);
		}
		fread(&Texture.flag, 8, 1, src);
		fread(&Texture.pic_length, 4, 1, src);
		fread(&Texture.width, 2, 1, src);
		fread(&Texture.height, 2, 1, src);
		fprintf(ini, "width=%d\nheight=%d\n\n", Texture.width, Texture.height);
		printf("%s size:0x%X width:%d height:%d\n", Index[i].FileName, Texture.pic_length, Texture.width, Texture.height);
		data = malloc(Texture.pic_length);
		fread(data, Texture.pic_length, 1, src);
		if (Texture.flag == 0x0810000100004000)//png
		{
			sprintf(namebuff, "%s.png", Index[i].FileName);
			dst = fopen(namebuff, "wb");
			fwrite(data, Texture.pic_length, 1, dst);
			free(data);
			fclose(dst);
		}
		else if (Texture.flag == 0x0810000000004000)//32位rgba
		{
			sprintf(namebuff, "%s.png", Index[i].FileName);
			dst = fopen(namebuff, "wb");
			WritePng32bit(dst, Texture.width, Texture.height, data);
			free(data);
			fclose(dst);
		}
		else if (Texture.flag == 0x0210000000000080)//8位灰度图
		{
			sprintf(namebuff, "%s.png", Index[i].FileName);
			dst = fopen(namebuff, "wb");
			WritePng8bit(dst, Texture.width, Texture.height, data);
			free(data);
			fclose(dst);
		}
		else
		{
			printf("暂不支持的文件类型！flag:0x%llX\n", Texture.flag);
			system("pause");
			free(data);
			exit(0);
		}
		if (Texture.block_length != 0)
		{
			fseek(src, Texture.block_length + 7 & ~7, SEEK_SET);
			parts_t Parts;
			fread(Parts.magic, 20, 1, src);
			if (strncmp(Parts.magic, "Parts   ", 8) != 0)
			{
				printf("不支持的文件类型，请检查是否有Parts块\n");
				system("pause");
				exit(0);
			}
			fread(&Parts.length, 4, 1, src);
			fread(&Parts.frame_num, 4, 1, src);
			fputs("[Parts]\n", ini);
			fprintf(ini, "frame_num=%d\n", Parts.frame_num);
			printf("\tframe_num=%d\n", Parts.frame_num);
			for (j = 1; j <= Parts.frame_num; j++)
			{
				frame_t Frame;
				fread(&Frame, sizeof(frame_t), 1, src);
				fprintf(ini, "unk1_%d=0x%X\n;\tunk1_%d=%f\nunk2_%d=0x%X\n;\tunk2_%d=%f\n", j, Frame.unk1, j, *((float*)&Frame.unk1), j, Frame.unk2, j, *((float*)&Frame.unk2));
				fprintf(ini, "frameWidth_%d=0x%X\n;\tframeWidth_%d=%f\nframeHeight_%d=0x%X\n;\tframeHeight_%d=%f\n", j, Frame.frameWidth, j, *((float*)&Frame.frameWidth), j, Frame.frameHeight, j, *((float*)&Frame.frameHeight));
				fprintf(ini, "frameXScale_%d=0x%X\n;\tframeXScale_%d=%f|start:%f\nframeYScale_%d=0x%X\n;\tframeYScale_%d=%f|start:%f\n", j, Frame.frameXScale, j, *((float*)&Frame.frameXScale), *((float*)&Frame.frameXScale) * Texture.width, j, Frame.frameYScale, j, *((float*)&Frame.frameYScale), *((float*)&Frame.frameYScale) * Texture.height);
				fprintf(ini, "frameWidthScale_%d=0x%X\n;\tframeWidthScale_%d=%f|start:%f\nframeHeightScale_%d=0x%X\n;\tframeHeightScale_%d=%f|start:%f\n\n", j, Frame.frameWidthScale, j, *((float*)&Frame.frameWidthScale), *((float*)&Frame.frameWidthScale) * Texture.width, j, Frame.frameHeightScale, j, *((float*)&Frame.frameHeightScale), *((float*)&Frame.frameHeightScale) * Texture.height);
			}
			fseek(src, ftell(src) + 7 & ~7, SEEK_SET);
			anime_t Anime;
			fread(Anime.magic, 20, 1, src);
			if (strncmp(Anime.magic, "Anime   ", 8) != 0)
			{
				printf("不支持的文件类型，请检查是否有Anime块\n");
				system("pause");
				exit(0);
			}
			fread(&Anime.length, 4, 1, src);
			fread(&Anime.info_num, 4, 1, src);
			fputs("[Anime]\n", ini);
			fprintf(ini, "info_num=%d\n\n", Anime.info_num);
			if (Anime.info_num != 0)
			{
				printf("Anime块有数据！info_num=%d\n", Anime.info_num);
				system("pause");
			}
		}
		fclose(ini);
		fclose(src);
	}
}

int main(int argc, char* argv[])
{
	printf("project：Helheim-DATE A LIVE 莲Dystopia\n用于解包tex文件，将tex文件放进文件夹，再拖文件夹到程序上\nby Darkness-TX 2022.07.12\n\n");
	process_dir(argv[1]);
	Unpack(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}