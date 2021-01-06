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
	offset_dict = {}
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
					offset_dict[param] = src.tell() - 4
				param = byte2int(src.read(4))
				param = byte2int(src.read(4))
				count += 0x0C
			for param in param_list:
				if param >> 30 == 0:
					data_offset.append(param)
			src.read(size - count - 0x10)
	str_off = []
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
				row = int2byte(data).decode('932').replace('\x00','')
			else:
				continue
		else:
			data = src.read(size)
			row = data.decode('932').replace('\x00','')
		if len(row) != len(row.encode('932')):
			str_off.append(offset_dict[offset])
		elif row == '#Name[0]':
			str_off.append(offset_dict[offset])
	return str_off

def Write_str(src,dst,str_off,str_list,dicts):
	for i in range(0,len(str_off)):
		row = str_list[i].rstrip('\r\n')
		if row == '#Name[0]':
			continue
		dst.seek(0,os.SEEK_END)
		noffset = dst.tell()
		src.seek(str_off[i],os.SEEK_SET)
		ooffset = byte2int(src.read(4))
		src.seek(ooffset,os.SEEK_SET)
		dst.write(src.read(4))#dtype
		src.seek(4,os.SEEK_CUR)
		size = len(row.replace('・','·').encode('936') + b'\x00')
		if size % 4 != 0:
			zero_size = 4 - (size % 4)
		else:
			zero_size = 0
		dst.write(int2byte(int((size + zero_size) / 4)))#offset_unit
		dst.write(src.read(4))#field
		dst.write(int2byte(size + zero_size))#size
		for ch in row:
			try:
				if dicts[ch] > 0xFF:
					dst.write(struct.pack('>H',dicts[ch]))
				else:
					dst.write(struct.pack('B',dicts[ch]))
			except Exception as inst:
				print(row)
		for k in range(0,zero_size + 1):
			dst.write(b'\x00')
		dst.seek(str_off[i],os.SEEK_SET)
		dst.write(int2byte(noffset))

def main():
	fn_list = walk(sys.argv[1])
	tbl = open('tbl.txt','r',encoding='utf16')
	dicts = {}
	for rows in tbl:
		row = rows.rstrip('\r\n').split('=')
		dicts[row[1]]=int(row[0],16)
	for fn in fn_list:
		if fn[-4:] == '.txt':
			continue
		print(fn)
		src = open(fn,'rb')
		if CheckHeader(src):
			export_data_offset = Get_EXPORT_DATA(src)
			if Get_GLOBAL_DATA(src):
				str_off = Get_CODE_START(src,export_data_offset)
				if os.path.exists('txt_out') == False:
					os.mkdir('txt_out')
				src.seek(0,os.SEEK_SET)
				data = src.read()
				dst = open(fn.replace(sys.argv[1], 'txt_out') + '.bin','wb')
				dst.write(data)
				txt = open(fn.replace(sys.argv[1],'txt_cn') + '.txt','r',encoding='utf16')
				str_list = txt.readlines()
				if len(str_off) != len(str_list):
					print(len(str_list),len(str_off))
					os.system('pause')
					exit(0)
				Write_str(src,dst,str_off,str_list,dicts)

main()