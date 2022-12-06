# -*- coding: utf-8 -*-
import os
import sys
import io
import struct

def main():
	src = open('St000_SldtDat.bin','rb')
	data = src.read(2)
	count = struct.unpack('H',data)[0]
	dst = open('St000_SldtDat.txt','w',encoding='utf16')
	for i in range(0,count):
		cpnum = struct.unpack('H',src.read(2))[0]
		stnum = struct.unpack('H',src.read(2))[0]
		cnum = struct.unpack('L',src.read(4))[0]
		offset = struct.unpack('L',src.read(4))[0]
		offset += 0x60
		filenum = struct.unpack('H',src.read(2))[0]
		allnum = struct.unpack('H',src.read(2))[0]
		unk1 = struct.unpack('H',src.read(2))[0]
		unk2 = struct.unpack('H',src.read(2))[0]
		src.seek(0x82, os.SEEK_CUR)
		string = 'cpnum:%04X|stnum:%04X|cnum:%08X|offset:%08X|filenum:%04X|allnum:%04X|unk1:%04X|unk2:%04X\n'%(cpnum,stnum,cnum,offset,filenum,allnum,unk1,unk2)
		dst.write(string)

main()