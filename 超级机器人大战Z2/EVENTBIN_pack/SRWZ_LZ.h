#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

typedef struct info_s
{
	unit32 buffsize;
	unit32 distance;
	struct info_s *next;
}Nodeinfo, *Linkinfo;

//sub_88292CC
unit32 get_start_offset(unit8* data)
{
	unit8* offset = data, buff = 0;
	int size = 0, buff_size = 0, buff2 = 0;
	do
	{
		buff = *offset;
		size <<= 7;
		size |= buff;
		offset++;
	} while ((size & 1) == 0);
	size >>= 1;
	if (size <= 0)
	{
		printf("解压大小错误！size:0x%X\nsceKernelDcacheWritebackInvalidateAll()\n", size);
		system("pause");
		return -1;
	}
	do
	{
		buff = *offset;
		buff2 <<= 7;
		buff2 |= buff;
		offset++;
	} while ((buff2 & 1) == 0);
	buff2 >>= 1;
	buff_size = ((buff2 >> 1) & 0x0F) + 8;
	buff_size = 1 << buff_size;
	//buff_size以0x100为最低大小，所以上面的计算要+8，具体实现看make_buff_size
	if (buff_size >= size)
	{
		if ((buff2 & 0x21) != 1)
		{
			if (buff2 & 0x40)
			{
				//但是buff2后面并不会用到，不知道这段有啥意义
				//当然本身(buff2 & 0x21) != 1就没出现过
				buff2 = 0;
				do
				{
					buff = *offset;
					buff2 <<= 7;
					buff2 |= buff;
					offset++;
				} while ((buff2 & 1) == 0);
			}
		}
	}
	else
	{
		if (buff2 & 0x40)
		{
			buff2 = 0;
			do
			{
				buff = *offset;
				buff2 <<= 7;
				buff2 |= buff;
				offset++;
			} while ((buff2 & 1) == 0);
		}
	}
	//对，就是这样，这里直接清零刚刚的计算了，只能当成是塞垃圾数据了，
	//当然更可能的是这算法是另一种算法的简化版本，在这算法里弃用了上面的运算结果，
	//下面这轮运算是必须的，但文件中是直接读取0x01强制结束掉，果然就是简化版本吧？
	buff2 = 0;
	do
	{
		buff = *offset;
		buff2 <<= 7;
		buff2 |= buff;
		offset++;
	} while ((buff2 & 1) == 0);
	return offset - data;
	/*
	unit8* a0 = data;
	unit32 v0 = 0;
	unit32 v1 = 0;
	do
	{
		v0 = *a0;
		v1 <<= 7;
		v1 |= v0;
		a0 += 1;
	} while ((v1 & 1) == 0);
	unit32 s0 = v1 >> 1;
	if (s0 <= 0)
	{
		printf("sceKernelDcacheWritebackInvalidateAll\n");
		system("pause");
		return -1;
	}
	v1 = 0;
	do
	{
		v0 = *a0;
		v1 <<= 7;
		v1 |= v0;
		a0 += 1;
	} while ((v1 & 1) == 0);
	unit32 a1 = v1 >> 1;
	//ext v0,a1,0x1,0x4
	//addiu v0,v0,0x8
	v0 = ((a1 >> 1) & 0x0F) + 8;
	v1 = 1;
	v0 = v1 << v0;
	if (v0 >= s0)
	{
		v0 = a1 & 0x21;
		if (v0 != v1)
		{
			v0 = a1 & 0x40;
			if (v0)
			{
				v1 = 0;
				do
				{
					v0 = *a0;
					v1 <<= 7;
					v1 |= v0;
					a0 += 1;
				} while ((v1 & 1) == 0);
			}
		}
	}
	else
	{
		v0 = a1 & 0x40;
		if (v0)
		{
			v1 = 0;
			do
			{
				v0 = *a0;
				v1 <<= 7;
				v1 |= v0;
				a0 += 1;
			} while ((v1 & 1) == 0);
		}
	}
	v1 = 0;
	do
	{
		v0 = *a0;
		v1 <<= 7;
		v1 |= v0;
		a0 += 1;
	} while ((v1 & 1) == 0);
	return a0 - data;
	*/
}

