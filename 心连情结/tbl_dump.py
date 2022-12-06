import os
import sys
import struct

src = open('BOOT.BIN','rb')
dst = open('orglist.txt','w',encoding='utf16')
#8140-81FC
offset = 0x133714
count = 0
src.seek(offset,os.SEEK_SET)
while count < 0x498:
	code_shiftjis = src.read(2)
	code_shiftjis = code_shiftjis[1:] + code_shiftjis[:1]
	code_utf16 = src.read(2)
	code_utf8 = src.read(4)
	code_utf8 = code_utf8[2:3] + code_utf8[1:2] + code_utf8[:1]
	#if code_shiftjis.decode('932') != code_utf16.decode('utf16'):# or code_utf16.decode('utf16') != code_utf8.decode('utf-8') or code_utf8.decode('utf-8') != code_shiftjis.decode('932'):
	#	print('%s|%s'%(code_shiftjis.decode('932'),code_utf16.decode('utf16')))
	dst.write('%02X%02X=%s\n'%(code_shiftjis[0],code_shiftjis[1],code_shiftjis.decode('932')))
	count += 8
#824F-8258
offset = 0x1336C4
count = 0
src.seek(offset,os.SEEK_SET)
while count < 0x50:
	code_shiftjis = src.read(2)
	code_shiftjis = code_shiftjis[1:] + code_shiftjis[:1]
	code_utf16 = src.read(2)
	code_utf8 = src.read(4)
	code_utf8 = code_utf8[2:3] + code_utf8[1:2] + code_utf8[:1]
	#if code_shiftjis.decode('932') != code_utf16.decode('utf16'):# or code_utf16.decode('utf16') != code_utf8.decode('utf-8') or code_utf8.decode('utf-8') != code_shiftjis.decode('932'):
	#	print('%s|%s'%(code_shiftjis.decode('932'),code_utf16.decode('utf16')))
	dst.write('%02X%02X=%s\n'%(code_shiftjis[0],code_shiftjis[1],code_shiftjis.decode('932')))
	count += 8
#8260-829A
offset = 0x133524
count = 0
src.seek(offset,os.SEEK_SET)
while count < 0x1A0:
	code_shiftjis = src.read(2)
	code_shiftjis = code_shiftjis[1:] + code_shiftjis[:1]
	code_utf16 = src.read(2)
	code_utf8 = src.read(4)
	code_utf8 = code_utf8[2:3] + code_utf8[1:2] + code_utf8[:1]
	#if code_shiftjis.decode('932') != code_utf16.decode('utf16'):# or code_utf16.decode('utf16') != code_utf8.decode('utf-8') or code_utf8.decode('utf-8') != code_shiftjis.decode('932'):
	#	print('%s|%s'%(code_shiftjis.decode('932'),code_utf16.decode('utf16')))
	dst.write('%02X%02X=%s\n'%(code_shiftjis[0],code_shiftjis[1],code_shiftjis.decode('932')))
	count += 8
#829F-82F1
offset = 0x13328C
count = 0
src.seek(offset,os.SEEK_SET)
while count < 0x298:
	code_shiftjis = src.read(2)
	code_shiftjis = code_shiftjis[1:] + code_shiftjis[:1]
	code_utf16 = src.read(2)
	code_utf8 = src.read(4)
	code_utf8 = code_utf8[2:3] + code_utf8[1:2] + code_utf8[:1]
	#if code_shiftjis.decode('932') != code_utf16.decode('utf16'):# or code_utf16.decode('utf16') != code_utf8.decode('utf-8') or code_utf8.decode('utf-8') != code_shiftjis.decode('932'):
	#	print('%s|%s'%(code_shiftjis.decode('932'),code_utf16.decode('utf16')))
	dst.write('%02X%02X=%s\n'%(code_shiftjis[0],code_shiftjis[1],code_shiftjis.decode('932')))
	count += 8
#8340-8396
offset = 0x132FDC
count = 0
src.seek(offset,os.SEEK_SET)
while count < 0x2B0:
	code_shiftjis = src.read(2)
	code_shiftjis = code_shiftjis[1:] + code_shiftjis[:1]
	code_utf16 = src.read(2)
	code_utf8 = src.read(4)
	code_utf8 = code_utf8[2:3] + code_utf8[1:2] + code_utf8[:1]
	#if code_shiftjis.decode('932') != code_utf16.decode('utf16'):# or code_utf16.decode('utf16') != code_utf8.decode('utf-8') or code_utf8.decode('utf-8') != code_shiftjis.decode('932'):
	#	print('%s|%s'%(code_shiftjis.decode('932'),code_utf16.decode('utf16')))
	dst.write('%02X%02X=%s\n'%(code_shiftjis[0],code_shiftjis[1],code_shiftjis.decode('932')))
	count += 8
