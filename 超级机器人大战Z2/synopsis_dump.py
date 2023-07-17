# -*- coding:utf-8 -*-
#用于导出剧情简介文本
#by Darkness-TX 2023.04.15
import struct
import os
import sys
import io

def byte2int(byte):
	return struct.unpack('H',byte)[0]

def FormatString(string, count):
	res = "○%08d○%s\n●%08d●%s\n\n"%(count, string, count, string)
	return res

def main():
	src = open(sys.argv[1],'rb')
	src.seek(0,os.SEEK_END)
	filesize = src.tell()
	src.seek(0,os.SEEK_SET)
	dst = open(sys.argv[1]+'.txt','w',encoding='utf16')
	while src.tell() < filesize:
		num = byte2int(src.read(2))
		size = byte2int(src.read(2))
		string = src.read(size).replace(b'\x00',b'').decode('932').replace('\n','↙')
		dst.write(FormatString(string,num))

if '__main__' == __name__:
	main()