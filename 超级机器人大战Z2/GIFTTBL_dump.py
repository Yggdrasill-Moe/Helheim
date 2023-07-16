# -*- coding:utf-8 -*-
#用于导出GIFTTBL文件的文本
#by Darkness-TX 2023.04.20
import struct
import os
import sys
import io

def walk(adr):
	mylist=[]
	for root,dirs,files in os.walk(adr):
		for name in files:
			adrlist=os.path.join(root, name)
			if adrlist.find('GIFTTBL') != -1:
				mylist.append(adrlist)
	return mylist

def byte2int(byte):
	return struct.unpack('L',byte)[0]

def dumpstr(src):
	bstr = b''
	c = src.read(1)
	while c != b'\x00' and c != b'':
		bstr += c
		c = src.read(1)
	return bstr.decode('932')

def FormatString(string, count):
	res = "○%08d○%s\n●%08d●%s\n\n"%(count, string, count, string)
	return res

def main():
	fn_lst = walk('EVENT')
	text_opcode = b'\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF'
	str_lst = []
	for fn in fn_lst:
		src = open(fn,'rb')
		off_lst = []
		data = src.read()
		str_offset_start = data.find(text_opcode)
		if str_offset_start != -1:
			print(fn)
		while str_offset_start != -1:
			str_offset_start += 8
			off_lst.append(byte2int(data[str_offset_start:str_offset_start + 4]))
			str_offset_start = data.find(text_opcode, str_offset_start)
		for offset in off_lst:
			src.seek(offset,os.SEEK_SET)
			str_lst.append(dumpstr(src))
	if len(str_lst) != 0:
		dst = open('GIFTTBL.txt','w',encoding='utf16')
		i = 0
		for row in str_lst:
			dst.write(FormatString(row,i))
			i += 1

if '__main__' == __name__:
	main()