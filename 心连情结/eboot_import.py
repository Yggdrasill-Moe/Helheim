# -*- coding: utf-8 -*-
import os
import sys
import io
import struct
import hashlib

def main():
	tbl = open('tbl.txt','r',encoding='utf16')
	dicts = {}
	for rows in tbl:
		row = rows.rstrip('\r\n').split('=')
		if len(row) == 3:
			row[1] = '='
		dicts[row[1]]=int(row[0],16)
	dicts['↙'] = 0x0A
	fn = 'EBOOT.BIN'
	txt = open(fn + '.TXT','r',encoding='utf16')
	rows = txt.readlines()
	str_lst = []
	for row in rows:
		if row[0] == '●':
			num = int(row[1:9])
			offset = int(row[11:18],16)
			length = int(row[19:row.rfind('●')])
			string = row[row.rfind('●') + 1:].rstrip('\r\n')
			str_lst.append((num,offset,length,string))
	src = open(fn,'rb')
	data = src.read()
	dst = open('EBOOTn.BIN','wb')
	dst.write(data)
	for row in str_lst:
		dst.seek(row[1],os.SEEK_SET)
		for i in range(0,row[2]):
			dst.write(b'\x00')
	for i in range(0,len(str_lst)):
		dst.seek(str_lst[i][1],os.SEEK_SET)
		for ch in str_lst[i][3]:
			if dicts[ch] > 0xFF:
				dst.write(struct.pack('>H',dicts[ch]))
			else:
				dst.write(struct.pack('B',dicts[ch]))
		if i != len(str_lst) - 1 and dst.tell() > str_lst[i+1][1] - 2:#预留两个00
			print(hex(dst.tell()),hex(str_lst[i+1][1]))
			#print(str_lst[i])
			#os.system('pause')

main()