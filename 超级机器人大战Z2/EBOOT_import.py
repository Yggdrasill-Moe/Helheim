# -*- coding:utf-8 -*-
#用于导入EBOOT.BIN的文本及索引信息
#by Darkness-TX 2023.04.07
import struct
import os
import sys
import io

def int2byte(num):
	return struct.pack('L',num)

def writeindex(src):
	files = [f for f in os.listdir() if os.path.isfile(f) and f.find('txt') != -1 and f.find('new') != -1]
	offdict = {}
	offdict['DATA000'] = 0x34AB34
	offdict['DATA013'] = 0x44567C
	offdict['DATA014'] = 0x347A9C
	offdict['DATA015'] = 0x3A974C
	offdict['DATA020'] = 0x4403E8
	offdict['DATA021'] = 0x441228
	offdict['SRVC.BI'] = 0x4B2FCC
	offdict['Z2DATA.'] = 0x1E6138
	offdict['Z2DATA3'] = 0x442EDC
	offdict['VEFF2DX'] = 0x33E278
	for file in files:
		if file[:7] in offdict:
			print('change:%s'%(file[:7]))
			src.seek(offdict[file[:7]],os.SEEK_SET)
			info = open(file,'r',encoding='utf16')
			for rows in info.readlines():
				offset = int(rows.split('|')[1],16)
				src.write(int2byte(offset))

def main():
	baseaddr = 0x8803FAC
	tbl = open('tbl.txt','r',encoding='utf16')
	dicts = {}
	for rows in tbl:
		row = rows.rstrip('\r\n').split('=')
		if len(row) == 3:
			row[1] = '='
		dicts[row[1]]=int(row[0],16)
	src = open('BOOT.BIN','rb')
	dst = open('EBOOT.BIN','wb')
	data = src.read()
	dst.write(data)
	txt = open('BOOT.TXT','r',encoding='utf16')
	logflag = False
	txt_lst = txt.readlines()
	for i in range(0,len(txt_lst)):
		if txt_lst[i][0] == '●':
			row = txt_lst[i].rstrip('\r\n')[19:].replace('\\n','\n')
			offset = int(txt_lst[i][1:9],16)
			size = int(txt_lst[i][10:18],16)
			org_offset = int(txt_lst[i-1][1:9],16)
			if org_offset != offset:
				if data.find(int2byte(baseaddr + org_offset)) != -1:
					print('offset:%X change:%X'%(data.find(int2byte(baseaddr + org_offset)),baseaddr + offset))
					print(txt_lst[i])
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
			if len(w) > size:
				if logflag == False:
					log = open('log.txt','w',encoding='utf16')
					logflag = True
				log.write('mem_offset:%X\n'%(baseaddr + org_offset))
				log.write(txt_lst[i-1] + txt_lst[i] + '\n')
			else:
				dst.seek(offset,os.SEEK_SET)
				dst.write(w)
				for j in range(0,size - len(w)):
					dst.write(b'\x00')
	txt = open('BOOT_UTF8.TXT','r',encoding='utf16')
	txt_lst = txt.readlines()
	for i in range(0,len(txt_lst)):
		if txt_lst[i][0] == '●':
			row = txt_lst[i].rstrip('\r\n')[19:].replace('↙','\n')
			offset = int(txt_lst[i][1:9],16)
			size = int(txt_lst[i][10:18],16)
			org_offset = int(txt_lst[i-1][1:9],16)
			if org_offset != offset:
				if data.find(int2byte(baseaddr + org_offset)) != -1:
					print('offset:%X change:%X'%(data.find(int2byte(baseaddr + org_offset)),baseaddr + offset))
					print(txt_lst[i])
			w = row.encode('utf-8')
			if len(w) > size:
				if logflag == False:
					log = open('log.txt','w',encoding='utf16')
					logflag = True
				log.write('mem_offset:%X\n'%(baseaddr + org_offset))
				log.write(txt_lst[i-1] + txt_lst[i] + '\n')
			else:
				dst.seek(offset,os.SEEK_SET)
				dst.write(w)
				for j in range(0,size - len(w)):
					dst.write(b'\x00')
	font = open('FONT.BIN','rb')
	dst.seek(0x1E6A74,os.SEEK_SET)
	dst.write(font.read())
	writeindex(dst)

if '__main__' == __name__:
	main()