unit32 get_size(unit8* data)
{
	unit8 *offset = data, buff = 0;
	int size = 0;
	do
	{
		buff = *offset;
		size <<= 7;
		size |= buff;
		offset++;
	} while ((size & 1) == 0);
	return size >> 1;
	/*
	unit8* a0 = data;
	unit32 v0 = 0;
	unit32 v1 = 0;
	do
	{
		v0 = *a0;
		v1 <<= 7;
		v1 |= v0;
		a0 += 1;
	} while ((v1 & 1) == 0);
	unit32 s0 = v1 >> 1;
	return s0;
	*/
}

//sub_88294BC
unit32 LZdecompress(unit8* cdata, unit8* udata, unit32 size)
{
	unit8 *src = cdata, *dst = udata, *window = udata;
	unit8 buff = 0, buff2 = 0, sign = 0;
	int buffsize = 0, round = 0, distance = 0;
	do
	{
		//创建数据
		sign = *src;
		src++;
		buffsize = sign & 0x0F;
		round = sign >> 4;
		if (buffsize == 0)//处理低位，表示初始数据的buffsize
		{
			do
			{
				buff = *src;
				buffsize <<= 7;
				buffsize |= buff;
				src++;
			} while ((buffsize & 1) == 0);
			buffsize >>= 1;
		}
		if (round == 0)//处理高位，表示后续解压的回数
		{
			do
			{
				buff = *src;
				round <<= 7;
				round |= buff;
				src++;
			} while ((round & 1) == 0);
			round >>= 1;
		}
		do
		{
			*dst = *src;
			buffsize--;
			src++;
			dst++;
		} while (buffsize != 0);
		if ((unit32)(dst - udata) >= size)
			return 0;
		//开始处理
		do
		{
			sign = *src;
			src++;
			distance = sign & 0x0F;
			buffsize = sign >> 4;
			//解压就是经典LZ压缩的（前向距离,长度），但滑动窗口大小似乎没有限制，前向距离可以非常大
			if ((distance & 1) == 0)//处理低位
			{
				do
				{
					buff = *src;
					distance <<= 7;
					distance |= buff;
					src++;
				} while ((distance & 1) == 0);
			}
			distance >>= 1;
			if (buffsize == 0)//处理高位
			{
				do
				{
					buff = *src;
					buffsize <<= 7;
					buffsize |= buff;
					src++;
				} while ((buffsize & 1) == 0);
				buffsize >>= 1;
			}
			buffsize++;
			window = dst - distance - 1;
			if (window < udata)
				return 1;
			if ((unit32)(dst + buffsize - udata) >= size)
				buffsize = udata + size - dst;
			do
			{
				*dst = *window;
				window++;
				dst++;
				buffsize--;
			} while (buffsize != 0);
			round--;
		} while (round != 0);
	} while ((unit32)(dst - udata) < size);
	return 0;
	/*
	unit8 *a0 = cdata;
	unit8 *a2 = udata;
	unit32 t0 = 0;
	unit32 t1 = 0;
	unit32 a3 = 0;
	unit32 a1 = 0;
	unit32 t2 = 0;
	while (1)
	{
		t0 = *a0;
		t1 = t0 & 0x0F;
		a0 += 1;
		if (t1 == 0)
		{
			a3 = 0;
			do
			{
				a1 = *a0;
				a3 <<= 7;
				a3 |= a1;
				a0 += 1;
			} while ((a3 & 1) == 0);
			t1 = a3 >> 1;
		}
		t2 = t0 >> 4;
		if (t2 == 0)
		{
			a3 = 0;
			do
			{
				a1 = *a0;
				a3 <<= 7;
				a3 |= a1;
				a0 += 1;
			} while ((a3 & 1) == 0);
			t2 = a3 >> 1;
		}
		do
		{
			a1 = *a0;
			t1--;
			*a2 = a1 & 0xFF;
			a0 += 1;
			a2 += 1;
		} while (t1 != 0);
		if ((unit32)(a2 - udata) >= size)
			return 0;
		while (1)
		{
			t1 = *a0;
			a3 = t1 & 0x0F;
			a0 += 1;
			if ((a3 & 1) == 0)
			{
				do
				{
					a1 = *a0;
					a3 <<= 7;
					a3 |= a1;
					a0 += 1;
				} while ((a3 & 1) == 0);
			}
			a1 = a3 >> 1;
			a3 = t1 >> 4;
			t0 = ~a1;
			if (a3 == 0)
			{
				do
				{
					a1 = *a0;
					a3 <<= 7;
					a3 |= a1;
					a0 += 1;
				} while ((a3 & 1) == 0);
				a3 >>= 1;
			}
			a3 = a3 + 1;
			a1 = (unit32)(a2 + t0);
			if ((unit8 *)a1 < udata)
				return 1;
			if ((unit32)(a2 + a3 - udata) >= size)
			{
				a3 = udata + size - a2;
				a1 = (unit32)(a2 + t0);
			}
			do
			{
				a1 = (unit32)(a2 + t0);
				a1 = *(unit8*)a1;
				a3 = a3 - 1;
				*a2 = a1 & 0xFF;
				a2 += 1;
			} while (a3 != 0);
			t2 = t2 - 1;
			if (t2 == 0)
				break;
		}
		if ((unit32)(a2 - udata) >= size)
			break;
	}
	return 0;
	*/
}

