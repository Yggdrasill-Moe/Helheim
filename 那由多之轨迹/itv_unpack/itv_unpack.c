/*
用于解包falcom的itv文件
made by Darkness-TX
2019.10.10
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

struct itv
{
	char magic[4];
	unit16 numOfFrames;
	unit16 numOfColors;
	unit32 blockXSize;
	unit32 blockYSize;
	unit16 width;
	unit16 height;
	unit16 importantColors;
	//额外自制
	unit32 numOfXBlocks;
	unit32 numOfYBlocks;
	unit32 numOfBlocks;
};

unit32 process_dir(char *dname)
{
	long Handle;
	unit32 i = 0;
	struct _finddata64i32_t FileInfo;
	_chdir(dname);//跳转路径
	if ((Handle = _findfirst("*.itv", &FileInfo)) == -1L)
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

//有bug，导出有问题，应该是哪里搞错了，
//但大部分代码是没有问题的，所以此工具仅供参考，bug暂时没空去解决
void UnpackITV(char *fname)
{
	FILE *src = NULL, *dst = NULL;
	unit32 i = 0, k = 0, frame = 0;
	png_colorp pcolor = NULL;
	unit8 *png_alpha = NULL, *data = NULL, dstname[MAX_PATH];
	for (i = 0; i < FileNum; i++)
	{
		src = fopen(Index[i].name, "rb");
		struct itv ITV;
		fread(ITV.magic, 4, 1, src);
		if (strncmp(ITV.magic, "MMV3", 4))
		{
			printf("%s的文件头不是MMV3！\n", Index[i].name);
			fclose(src);
			system("pause");
			continue;
		}
		fseek(src, 4, SEEK_CUR);
		fread(&ITV.numOfFrames, 2, 1, src);
		fseek(src, 8, SEEK_CUR);
		fread(&ITV.numOfColors, 2, 1, src);
		fread(&ITV.blockXSize, 1, 1, src);
		ITV.blockXSize = 1 << ITV.blockXSize;
		fread(&ITV.blockYSize, 1, 1, src);
		ITV.blockYSize = 1 << ITV.blockYSize;
		fread(&ITV.width, 2, 1, src);
		fread(&ITV.height, 2, 1, src);
		fseek(src, 2, SEEK_CUR);
		fread(&ITV.importantColors, 2, 1, src);
		if (ITV.numOfColors != 256)
		{
			printf("%s的调色板不是256色！\n", Index[i].name);
			fclose(src);
			system("pause");
			continue;
		}
		ITV.numOfXBlocks = (ITV.width + ITV.blockXSize - 1) / ITV.blockXSize;
		ITV.numOfYBlocks = (ITV.height + ITV.blockYSize - 1) / ITV.blockYSize;
		ITV.numOfBlocks = ITV.numOfXBlocks * ITV.numOfYBlocks;
		printf("%s frames:%d colors:%d width:%d height:%d block_x:%d block_y:%d block_x_num:%d block_y_num:%d block_num:%d\n", Index[i].name, ITV.numOfFrames, ITV.numOfColors, ITV.width, ITV.height, ITV.blockXSize, ITV.blockYSize, ITV.numOfXBlocks, ITV.numOfYBlocks, ITV.numOfBlocks);
		pcolor = (png_colorp)malloc(ITV.numOfColors * sizeof(png_color));
		png_alpha = (unit8 *)malloc(ITV.numOfColors * sizeof(unit8));
		data = malloc(ITV.numOfColors * 4);
		fread(data, ITV.numOfColors * 4, 1, src);
		for (k = 0; k < ITV.numOfColors; k++)
		{
			pcolor[k].red = data[k * 4 + 0];
			pcolor[k].green = data[k * 4 + 1];
			pcolor[k].blue = data[k * 4 + 2];
			png_alpha[k] = data[k * 4 + 3];
		}
		free(data);
		unit8 bloxels[512][0x3F * 4][2][2] = { 0 };//numOfBlocks
		sprintf(Index[i].name, "%s_unpack", Index[i].name);
		_mkdir(Index[i].name);
		_chdir(Index[i].name);
		data = malloc(ITV.width * ITV.height);
		memset(data, 0, ITV.width * ITV.height);
		for (frame = 0; frame < ITV.numOfFrames; frame++)
		{
			for (int yBlock = 0; yBlock < ITV.numOfYBlocks; yBlock++)
			{
				int block_y_min = yBlock * ITV.blockYSize;
				int block_y_max = (yBlock + 1) * ITV.blockYSize;
				if (block_y_max > ITV.height)
					block_y_max = ITV.height;
				for (int xBlock = 0; xBlock < ITV.numOfXBlocks; xBlock++)
				{
					int s6 = 1;
					if (frame != 0)
					{
						if (frame < 0x4)
							s6 = 0;
					}
					else
					{
						s6 = ITV.numOfFrames;
						if (4 < s6)
							s6 = 4;
					}
					int block = ITV.numOfXBlocks * yBlock + xBlock;
					int s4 = 0;
					while (s4 < s6)
					{//暂时没有明白s6是啥
						int numOfBloxels = 0;
						fread(&numOfBloxels, 1, 1, src);
						for (int l = 0; l < numOfBloxels; l++)
						{
							for (int j = 0; j < 2; j++)
							{
								for (int k = 0; k < 2; k++)
									fread(&bloxels[block][(frame % 4) * 0x3F + l][j][k], 1, 1, src);
							}
						}
						s4++;
					}
					int block_x_min = xBlock * ITV.blockXSize;
					int block_x_max = (xBlock + 1) * ITV.blockXSize;
					if (block_x_max > ITV.width)
						block_x_max = ITV.width;
					int y_start = block_y_min;
					while (y_start < block_y_max)
					{
						int block_area = (block_x_max - block_x_min) * (block_y_max - block_y_min);
						block_area = ((block_area >> 0x1F) >> 0x1E) + block_area;  

						int x_start = block_x_min;
						unit8 lastByteRead = 0;
						while (x_start < block_x_max)
						{
							bool pixelCopy = false;
							unit32 currentByte = 0;
							fread(&currentByte, 1, 1, src);
							fseek(src, -1, SEEK_CUR);

							int amountToWrite = 0;
							if (currentByte != 0xFF)
							{
								if (currentByte != 0xFE)
								{
									pixelCopy = true;
									amountToWrite = 1;
								}
								else
								{
									fseek(src, 1, SEEK_CUR);
									fread(&amountToWrite, 1, 1, src);
									amountToWrite += 1;
									fseek(src, -1, SEEK_CUR);
								}
							}
							else
							{
								if (!(x_start == block_x_min && y_start == block_y_min))
								{
									currentByte = lastByteRead;
									fseek(src, 1, SEEK_CUR);
									fread(&amountToWrite, 1, 1, src);
									fseek(src, -1, SEEK_CUR);
									pixelCopy = true;
								}
								else
								{
									fseek(src, 1, SEEK_CUR);
									fread(&currentByte, 1, 1, src);
									fseek(src, -1, SEEK_CUR);
									//FIXED ME:这里有问题，不知道为什么要绘制这么大的区域
									//pixelCopy = currentByte != 0xFE
									amountToWrite = block_area >> 0x2;
								}
							}
							while (amountToWrite >= 0)
							{
								if (pixelCopy)
								{
									int bw = 2;
									int bh = 2;
									if ((block_y_max - y_start) == 1)
										bh = 1;
									if ((block_x_max - x_start) == 1)
										bw = 1;

									for (int bloxelY = 0; bloxelY < bh; bloxelY++)
									{
										for (int bloxelX = 0; bloxelX < bw; bloxelX++)
										{
											int y = bloxelY + y_start;
											int x = bloxelX + x_start;
											data[y * ITV.width + x] = bloxels[block][currentByte][bloxelY][bloxelX];	
										}
									}
								}
								amountToWrite = amountToWrite - 1;
								if (amountToWrite > 0)
								{
									x_start = x_start + 0x2;
									if (x_start >= block_x_max)
									{
										x_start = block_x_min;
										y_start = y_start + 2;
									}
								}
							}
							fread(&lastByteRead, 1, 1, src);
							x_start = x_start + 2;
						}
						y_start = y_start + 2;
					}
				}
			}
			sprintf(dstname, "%03d.png", frame);
			dst = fopen(dstname, "wb");
			//fwrite(data, ITV.width*ITV.height, 1, dst);
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
			png_set_IHDR(png_ptr, info_ptr, ITV.width, ITV.height, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			png_set_PLTE(png_ptr, info_ptr, pcolor, ITV.numOfColors);
			png_set_tRNS(png_ptr, info_ptr, (png_bytep)png_alpha, ITV.numOfColors, (png_color_16p)0);
			png_write_info(png_ptr, info_ptr);
			for (unit32 l = 0; l < ITV.height; l++)
				png_write_row(png_ptr, data + l * ITV.width);
			png_write_end(png_ptr, info_ptr);
			png_destroy_write_struct(&png_ptr, &info_ptr);
			fclose(dst);
		}
		free(data);
		free(pcolor);
		free(png_alpha);
		_chdir("..");
		fclose(src);
	}
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project：Helheim-那由多之轨迹\n用于解包falcom的itv文件。\n将文件夹拖到程序上。\nby Darkness-TX 2019.10.10\n\n");
	process_dir(argv[1]);
	UnpackITV(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}