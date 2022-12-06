# -*- coding: utf-8 -*-
import os
import sys
import io
import struct
import hashlib

def walk(adr):
	mylist=[]
	for root,dirs,files in os.walk(adr):
		for name in files:
			if name[-4:] != '.bin':
				continue
			adrlist=os.path.join(root, name)
			mylist.append(adrlist)
	return mylist

def main():
	fn_lst = walk('org')
	tbl = open('tbl.txt','r',encoding='utf16')
	dicts = {}
	for rows in tbl:
		row = rows.rstrip('\r\n').split('=')
		if len(row) == 3:
			row[1] = '='
		dicts[row[1]]=int(row[0],16)
	jump_dicts = {}
	jump_dicts_dst = {}
	src = open('St000_SldtDat.txt','r',encoding='utf16')
	for rows in src:
		row = rows.rstrip('\r\n').split('|')
		uid = row[0].split(':')[1] + row[1].split(':')[1][1:]
		if uid not in jump_dicts:
			jump_dicts[uid] = []
			jump_dicts_dst[uid] = []
		jump_dicts[uid].append((int(row[3].split(':')[1],16),int(row[5].split(':')[1],16)))
		jump_dicts_dst[uid].append((int(row[3].split(':')[1],16),int(row[5].split(':')[1],16)))
	src = open('St000_SldtDat.bin','rb')
	jump_data = src.read()
	jump_dst = open('dst\\St000.cpk_unpacked\\St000_SldtDat.bin','wb')
	jump_dst.write(jump_data)
	for fn in fn_lst:
		print(fn)
		uid = fn[fn.find('Cp')+2:fn.find('Cp')+6] + fn[fn.find('St')+2:fn.find('St')+5]
		str_lst = []
		src = open(fn,'rb')
		data = src.read()
		txt = open(fn.replace('.bin','.txt'),'r',encoding='utf16')
		for row in txt.readlines():
			if row[0] == '●':
				str_lst.append((int(row[1:29].split('|')[0]),row[1:29].split('|')[1],int(row[1:29].split('|')[2],16),int(row[1:29].split('|')[3],16),row[30:].rstrip('\r\n')))
		dst = open('dst'+fn[fn.find('\\'):-4]+'.cpk_unpacked'+fn[fn.find('\\'):],'wb')
		offset = 0
		for row in str_lst:
			string = b''
			for ch in row[4]:
				if dicts[ch] > 0xFF:
					string += struct.pack('>H',dicts[ch])
				else:
					string += struct.pack('B',dicts[ch])
			string = string.replace('\\n'.encode('932'),b'\x0D\x0A')
			#string = row[4].encode('932').replace('\\n'.encode('932'),b'\x0D\x0A')
			if row[1] == 'N' and len(string) > 20:
				print(row[4],'N')
				exit()
			elif row[1] == 'C' and len(string) > 0x38:
				print(row[4],'C')
				exit()
			if uid in jump_dicts:
				if len(string) != row[2] and row[1] not in ['N','C']:
					change = len(string) - row[2]
					for c in range(0,len(jump_dicts[uid])):
						if jump_dicts[uid][c][0] >= row[3]:
							jump_dicts_dst[uid][c] = (jump_dicts_dst[uid][c][0] + change,jump_dicts_dst[uid][c][1])
			if row[1] == 'T' or row[1] == 'L':
				dst.write(data[offset:row[3]])
				dst.write(string + b'\x00')
				offset = row[3] + row[2] + 1
			elif row[1] == 'N':
				dst.write(data[offset:row[3]])
				dst.write(string)
				for i in range(0,20 - len(string)):
					dst.write(b'\x00')
				offset = row[3] + 20
			elif row[1] == 'O':
				dst.write(data[offset:row[3]])
				dst.write(string + b'\x00\x00')
				offset = row[3] + row[2] + 2
			elif row[1] == 'C':
				dst.write(data[offset:row[3]])
				dst.write(string)
				for i in range(0,0x38 - len(string)):
					dst.write(b'\x00')
				offset = row[3] + 0x38
		count = len(data) - 1
		while data[count] == 0x88:
			count -= 1
		count += 1
		dst.write(data[offset:count])
		#先填充到0x10
		if dst.tell() % 0x10 != 0:
			for i in range(0,0x10 - dst.tell() % 0x10):
				dst.write(b'\x88')
		#判断填充字节数
		buff = int(hex(dst.tell())[-2],16)
		if (buff + 1) % 2 != 0:
			for i in range(0,0x10):
				dst.write(b'\x88')
		#jump表更新
		if uid in jump_dicts_dst:
			for c in range(0,len(jump_dicts_dst[uid])):
				jump_dst.seek(2 + 8 + jump_dicts_dst[uid][c][1] * 0x96,os.SEEK_SET)
				jump_dst.write(struct.pack('L',jump_dicts_dst[uid][c][0] - 0x60))

def main2():
	fn_lst = walk('org')
	for fn in fn_lst:
		src = open(fn,'rb')
		data = src.read()
		m = hashlib.md5()
		m.update(data)
		src = open(fn.replace('org','dst'),'rb')
		data = src.read()
		m2 = hashlib.md5()
		m2.update(data)
		if m.hexdigest() != m2.hexdigest():
			print(fn)
			print(m.hexdigest())
			print(m2.hexdigest())

main()