# -*- coding:utf-8 -*-
#用于导入GIFTTBL文件的文本
#by Darkness-TX 2023.05.20
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
	str_lst = []
	txt = open('GIFTTBL.txt','r',encoding='utf16')
	for rows in txt.readlines():
		if rows[0] == '●':
			str_lst.append(rows[10:].rstrip('\r\n'))
	text_opcode = b'\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF'
	i = 0
	for fn in fn_lst:
		src = open(fn,'rb')
		off_lst = []
		off_lst_new = []
		offset_start = []
		data = src.read()
		src.seek(0,os.SEEK_SET)
		str_offset_start = data.find(text_opcode)
		if str_offset_start != -1:
			print(fn)
		while str_offset_start != -1:
			str_offset_start += 8
			offset_start.append(str_offset_start)
			off_lst.append(byte2int(data[str_offset_start:str_offset_start + 4]))
			str_offset_start = data.find(text_opcode, str_offset_start)
		if len(off_lst) != 0:
			off_start = off_lst[0]
			dst = open(fn.replace('EVENT','EVENT_dst'),'wb')
			dst.write(src.read(off_start))
			for j in off_lst:
				off_lst_new.append(dst.tell())
				row = str_lst[i]
				i += 1
				try:
					for ch in row:
						if dicts[ch] > 0xFF:
							dst.write(struct.pack('>H',dicts[ch]))
						elif dicts[ch] <= 0xFF:
							dst.write(struct.pack('B',dicts[ch]))
				except Exception as inst:
					print(row)
					print(inst)
					#os.system("pause")
				dst.write(b'\x00')
				if (dst.tell() % 2) != 0:
					dst.write(b'\x00')
			if (dst.tell() % 8) != 0:
				for j in range(0,8 - (dst.tell() % 8)):
					dst.write(b'\x00')
			j = 0
			for offset in offset_start:
				dst.seek(offset,os.SEEK_SET)
				dst.write(int2byte(off_lst_new[j]))
				j += 1

if '__main__' == __name__:
	main()