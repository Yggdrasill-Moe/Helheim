import os
import sys
import struct

tbl = open('汉化编码表.txt','r',encoding='utf16')
src = open('BOOT.BIN','rb')
dst = open('EBOOT.BIN','wb')
data = src.read()
dst.write(data)
offset = 0x132B66
dst.seek(offset,os.SEEK_SET)
char = tbl.read(1)
count = 0
buff = 0
while char:
	dst.write(char.encode('utf16')[2:])
	if len(char.encode('utf-8')) == 3:
		dst.write(char.encode('utf-8')[2:])
	dst.write(char.encode('utf-8')[1:2])
	dst.write(char.encode('utf-8')[:1])
	if len(char.encode('utf-8')) == 2:
		dst.write(b'\x00')
	dst.seek(3,os.SEEK_CUR)
	count += 8
	if count == 0x2F0:
		count = 0
		offset -= 0x2F0
		dst.seek(offset,os.SEEK_SET)
		buff += 1
	char = tbl.read(1)
print('共导入%d页'%(buff))