#839F-849F
offset = 0x132E54
count = 0
src.seek(offset,os.SEEK_SET)
while count < 0x188:
	code_shiftjis = src.read(2)
	code_shiftjis = code_shiftjis[1:] + code_shiftjis[:1]
	code_utf16 = src.read(2)
	code_utf8 = src.read(4)
	code_utf8 = code_utf8[2:3] + code_utf8[1:2] + code_utf8[:1]
	#if code_shiftjis.decode('932') != code_utf16.decode('utf16'):# or code_utf16.decode('utf16') != code_utf8.decode('utf-8') or code_utf8.decode('utf-8') != code_shiftjis.decode('932'):
	#	print('%s|%s'%(code_shiftjis.decode('932'),code_utf16.decode('utf16')))
	dst.write('%02X%02X=%s\n'%(code_shiftjis[0],code_shiftjis[1],code_shiftjis.decode('932')))
	count += 8
#核心部分
#889F-97FC
offset = 0x132B64
for i in range(0,31):
	count = 0
	src.seek(offset,os.SEEK_SET)
	while count < 0x2F0:
		code_shiftjis = src.read(2)
		code_shiftjis = code_shiftjis[1:] + code_shiftjis[:1]
		code_utf16 = src.read(2)
		code_utf8 = src.read(4)
		code_utf8 = code_utf8[2:3] + code_utf8[1:2] + code_utf8[:1]
		if code_shiftjis.decode('932') != code_utf16.decode('utf16'):# or code_utf16.decode('utf16') != code_utf8.decode('utf-8') or code_utf8.decode('utf-8') != code_shiftjis.decode('932'):
			print('%s|%s'%(code_shiftjis.decode('932'),code_utf16.decode('utf16')))
		dst.write('%02X%02X=%s\n'%(code_shiftjis[0],code_shiftjis[1],code_shiftjis.decode('932')))
		count += 8
	offset -= 0x2F0
#9840-9872
offset = 0x12D1AC
count = 0
src.seek(offset,os.SEEK_SET)
while count < 0x198:
	code_shiftjis = src.read(2)
	code_shiftjis = code_shiftjis[1:] + code_shiftjis[:1]
	code_utf16 = src.read(2)
	code_utf8 = src.read(4)
	code_utf8 = code_utf8[2:3] + code_utf8[1:2] + code_utf8[:1]
	if code_shiftjis.decode('932') != code_utf16.decode('utf16'):# or code_utf16.decode('utf16') != code_utf8.decode('utf-8') or code_utf8.decode('utf-8') != code_shiftjis.decode('932'):
		print('%s|%s'%(code_shiftjis.decode('932'),code_utf16.decode('utf16')))
	dst.write('%02X%02X=%s\n'%(code_shiftjis[0],code_shiftjis[1],code_shiftjis.decode('932')))
	count += 8
#989F-98FC
offset = 0x12D004
count = 0
src.seek(offset,os.SEEK_SET)
while count < 0x1A8:
	code_shiftjis = src.read(2)
	code_shiftjis = code_shiftjis[1:] + code_shiftjis[:1]
	code_utf16 = src.read(2)
	code_utf8 = src.read(4)
	code_utf8 = code_utf8[2:3] + code_utf8[1:2] + code_utf8[:1]
	if code_shiftjis.decode('932') != code_utf16.decode('utf16'):# or code_utf16.decode('utf16') != code_utf8.decode('utf-8') or code_utf8.decode('utf-8') != code_shiftjis.decode('932'):
		print('%s|%s'%(code_shiftjis.decode('932'),code_utf16.decode('utf16')))
	dst.write('%02X%02X=%s\n'%(code_shiftjis[0],code_shiftjis[1],code_shiftjis.decode('932')))
	count += 8
#997A-E978
offset = 0x12CE6C
count = 0
src.seek(offset,os.SEEK_SET)
while count < 0x198:
	code_shiftjis = src.read(2)
	code_shiftjis = code_shiftjis[1:] + code_shiftjis[:1]
	code_utf16 = src.read(2)
	code_utf8 = src.read(4)
	code_utf8 = code_utf8[2:3] + code_utf8[1:2] + code_utf8[:1]
	if code_shiftjis.decode('932') != code_utf16.decode('utf16'):# or code_utf16.decode('utf16') != code_utf8.decode('utf-8') or code_utf8.decode('utf-8') != code_shiftjis.decode('932'):
		print('%s|%s'%(code_shiftjis.decode('932'),code_utf16.decode('utf16')))
	dst.write('%02X%02X=%s\n'%(code_shiftjis[0],code_shiftjis[1],code_shiftjis.decode('932')))
	count += 8
