# -*- coding:utf-8 -*-
#用于导出OPERATE文件的文本
#by Darkness-TX 2023.04.07
import struct
import os
import sys
import io

def walk(adr):
	mylist=[]
	for root,dirs,files in os.walk(adr):
		for name in files:
			adrlist=os.path.join(root, name)
			if adrlist.find('OPERATE') != -1:
				mylist.append(adrlist)
	return mylist

def byte2int(byte):
	return struct.unpack('H',byte)[0]

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
	for fn in fn_lst:
		print(fn)
		src = open(fn,'rb')
		src.seek(4,os.SEEK_SET)
		offset = byte2int(src.read(2))
		size = byte2int(src.read(2))
		src.seek(offset,os.SEEK_SET)
		str_lst = []
		while src.tell() < size:
			string = dumpstr(src)
			if string != '':
				str_lst.append(string)
		if len(str_lst) != 0:
			dst = open(fn + '.txt','w',encoding='utf16')
			count = 0
			for row in str_lst:
				dst.write(FormatString(row.replace('\n','\\n'),count))
				count += 1

if '__main__' == __name__:
	main()