# -*- coding:utf-8 -*-
#用于导入剧情简介文本
#by Darkness-TX 2023.04.15
import struct
import os
import sys
import io

def int2byte(num):
	return struct.pack('H',num)

def main():
	tbl = open('tbl.txt','r',encoding='utf16')
	dicts = {}
	for rows in tbl:
		row = rows.rstrip('\r\n').split('=')
		if len(row) == 3:
			row[1] = '='
		dicts[row[1]]=int(row[0],16)
	src = open(sys.argv[1]+'.txt','r',encoding='utf16')
	dst = open(sys.argv[1]+'.new','wb')
	for rows in src.readlines():
		if rows[0] == '●':
			num = int(rows[1:9])
			row = rows.rstrip('\r\n')[10:]
			w = b''
			try:
				for ch in row:
					if dicts[ch] > 0xFF:
						w += struct.pack('>H',dicts[ch])
					elif dicts[ch] <= 0xFF:
						w += struct.pack('B',dicts[ch])
			except Exception as inst:
				print(row)
				print(inst)
				#os.system("pause")
			w += b'\x00'
			if len(w)%2 != 0:
				w += b'\x00'
			dst.write(int2byte(num))
			dst.write(int2byte(len(w)))
			dst.write(w)

if '__main__' == __name__:
	main()