# -*- coding:utf-8 -*-
#用于导入SRVC的文本
#by Darkness-TX 2023.04.10
import struct
import os
import sys
import io

def walk(adr):
	mylist=[]
	for root,dirs,files in os.walk(adr):
		for name in files:
			adrlist=os.path.join(root, name)
			if adrlist[-3:] == 'txt' and adrlist.find('point') == -1:
				mylist.append(adrlist.replace('.txt',''))
	return mylist

def byte2int(byte):
	return struct.unpack('L',byte)[0]

def short2int(byte):
	return struct.unpack('H',byte)[0]

def int2byte(num):
	return struct.pack('L',num)

def int2short(num):
	return struct.pack('H',num)

def main():
	fn_lst = walk('SRVC.BIN_unpack')
	tbl = open('tbl.txt','r',encoding='utf16')
	dicts = {}
	for rows in tbl:
		row = rows.rstrip('\r\n').split('=')
		if len(row) == 3:
			row[1] = '='
		dicts[row[1]]=int(row[0],16)
	if os.path.exists('SRVC.BIN_dst') == False:
		os.mkdir('SRVC.BIN_dst')
	for fn in fn_lst:
		print(fn)
		src = open(fn,'rb')
		src.seek(0,os.SEEK_END)
		filesize = src.tell()
		src.seek(0,os.SEEK_SET)
		tmp = open(fn+'.tmp','wb+')
		txt = open(fn+'.txt','r',encoding='utf16')
		offset_dic = {}
		for rows in txt.readlines():
			if rows[0] == '●':
				row = rows.rstrip('\r\n')[19:]
				str_off = int(rows[10:18],16)
				offset_dic[str_off] = tmp.tell()
				try:
					for ch in row:
						if ch == '\n':
							tmp.write(b'\x0A')
						elif dicts[ch] > 0xFF:
							tmp.write(struct.pack('>H',dicts[ch]))
						elif dicts[ch] <= 0xFF:
							tmp.write(struct.pack('B',dicts[ch]))
				except Exception as inst:
					print(hex(str_off))
					print(row)
					print(inst)
					#os.system("pause")
				tmp.write(struct.pack('B',0))
		tmp.seek(0,os.SEEK_SET)
		dst = open(fn.replace('SRVC.BIN_unpack','SRVC.BIN_dst'),'wb')
		magic = short2int(src.read(2))
		if magic != 0x4801:
			print('magic:0x%X\n'%(magic))
			os.system('pause')
		dst.write(int2short(magic))
		group = struct.unpack('B',src.read(1))[0]
		group1 = struct.unpack('B',src.read(1))[0]
		group2 = short2int(src.read(2))
		group3 = short2int(src.read(2))
		dst.write(struct.pack('B',group))
		dst.write(struct.pack('B',group1))
		dst.write(int2short(group2))
		dst.write(int2short(group3))
		if group3 == 0:
			if src.tell() + 0x120 + 8 != filesize:
				if fn.find('00000181') != -1 or fn.find('00000182') != -1 or fn.find('00000271') != -1 or fn.find('00000276') != -1:
					if fn.find('00000181') != -1:
						dst.write(src.read(0x3B4-8))
						group3 = 0x58
					elif fn.find('00000182') != -1:
						dst.write(src.read(0x348-8))
						group3 = 0x4C
					elif fn.find('00000271') != -1:
						dst.write(src.read(0x350-8))
						group3 = 0x46
					elif fn.find('00000276') != -1:
						dst.write(src.read(0x5D0-8))
						group3 = 0x8E
				else:
					buff = byte2int(src.read(4))
					dst.write(int2byte(buff))
					if buff == 0xFF000000:
						src.seek(-4,os.SEEK_CUR)
						dst.seek(-4,os.SEEK_CUR)
					dst.write(src.read(0x120))
					buff = byte2int(src.read(4))
					dst.write(int2byte(buff))
					if buff != 0xFF000000:
						src.seek(-4,os.SEEK_CUR)
						dst.seek(-4,os.SEEK_CUR)
					dst.write(src.read(group2*4+2+4))
					group3 = short2int(src.read(2))
					dst.write(int2short(group3))
					buff = byte2int(src.read(4))
					dst.write(int2byte(buff))
					if buff != 0 and group2 != 0:
						group3 += 1
					src.seek(-4,os.SEEK_CUR)
					dst.seek(-4,os.SEEK_CUR)
		else:
			dst.write(src.read(group*8+group2*8+group1*4+0x120))
		point = open(fn+'_point.txt','r',encoding='utf16')
		point_data = point.readlines()
		if len(point_data) != group3:
			print(len(point_data),group3)
			os.system('pause')
		for i in range(0,group3):
			dst.write(src.read(4))
			src.seek(4,os.SEEK_CUR)
			key = int(point_data[i].rstrip('\r\n').split('|')[1],16)
			dst.write(int2byte(offset_dic[key]))
		dst.write(tmp.read())
		tmp.close()
		os.remove(fn+'.tmp')
		if (dst.tell() % 0x10) != 0:
			for i in range(0,0x10 - (dst.tell() % 0x10)):
				dst.write(b'\x00')

if '__main__' == __name__:
	main()