# -*- coding:utf-8 -*-
#用于导出文件头为IKA SEQ SYSTEM  v4.1 HuneX 2010 的文本。
#by Darkness-TX 2018.05.03
import struct
import os
import sys
import io

def byte2int(byte):
	long_tuple=struct.unpack('L',byte)
	long = long_tuple[0]
	return long

def short2int(byte):
	long_tuple=struct.unpack('H',byte)
	long = long_tuple[0]
	return long

def int2byte(num):
	return struct.pack('L',num)

def FormatString(string, count):
	res = "○%08d○%s\n●%08d●%s\n\n"%(count, string, count, string)
	return res

def dumpstr(src):
	bstr = b''
	c = src.read(1)
	while c != b'\x00':
		bstr += c
		c = src.read(1)
	return bstr.decode('932')

for f in os.listdir('データバイナリ作_2.bin_unpack'):
	if not f.endswith('.dec'):
		continue
	fs = open('データバイナリ作_2.bin_unpack/'+f,'rb')
	print(f)
	header = fs.read(0x20)
	if header.decode('932') != 'IKA SEQ SYSTEM  v4.1 HuneX 2010 ':
		print('不支持的文件头')
		os.system('pause')
		continue
	data = fs.read()
	str_list = []
	pos = 0
	while data.find(b'\x24',pos) != -1:
		pos = data.find(b'\x24',pos)
		if data[pos + 1:pos + 3] == b'\x01\x5C' or data[pos + 1:pos + 3] == b'\x03\x5C':
			if data[pos + 4] == 38 or data[pos + 3] == 0:#'&'
				pos += 3
				continue
			count = data[pos + 3] - 4
			offset = pos + 4 + 0x20
			pos += count
			fs.seek(offset,os.SEEK_SET)
			l = dumpstr(fs)
			if len(l.encode('932')) == count:
				str_list.append(l)
			elif len(l.encode('932')) != 0:
				print('len:0x%X count:0x%X offset:0x%X %s'%(len(l),count,offset,l))
				os.system('pause')
		pos += 3
	if len(str_list) != 0:
		dst = open('データバイナリ作_2.bin_unpack/'+f[:-4]+'.txt','w',encoding='utf16')
		i = 0
		for string in str_list:
			dst.write(FormatString(string, i))
			i += 1
		dst.close()
	fs.close()