unit32 LZdecompressmod(unit8* cdata, unit8* udata, unit32 size)
{
	unit8* src = cdata, * dst = udata, * window = udata;
	unit8 buff = 0, buff2 = 0, sign = 0;
	int buffsize = 0, round = 0, distance = 0;
	do
	{
		//创建数据
		sign = *src;
		src++;
		buffsize = sign & 0x0F;
		round = sign >> 4;
		if (buffsize == 0)//处理低位，表示初始数据的buffsize
		{
			do
			{
				buff = *src;
				buffsize <<= 7;
				buffsize |= buff;
				src++;
			} while ((buffsize & 1) == 0);
			buffsize >>= 1;
		}
		if (round == 0)//处理高位，表示后续解压的回数
		{
			do
			{
				buff = *src;
				round <<= 7;
				round |= buff;
				src++;
			} while ((round & 1) == 0);
			round >>= 1;
		}
		do
		{
			*dst = *src;
			buffsize--;
			src++;
			dst++;
		} while (buffsize != 0);
		if ((unit32)(dst - udata) >= size)
			return src - cdata;
		//开始处理
		do
		{
			sign = *src;
			src++;
			distance = sign & 0x0F;
			buffsize = sign >> 4;
			//解压就是经典LZ压缩的（前向距离,长度），但滑动窗口大小似乎没有限制，前向距离可以非常大
			if ((distance & 1) == 0)//处理低位
			{
				do
				{
					buff = *src;
					distance <<= 7;
					distance |= buff;
					src++;
				} while ((distance & 1) == 0);
			}
			distance >>= 1;
			if (buffsize == 0)//处理高位
			{
				do
				{
					buff = *src;
					buffsize <<= 7;
					buffsize |= buff;
					src++;
				} while ((buffsize & 1) == 0);
				buffsize >>= 1;
			}
			buffsize++;
			window = dst - distance - 1;
			if (window < udata)
				return src - cdata;
			if ((unit32)(dst + buffsize - udata) >= size)
				buffsize = udata + size - dst;
			do
			{
				*dst = *window;
				window++;
				dst++;
				buffsize--;
			} while (buffsize != 0);
			round--;
		} while (round != 0);
	} while ((unit32)(dst - udata) < size);
	return src - cdata;
}

