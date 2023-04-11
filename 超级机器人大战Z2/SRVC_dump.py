# -*- coding:utf-8 -*-
#用于导出SRVC的文本
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
			if adrlist[-3:] == 'bin':
				mylist.append(adrlist)
	return mylist

def byte2int(byte):
	return struct.unpack('L',byte)[0]

def short2int(byte):
	return struct.unpack('H',byte)[0]

def dumpstr(src):
	bstr = b''
	c = src.read(1)
	while c != b'\x00' and c != b'':
		bstr += c
		c = src.read(1)
	return bstr.decode('932')

def FormatString(string, count, offset):
	res = "○%08d|%08X○%s\n●%08d|%08X●%s\n\n"%(count, offset, string, count, offset, string)
	return res

def main():
	fn_lst = walk('SRVC.BIN_unpack')
	for fn in fn_lst:
		print(fn)
		src = open(fn,'rb')
		src.seek(0,os.SEEK_END)
		filesize = src.tell()
		src.seek(0,os.SEEK_SET)
		magic = short2int(src.read(2))
		if magic != 0x4801:
			print('magic:0x%X\n'%(magic))
			os.system('pause')
		group = struct.unpack('B',src.read(1))[0]
		group1 = struct.unpack('B',src.read(1))[0]
		group2 = short2int(src.read(2))
		group3 = short2int(src.read(2))
		if group3 == 0:
			if src.tell() + 0x120 + 8 != filesize:
				if fn.find('00000181') != -1 or fn.find('00000182') != -1 or fn.find('00000271') != -1 or fn.find('00000276') != -1:
					if fn.find('00000181') != -1:
						src.seek(0x3B4,os.SEEK_SET)
						group3 = 0x58
					elif fn.find('00000182') != -1:
						src.seek(0x348,os.SEEK_SET)
						group3 = 0x4C
					elif fn.find('00000271') != -1:
						src.seek(0x350,os.SEEK_SET)
						group3 = 0x46
					elif fn.find('00000276') != -1:
						src.seek(0x5D0,os.SEEK_SET)
						group3 = 0x8E
				else:
					buff = byte2int(src.read(4))
					if buff == 0xFF000000:
						src.seek(-4,os.SEEK_CUR)
					src.seek(0x120,os.SEEK_CUR)
					buff = byte2int(src.read(4))
					if buff != 0xFF000000:
						src.seek(-4,os.SEEK_CUR)
					src.seek(group2*4+2+4,os.SEEK_CUR)
					group3 = short2int(src.read(2))
					buff = byte2int(src.read(4))
					if buff != 0 and group2 != 0:
						group3 += 1
					src.seek(-4,os.SEEK_CUR)
		else:
			src.seek(group*8+group2*8+group1*4+0x120,os.SEEK_CUR)
		if group3 != 0:
			point = open(fn+'_point.txt','w',encoding='utf16')
			point_dic = {}
			for i in range(0,group3):
				src.seek(4,os.SEEK_CUR)
				offset = byte2int(src.read(4))
				point_dic[offset] = 0
				point.write('%08d|%08X\n'%(i,offset))
			savepos = src.tell()
			str_lst = []
			for key in sorted(point_dic.keys()):
				src.seek(savepos + key,os.SEEK_SET)
				data = dumpstr(src)
				if len(data) != 0:
					str_lst.append((key,data))
				else:
					print(hex(src.tell()),hex(savepos),hex(key),group3)
			dst = open(fn+'.txt','w',encoding='utf16')
			for i in range(0,len(str_lst)):
				if str_lst[i][1][0] not in ['「','（']:
					print(str_lst[i],str_lst[i][1][0])
					os.system('pause')
				dst.write(FormatString(str_lst[i][1],i,str_lst[i][0]))

if '__main__' == __name__:
	main()