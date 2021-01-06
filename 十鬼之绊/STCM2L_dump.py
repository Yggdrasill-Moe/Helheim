# -*- coding:utf-8 -*-
#用于导出STCM2L脚本文件的文本，基于十鬼之绊（psp）
#made by Darkness-TX
#2019.03.05

import os
import sys
import struct
import io

def int2byte(num):
	return struct.pack('L',num)

def byte2int(byte):
	return struct.unpack('L',byte)[0]

def walk(adr):
	mylist=[]
	for root,dirs,files in os.walk(adr):
		for name in files:
			adrlist=os.path.join(root, name)
			mylist.append(adrlist)
	return mylist

#emmm......首先开头是STCM2L然后一串日期，这里占0x20
def CheckHeader(src):
	src.seek(0,os.SEEK_SET)
	if src.read(6) == 'STCM2L'.encode('932'):
		src.seek(0,os.SEEK_SET)
		return True
	else:
		src.seek(0,os.SEEK_SET)
		return False

'''
0x20处的int是EXPORT_DATA信息的offset，然后跟数目，明显特征就是这个offset跳过去前面有串EXPORT_DATA
每块的信息占0x28字节：
struct EXPORT_DATA_INFO
{
	unit32 flag;//不知干啥子用，多数为0
	char name[0x20];//名字
	unit32 offset;//地址
}；
'''
def Get_EXPORT_DATA(src):
	export_data_list = []
	src.seek(0x20,os.SEEK_SET)
	export_data_offset = byte2int(src.read(4))
	export_data_count = byte2int(src.read(4))
	src.seek(export_data_offset - 0x0C,os.SEEK_SET)
	if src.read(0x0C) != b'EXPORT_DATA\x00':
		print('模块头不是EXPORT_DATA！offset:0x%X'%(src.tell() - 0x0C))
		os.system('pause')
		exit(0)
	for i in range(0,export_data_count):
		dist = {}
		dist['flag'] = byte2int(src.read(4))
		dist['name'] = src.read(0x20).decode('932')
		dist['offset'] = byte2int(src.read(4))
		export_data_list.append(dist)
	return export_data_offset - 0x0C

def Get_GLOBAL_DATA(src):
	src.seek(0x50,os.SEEK_SET)
	if src.read(11).decode('932') == 'GLOBAL_DATA':
		src.seek(0x70,os.SEEK_SET)
		while src.read(0x10)[:12] != b'CODE_START_\x00':
			pass
		src.seek(-0x10,os.SEEK_CUR)
		return True
	return False

'''
struct InstructionHeader
{
	uint32_t is_call; // 0 or 1
	uint32_t opcode_offset;
	uint32_t param_count; // < 16
	uint32_t size;
};
如果is_call是1，opcode_offset是指令的offset
如果是0，opcode_offset是单纯的opcode
opcode:
	D2:普通文本
	D4:人名
	E7:选项
	F4BC:is_call是1，也是文本，但可能并不是F4BC，特征就是含有#Name[0]
	F85C:疑似选项
	23C:选项
	2C8:选项
'''
def Get_CODE_START(src,endoffset):
	if src.read(0x0C) != b'CODE_START_\x00':
		print('模块头不是CODE_START_！offset:0x%X'%(src.tell() - 0x10))
		os.system('pause')
		exit(0)
	data_offset = []
	while src.tell() != endoffset:
		is_call = byte2int(src.read(4))
		opcode = byte2int(src.read(4))
		param_count = byte2int(src.read(4))
		size = byte2int(src.read(4))
		if param_count != 0:
			param_list = []
			count = 0
			for i in range(0, param_count):
				param = byte2int(src.read(4))
				#过滤，可以按需增减
				if opcode == 0xD2 or opcode == 0xD4 or opcode == 0xE7 or opcode == 0xF4BC or opcode == 0x23C or opcode == 0x2C8 or opcode == 0xF85C:
					param_list.append(param)
				#elif opcode == 0x1D7:# or 0x1AE:
				#	param_list.append(param)
				param = byte2int(src.read(4))
				param = byte2int(src.read(4))
				count += 0x0C
			for param in param_list:
				if param >> 30 == 0:
					data_offset.append(param)
			src.read(size - count - 0x10)
	str_list = []
	for offset in data_offset:
		src.seek(offset,os.SEEK_SET)
		dtype = byte2int(src.read(4))
		offset_unit = byte2int(src.read(4))
		field = byte2int(src.read(4))
		size = byte2int(src.read(4))
		if offset_unit != size / 4:
			print(offset)
			os.system('pause')
		if size == 4:
			data = byte2int(src.read(4))
			if data != 0 and data & 0xFF000000 == 0:
				str_list.append(int2byte(data).decode('932').replace('\x00',''))
		else:
			data = src.read(size)
			str_list.append(data.decode('932').replace('\x00',''))
	return str_list

def main():
	fn_list = walk(sys.argv[1])
	for fn in fn_list:
		if fn[-4:] == '.txt':
			continue
		print(fn)
		src = open(fn,'rb')
		if CheckHeader(src):
			export_data_offset = Get_EXPORT_DATA(src)
			if Get_GLOBAL_DATA(src):
				str_list = Get_CODE_START(src,export_data_offset)
				if len(str_list) != 0:
					dst = open(fn + '.txt','w',encoding='utf16')
					for row in str_list:
						if len(row) != len(row.encode('932')):
							dst.write(row + '\n')
						elif row == '#Name[0]':
							dst.write(row + '\n')

main()