unit32 make_size(unit8* data, unit32 size)
{
	unit32 count = 0, i = 3;
	memset(data, 0, 4);
	//解压后的文件都是有填充到0x10对齐的，所以size最后一位必然为0
	size <<= 1;
	size |= 1;
	while (size & 0xFFFFFFFF)
	{
		data[i--] = size & 0xFF;
		count++;
		size >>= 8;
		size <<= 1;
	}
	return count;
}

unit32 make_buff_size(unit8* data, unit32 size)
{
	unit32 base = 0x100, count = 0, buff = 0;
	while (base < size)
	{
		base <<= 1;
		buff++;
	}
	buff <<= 1;
	buff |= 1;
	count = make_size(data, buff);
	return count;
}

unit32 get_match_size(unit8* data, unit32 buffsize, unit32 distance)
{
	unit32 count = 0, dstsize = 0;
	dstsize++;
	if(distance > 0x07)
	{
		count = make_size(data, distance);
		if (data[4 - count] >> 4 == 0)
			dstsize += count - 1;
		else
			dstsize += count;
	}
	if (buffsize > 0x0F)
	{
		count = make_size(data, buffsize);
		dstsize += count;
	}
	return dstsize;
}

//兜底用
unit32 FakeLZcompress(unit8* cdata, unit8* udata, unit32 size)
{
	unit8* src = udata, * dst = cdata, sizebuff[4];
	unit32 count = 0, i = 0;
	count = make_size(sizebuff, size);
	*dst = 0x00;
	dst++;
	for (i = 4 - count; i < 4; i++)
	{
		*dst = sizebuff[i];
		dst++;
	}
	*dst = 0x01;
	dst++;
	for (i = 0; i < size; i++)
	{
		*dst = *src;
		dst++;
		src++;
	}
	return dst - cdata;
}

