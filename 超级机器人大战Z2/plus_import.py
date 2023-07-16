# -*- coding:utf-8 -*-
#用于导入直接超长的文本
#by Darkness-TX 2023.04.08
import struct
import os
import sys
import io

def walk(adr):
	mylist=[]
	for root,dirs,files in os.walk(adr):
		for name in files:
			adrlist=os.path.join(root, name)
			if adrlist[-4:] == '.bin':
				mylist.append(adrlist)
	return mylist

def main():
	tbl = open('tbl.txt','r',encoding='utf16')
	dicts = {}
	for rows in tbl:
		row = rows.rstrip('\r\n').split('=')
		if len(row) == 3:
			row[1] = '='
		dicts[row[1]]=int(row[0],16)
	fn_lst = walk(sys.argv[1])
	dirname = sys.argv[1] + '.BIN_unpack'
	if os.path.exists(dirname) == False:
		os.mkdir(dirname)
	for fn in fn_lst:
		print(fn)
		src = open(fn,'rb')
		dst = open(fn.replace(sys.argv[1],dirname),'wb')
		dst.write(src.read())
		txt = open(fn+'.txt','r',encoding='utf16')
		txt_lst = txt.readlines()
		for i in range(0,len(txt_lst)):
			if txt_lst[i][0] == '●':
				row = txt_lst[i].rstrip('\r\n')[19:].replace('\\n','\n')
				offset = int(txt_lst[i][1:9],16)
				size = int(txt_lst[i][10:18],16)
				w = b''
				try:
					for ch in row:
						if ch == '\n':
							w += b'\x0A'
						elif dicts[ch] > 0xFF:
							w += struct.pack('>H',dicts[ch])
						elif dicts[ch] <= 0xFF:
							w += struct.pack('B',dicts[ch])
				except Exception as inst:
					print(txt_lst[i])
					print(inst)
					#os.system("pause")
				dst.seek(offset,os.SEEK_SET)
				dst.write(w)
				if len(w) < size:
					for j in range(0,size - len(w)):
						dst.write(b'\x00')

if '__main__' == __name__:
	main()