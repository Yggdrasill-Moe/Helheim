#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <zlib.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;//总文件数，初始计数为0

struct findex
{
	char FileName[260];//文件名
	unit32 FileNameOff;//文件名位置
	unit32 FileOffset;//文件偏移
}FIndex[300];

/* data 原数据 ndata 原数据长度 zdata 压缩后数据 nzdata 压缩后长度 */
int dacompress(Bytef *zdata, uLong *nzdata, Bytef *data, uLong ndata)
{
	z_stream c_stream;
	int err = 0;

	if (data && ndata > 0) {
		c_stream.zalloc = NULL;
		c_stream.zfree = NULL;
		c_stream.opaque = NULL;
		if (deflateInit2(&c_stream, Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) return -1;
		c_stream.next_in = data;
		c_stream.avail_in = ndata;
		c_stream.next_out = zdata;
		c_stream.avail_out = *nzdata;
		while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata) {
			if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK) return -1;
		}
		if (c_stream.avail_in != 0) return c_stream.avail_in;
		for (;;) {
			if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
			if (err != Z_OK) return -1;
		}
		if (deflateEnd(&c_stream) != Z_OK) return -1;
		*nzdata = c_stream.total_out;
		return 0;
	}
	return -1;
}

void compression(char *dname)
{
	FILE *OpenFile, *WriteFile, *PackFile;
	unit8 *buf, *ubuf, filename[260], newfilename[260], *data, zero = '\0';
	unit32 hflag, filen, noffset, foffset, j, fsize;
	unit16 flag;
	uLong flen, ulen;
		fopen_s(&OpenFile, dname, "rb");
		if (OpenFile == NULL)
		{
			printf("打开文件失败\n");
			system("pause");
			exit(0);
		}
		else
		{
			fread(&hflag, 4, 1, OpenFile);
			if (hflag == 0x200)
			{
				fseek(OpenFile, 0x20, SEEK_SET);
				fread(&filen, 4, 1, OpenFile);//文件数，之后为文件名偏移区起始位置，文件偏移区起始位置，都要加0x20
				fread(&noffset, 4, 1, OpenFile);
				noffset += 0x20;
				fread(&foffset, 4, 1, OpenFile);
				foffset += 0x20;
				printf("%s flag:0x%X nameoffset:0x%X fileoffset:0x%X\n", dname, hflag, noffset, foffset);
				fseek(OpenFile, noffset, SEEK_SET);
				for (j = 0; j < filen; j++)
				{
					fread(&FIndex[j].FileNameOff, 4, 1, OpenFile);
					FIndex[j].FileNameOff += 0x20;
				}
				fseek(OpenFile, foffset, SEEK_SET);
				for (j = 0; j < filen; j++)
				{
					fread(&FIndex[j].FileOffset, 4, 1, OpenFile);
					FIndex[j].FileOffset += 0x20;
				}
				for (j = 0; j < filen; j++)
				{
					fseek(OpenFile, FIndex[j].FileNameOff, SEEK_SET);
					fread(&FIndex[j].FileName, 16, 1, OpenFile);
					if (FIndex[j].FileName[15] != '\0')
					{
						fseek(OpenFile, -0x10, SEEK_CUR);
						fread(&FIndex[j].FileName, 32, 1, OpenFile);
						if (FIndex[j].FileName[31] != '\0')
						{
							fseek(OpenFile, -0x20, SEEK_CUR);
							fread(&FIndex[j].FileName, 48, 1, OpenFile);
						}
					}
				}
				sprintf(newfilename, "%s_new", dname);
				fseek(OpenFile, 0, SEEK_SET);
				data = malloc(FIndex[0].FileOffset);
				fread(data, FIndex[0].FileOffset, 1, OpenFile);
				fopen_s(&WriteFile, newfilename, "wb");
				fwrite(data, FIndex[0].FileOffset, 1, WriteFile);
				free(data);
				fseek(OpenFile, FIndex[0].FileOffset, SEEK_SET);
				sprintf(filename, "%s_unpack", dname);
				_chdir(filename);
				for (j = 0; j < filen; j++)
				{
					fseek(OpenFile, FIndex[j].FileOffset, SEEK_SET);
					FIndex[j].FileOffset = ftell(WriteFile) - 0x20;
					fread(&hflag, 4, 1, OpenFile);
					if (hflag == 0x10300)
					{
						fwrite(&hflag, 4, 1, WriteFile);
						fread(&flen, 4, 1, OpenFile);
						fread(&ulen, 4, 1, OpenFile);
						fseek(OpenFile, 20 + flen, SEEK_CUR);
						sprintf(FIndex[j].FileName, "%s.new", FIndex[j].FileName);
						fopen_s(&PackFile, FIndex[j].FileName, "rb");
						flen = compressBound(ulen);
						buf = malloc(flen);
						ubuf = malloc(ulen);
						fread(ubuf, ulen, 1, PackFile);
						dacompress(buf, &flen, ubuf, ulen);
						fwrite(&flen, 4, 1, WriteFile);
						fwrite(&ulen, 4, 1, WriteFile);
						fseek(WriteFile, 20, SEEK_CUR);
						fwrite(buf, flen, 1, WriteFile);
						free(buf);
						free(ubuf);
						fclose(PackFile);
						printf("\t%s flag:0x%X 文件大小：0x%X 解压大小：0x%X\n", FIndex[j].FileName, hflag, flen + 0x20, ulen);
					}
					else
					{
						printf("\t%s flag:0x%X\n", FIndex[j].FileName, hflag);
						system("pause");
					}
					if (ftell(WriteFile) % 16 != 0)
					{
						fseek(WriteFile, (ftell(WriteFile) / 16 + 1) * 16 - 1, SEEK_SET);
						fwrite(&zero, 1, 1, WriteFile);
					}
				}
				fsize = ftell(WriteFile) - 0x20;
				fseek(WriteFile, 4, SEEK_SET);
				fwrite(&fsize, 4, 1, WriteFile);
				fseek(WriteFile, foffset, SEEK_SET);
				for (j = 0; j < filen; j++)
					fwrite(&FIndex[j].FileOffset, 4, 1, WriteFile);
				fclose(WriteFile);
				_chdir("..");
			}
			else// if (hflag == 0x10300 || hflag == 0x10600)
			{
				fread(&flen, 4, 1, OpenFile);
				fread(&ulen, 4, 1, OpenFile);
				fseek(OpenFile, 0x20, SEEK_SET);
				fread(&flag, 2, 1, OpenFile);
				printf("%X", flag);
				system("pause");
				if (flag == 0xDA78)
				{
					fseek(OpenFile, 0x20, SEEK_SET);
					fopen_s(&WriteFile, strcat(dname, ".dec"), "wb");
					buf = malloc(flen);
					ubuf = malloc(ulen);
					fread(buf, flen, 1, OpenFile);
					uncompress(ubuf, &ulen, buf, flen);
					fwrite(ubuf, ulen, 1, WriteFile);
					free(buf);
					free(ubuf);
					fclose(WriteFile);
					printf("%s 文件大小：0x%X 解压大小：0x%X\n", dname, flen, ulen);
				}
				else
					printf("%s 文件大小：0x%X\n", dname, flen);
			}
		}
		fclose(OpenFile);
}

int main(int argc, char *argv[])
{
	printf("project：Helheim-3DS WE 2014\n用于压缩或打包zlib压缩的文件，将文件夹拖到程序上，\n已挂载可处理一万二千个文件的缓存区，文件夹中的文件请不要多余一万两千。\nby Darkness-TX 2016.7.8\n");
	compression(argv[1]);
	printf("已完成，总文件数%d\n", FileNum);
	system("pause");
	return 0;
}