unit32 LZcompress(unit8* cdata, unit8* udata, unit32 size)
{
	/*
	对比常规LZ压缩的不同处：
	1.前向距离没限制，只要能索引到都可以
	2.最低匹配字节数甚至可以降到1字节，但还好没用这机制，最低匹配是2字节
	3.(长度,前向距离)二元组最低占1字节，可动态扩展，所以最低匹配才设置成2字节
	4.增加了轮数(round)
	5.在当前已完成的（未匹配字节，已匹配字节）的流程下再增加未匹配字节数量直到出现下一次匹配数据，比较两者处理的数据量选更优解，
	达到选择了“局部”更优解的目的，动态规划的思想。

	因为以上不同，可以一直往前匹配到缓冲区起始，所以压缩效率上应该会大幅降低，
	但直接用常规LZ压缩的方式也是兼容的，二元组表示方式改一改就能用。
	这里尽量还原原算法。
	
	可优化的地方：
	查找算法属于暴力查找，可以引用下其他LZ系算法用到的哈希桶方式，MIN_MATCH为2，那就起始创建int hash[0x10000]的数组，后面再做成哈希桶方式，
	边查找边添加匹配长度等于2的offset到哈希桶中，查找时先读取哈希桶判断有没有匹配长度等于2的，如果有再在这个offset下向后继续匹配，如果没有则代表max_match < MIN_MATCH，
	需要注意LZ系压缩的特点是长度+offset可以超过当前位置，哪怕哈希桶中没有，可能当前位置-1却正好能匹配2字节，加入哈希桶或者查找时任选一个步骤来注意处理这种情况。
	
	由于上面的第五条未实现，所以压缩率在某些情况下上比原算法差一些，但是在压缩一些大文件的时候会比原压缩算法压缩率上要高，
	重点还是在第五条，这个选择方式个人认为并不好，选择“局部”更优这个“局部”不好定义，原算法判断的范围太小，选择“局部”更优后面很可能跟
	着一个“局部”更差，这样最终有可能会比直接选择“当前”最优解更差，这也是为什么在压缩一些大文件的时候会比原压缩算法压缩率上要高。
	本算法属于直接选择“当前”最优情况。
	*/
	//src:待压缩数据 dst:压缩数据 window:前向查找窗口 sizebuff:计算用缓存
	unit8 *src = udata, *dst = cdata, *window = udata, *no_match_offset = NULL, sizebuff[4];
	//dstsize:压缩数据长度 count:匹配长度 addr:地址偏移量 max_match:最大匹配长度 distance:前向距离 no_match:未匹配字节长度 round:匹配成功轮数
	unit32 dstsize = 0, count = 0, addr = 0, max_match = 0, distance = 0, no_match = 0, round = 0, i = 0;
	const unit32 MIN_MATCH = 2;//最小匹配数
	//info:(长度,前向距离)二元组链表，表中元素个数即为round
	Linkinfo info = malloc(sizeof(Nodeinfo)), q = info;
	info->next = NULL;
	while ((unit32)(src - udata) < size)
	{
		window = src - 1;//前向匹配从当前位置的前一个字节开始
		max_match = 0;
		distance = src - window;
		//最后小于MIN_MATCH的字节不需要匹配，排除后可以省略最后一轮的前向匹配几乎整个文件大小的查找过程，算是一个简单的优化
		if (size - (unit32)(src - udata) >= MIN_MATCH)
		{
			//逆向查找最大适合压缩的匹配串算法，加上可怕的无限制前向距离，性能大幅下降的来源……
			while (window >= udata)
			{
				addr = 0;
				count = 0;
				//要先判断是否超过范围，再判断是否匹配，这个小细节之前被忽略了，步骤反了过来导致先越界取值(*(src + addr))程序直接崩溃了
				while (((unit32)(src + addr - udata) < size) && (*(window + addr) == *(src + addr)))
				{
					count++;
					addr++;
				}
				//前向距离越接近匹配起始地址越好，距离越小生成的距离表示字节数越短，所以不能等于
				//同时，如果get_match_size()后大于等于当前匹配长度，那这次匹配数据反而不压缩更优，这也是为什么前向距离越接近匹配起始地址越好
				if (count > max_match && get_match_size(sizebuff, count - 1, src - window - 1) < count)
				{
					max_match = count;
					distance = src - window;
				}
				//如果这轮匹配后正好处理完所有原始数据，就没必要再继续前向匹配了，算是一个简单的优化
				if ((unit32)(src + max_match - udata) == size)
					break;
				window--;
			}
		}
		//第一个字节肯定是不匹配的，所以首次必然进max_match < MIN_MATCH分支
		if (max_match < MIN_MATCH)
		{
			//本次匹配小于MIN_MATCH，但却有已匹配达到MIN_MATCH的数据，代表本次开始是新的一轮，则输出前一轮的未匹配与匹配数据
			if (info->next != NULL)
			{
				round = 0;
				q = info;
				while (q->next != NULL)
				{
					round++;
					q = q->next;
				}
				//写(轮数,未匹配数据长度)
				if (round <= 0x0F && no_match <= 0x0F)
				{
					*dst = (round << 4) | no_match;
					dst++;
					dstsize++;
				}
				else if (round <= 0x0F && no_match > 0x0F)
				{
					*dst = round << 4;
					dst++;
					dstsize++;
					count = make_size(sizebuff, no_match);
					for (i = 4 - count; i < 4; i++)
					{
						*dst = sizebuff[i];
						dst++;
						dstsize++;
					}
				}
				else if (round > 0x0F && no_match <= 0x0F)
				{
					*dst = no_match;
					dst++;
					dstsize++;
					count = make_size(sizebuff, round);
					for (i = 4 - count; i < 4; i++)
					{
						*dst = sizebuff[i];
						dst++;
						dstsize++;
					}
				}
				else if (round > 0x0F && no_match > 0x0F)
				{
					*dst = 0;
					dst++;
					dstsize++;
					count = make_size(sizebuff, no_match);
					for (i = 4 - count; i < 4; i++)
					{
						*dst = sizebuff[i];
						dst++;
						dstsize++;
					}
					count = make_size(sizebuff, round);
					for (i = 4 - count; i < 4; i++)
					{
						*dst = sizebuff[i];
						dst++;
						dstsize++;
					}
				}
				//写未匹配数据
				while (no_match--)
				{
					*dst = *no_match_offset;
					dst++;
					no_match_offset++;
					dstsize++;
				}
				//写匹配数据信息
				q = info;
				while (q->next != NULL)
				{
					Linkinfo tmp = q;
					q = q->next;
					if (tmp != info)
					{
						tmp->next = NULL;
						free(tmp);
					}
					q->buffsize--;
					q->distance--;
					if (q->buffsize <= 0x0F && q->distance <= 0x07)
					{
						*dst = (q->buffsize << 4) | ((q->distance << 1) | 1);
						dst++;
						dstsize++;
					}
					//q->distance > 0x07的情况下，表示q->distance的4位可以全为0，但这样(distance & 1) == 0的情况只用到了全为0一种，
					//所以把另外7种情况也利用起来，真是极致啊，1字节也要省……
					else if (q->buffsize <= 0x0F && q->distance > 0x07)
					{
						count = make_size(sizebuff, q->distance);
						if (sizebuff[4 - count] >> 4 == 0)
						{
							*dst = (q->buffsize << 4) | sizebuff[4 - count];
							dst++;
							dstsize++;
							for (i = 4 - count + 1; i < 4; i++)
							{
								*dst = sizebuff[i];
								dst++;
								dstsize++;
							}
						}
						else
						{
							*dst = q->buffsize << 4;
							dst++;
							dstsize++;
							for (i = 4 - count; i < 4; i++)
							{
								*dst = sizebuff[i];
								dst++;
								dstsize++;
							}
						}
					}
					else if (q->buffsize > 0x0F && q->distance <= 0x07)
					{
						*dst = (q->distance << 1) | 1;
						dst++;
						dstsize++;
						count = make_size(sizebuff, q->buffsize);
						for (i = 4 - count; i < 4; i++)
						{
							*dst = sizebuff[i];
							dst++;
							dstsize++;
						}
					}
					else if (q->buffsize > 0x0F && q->distance > 0x07)
					{
						count = make_size(sizebuff, q->distance);
						if (sizebuff[4 - count] >> 4 == 0)
						{
							*dst = sizebuff[4 - count];
							dst++;
							dstsize++;
							for (i = 4 - count + 1; i < 4; i++)
							{
								*dst = sizebuff[i];
								dst++;
								dstsize++;
							}
						}
						else
						{
							*dst = 0;
							dst++;
							dstsize++;
							for (i = 4 - count; i < 4; i++)
							{
								*dst = sizebuff[i];
								dst++;
								dstsize++;
							}
						}
						count = make_size(sizebuff, q->buffsize);
						for (i = 4 - count; i < 4; i++)
						{
							*dst = sizebuff[i];
							dst++;
							dstsize++;
						}
					}
				}
				//重置
				info->next = NULL;
				q = info;
				no_match = 0;
			}
			//本次匹配小于MIN_MATCH，则代表还在收集匹配小于MIN_MATCH的数据中，
			//未匹配还是已匹配但小于MIN_MATCH的都是加1
			no_match++;
			src++;
			no_match_offset = src - no_match;
		}
		else
		{
			Linkinfo p = malloc(sizeof(Nodeinfo));
			p->next = NULL;
			p->buffsize = max_match;
			p->distance = distance;
			q->next = p;
			q = q->next;
			src += max_match;
		}
	}
	//处理最后一次的剩余数据
	round = 0;
	//如果最后一次是正好匹配完，那轮数就不为0
	q = info;
	while (q->next != NULL)
	{
		round++;
		q = q->next;
	}
	//写(轮数,未匹配数据长度)
	if (round <= 0x0F && no_match <= 0x0F)
	{
		*dst = (round << 4) | no_match;
		dst++;
		dstsize++;
	}
	else if (round <= 0x0F && no_match > 0x0F)
	{
		*dst = round << 4;
		dst++;
		dstsize++;
		count = make_size(sizebuff, no_match);
		for (i = 4 - count; i < 4; i++)
		{
			*dst = sizebuff[i];
			dst++;
			dstsize++;
		}
	}
	else if (round > 0x0F && no_match <= 0x0F)
	{
		*dst = no_match;
		dst++;
		dstsize++;
		count = make_size(sizebuff, round);
		for (i = 4 - count; i < 4; i++)
		{
			*dst = sizebuff[i];
			dst++;
			dstsize++;
		}
	}
	else if (round > 0x0F && no_match > 0x0F)
	{
		*dst = 0;
		dst++;
		dstsize++;
		count = make_size(sizebuff, no_match);
		for (i = 4 - count; i < 4; i++)
		{
			*dst = sizebuff[i];
			dst++;
			dstsize++;
		}
		count = make_size(sizebuff, round);
		for (i = 4 - count; i < 4; i++)
		{
			*dst = sizebuff[i];
			dst++;
			dstsize++;
		}
	}
	//最后剩下的数据大部分都是未匹配数据，自然不带匹配数据信息，那轮数必然为0，
	//为0会触发make_size机制，所以需要特别处理
	if (round == 0)
	{
		*dst = 0x01;
		dst++;
		dstsize++;
	}
	//写未匹配数据
	while (no_match--)
	{
		*dst = *no_match_offset;
		dst++;
		no_match_offset++;
		dstsize++;
	}
	//写匹配数据信息
	q = info;
	while (q->next != NULL)
	{
		Linkinfo tmp = q;
		q = q->next;
		if (tmp != info)
		{
			tmp->next = NULL;
			free(tmp);
		}
		q->buffsize--;
		q->distance--;
		if (q->buffsize <= 0x0F && q->distance <= 0x07)
		{
			*dst = (q->buffsize << 4) | ((q->distance << 1) | 1);
			dst++;
			dstsize++;
		}
		//q->distance > 0x07的情况下，表示q->distance的4位可以全为0，但这样(distance & 1) == 0的情况只用到了全为0一种，
		//所以把另外7种情况也利用起来，真是极致啊，1字节也要省……
		else if (q->buffsize <= 0x0F && q->distance > 0x07)
		{
			count = make_size(sizebuff, q->distance);
			if (sizebuff[4 - count] >> 4 == 0)
			{
				*dst = (q->buffsize << 4) | sizebuff[4 - count];
				dst++;
				dstsize++;
				for (i = 4 - count + 1; i < 4; i++)
				{
					*dst = sizebuff[i];
					dst++;
					dstsize++;
				}
			}
			else
			{
				*dst = q->buffsize << 4;
				dst++;
				dstsize++;
				for (i = 4 - count; i < 4; i++)
				{
					*dst = sizebuff[i];
					dst++;
					dstsize++;
				}
			}
		}
		else if (q->buffsize > 0x0F && q->distance <= 0x07)
		{
			*dst = (q->distance << 1) | 1;
			dst++;
			dstsize++;
			count = make_size(sizebuff, q->buffsize);
			for (i = 4 - count; i < 4; i++)
			{
				*dst = sizebuff[i];
				dst++;
				dstsize++;
			}
		}
		else if (q->buffsize > 0x0F && q->distance > 0x07)
		{
			count = make_size(sizebuff, q->distance);
			if (sizebuff[4 - count] >> 4 == 0)
			{
				*dst = sizebuff[4 - count];
				dst++;
				dstsize++;
				for (i = 4 - count + 1; i < 4; i++)
				{
					*dst = sizebuff[i];
					dst++;
					dstsize++;
				}
			}
			else
			{
				*dst = 0;
				dst++;
				dstsize++;
				for (i = 4 - count; i < 4; i++)
				{
					*dst = sizebuff[i];
					dst++;
					dstsize++;
				}
			}
			count = make_size(sizebuff, q->buffsize);
			for (i = 4 - count; i < 4; i++)
			{
				*dst = sizebuff[i];
				dst++;
				dstsize++;
			}
		}
	}
	info->next = NULL;
	return dstsize;
}