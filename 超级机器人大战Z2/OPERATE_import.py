# -*- coding:utf-8 -*-
#用于导入OPERATE文件的文本
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
			if adrlist.find('OPERATE') != -1 and adrlist.find('txt') == -1:
				mylist.append(adrlist)
	return mylist

def byte2int(byte):
	return struct.unpack('H',byte)[0]

def int2byte(num):
	return struct.pack('L',num)

def main():
	fn_lst = walk('EVENT')
	tbl = open('tbl.txt','r',encoding='utf16')
	dicts = {}
	for rows in tbl:
		row = rows.rstrip('\r\n').split('=')
		if len(row) == 3:
			row[1] = '='
		dicts[row[1]]=int(row[0],16)
	for fn in fn_lst:
		print(fn)
		src = open(fn,'rb')
		dst = open(fn.replace('EVENT','EVENT_dst'),'wb')
		src.seek(4,os.SEEK_SET)
		offset = byte2int(src.read(2))
		size = byte2int(src.read(2))
		src.seek(0,os.SEEK_SET)
		dst.write(src.read(offset))
		txt = open(fn+'.txt','r',encoding='utf16')
		for rows in txt.readlines():
			if rows[0] == '●':
				row = rows.rstrip('\r\n')[10:].replace('\\n','\n')
				try:
					for ch in row:
						if ch == '\n':
							dst.write(b'\x0A')
						elif dicts[ch] > 0xFF:
							dst.write(struct.pack('>H',dicts[ch]))
						elif dicts[ch] <= 0xFF:
							dst.write(struct.pack('B',dicts[ch]))
				except Exception as inst:
					print(row)
					print(inst)
					#os.system("pause")
				dst.write(b'\x00\x00')
		if (dst.tell() % 8) != 0:
			for i in range(0,8 - (dst.tell() % 8)):
				dst.write(b'\x00')
		size = dst.tell()
		dst.seek(6,os.SEEK_SET)
		dst.write(struct.pack('H',size))

if '__main__' == __name__:
	main()