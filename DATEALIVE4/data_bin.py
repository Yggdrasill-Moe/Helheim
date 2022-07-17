'''
project：Helheim-DATE A LIVE 莲Dystopia
用于导入导出databin的数据
by Destinyの火狐 2022.07.16
'''
# -*- coding:utf-8 -*-
import os
import struct
import io
import sys

def help():
	print('usage:pac_text [-d|-i] folder')
	print('\t-d\tdump text from databin')
	print('\t-i\tpack text into databin')

def walk(adr,extname):
	mylist=[]
	for root,dirs,files in os.walk(adr):
		for name in files:
			if name[-4:] != extname or name.find('data_') == -1:
				continue
			adrlist=os.path.join(root, name)
			mylist.append(adrlist)
	return mylist

def dumpstr(src):
	bstr = b''
	c = src.read(1)
	while c != b'\x00':
		bstr += c
		c = src.read(1)
	return bstr.decode('utf-8')

def byte2int(byte):
	return struct.unpack('L',byte)[0]

def int2byte(num):
	return struct.pack('L',num)

def dump():
	fn_lst = walk(sys.argv[2],'.bin')
	for fn in fn_lst:
		print(fn)
		src = open(fn,'rb')
		title_num = byte2int(src.read(4))
		data_num = byte2int(src.read(4))
		data_off = byte2int(src.read(4))
		title_lst = []
		for i in range(0, title_num):
			title_type = byte2int(src.read(4))
			title_off = byte2int(src.read(4))
			title_lst.append((title_type, title_off))
		data_lst = []
		for i in range(0, data_num):
			src.seek(i * title_num * 4 + data_off, os.SEEK_SET)
			member_lst = []
			for j in range(0, title_num):
				member_lst.append(byte2int(src.read(4)))
			data_lst.append(member_lst)
		dst = open(fn[:-4] + '.txt', 'w', encoding='utf-8-sig')
		title_result = []
		for row in title_lst:
			src.seek(row[1], os.SEEK_SET)
			name = dumpstr(src)
			title_result.append('%s|%d'%(name,row[0]))
		dst.write(','.join(title_result) + '\n')
		for rows in data_lst:
			data_result = []
			for i in range(0,len(rows)):
				if title_lst[i][0] == 1:
					data_result.append(str(rows[i]))
				elif title_lst[i][0] == 5:
					src.seek(rows[i], os.SEEK_SET)
					data_result.append(dumpstr(src).replace('\n','[\\n]'))
				else:
					print('未知的title类型 type:%d'%(title_lst[i][0]))
					os.system('pause')
			dst.write(','.join(data_result) + '\n')

def pack():
	fn_lst = walk(sys.argv[2],'.txt')
	for fn in fn_lst:
		print(fn)
		src = open(fn,'r',encoding='utf-8-sig')
		lines = src.readlines()
		title_lst = lines[0].rstrip('\r\n').split(',')
		dst = open(fn[:-4] + '.bin', 'wb')
		data_start = len(title_lst) * 2 * 4 + 12
		dst.write(int2byte(len(title_lst)))
		dst.write(int2byte(len(lines) - 1))
		dst.write(int2byte(data_start))
		title_type = []
		for title in title_lst:
			dst.write(int2byte(int(title.split('|')[1])))
			title_type.append(int(title.split('|')[1]))
			dst.write(int2byte(0))
		dst.seek((len(lines) - 1) * len(title_lst) * 4, os.SEEK_CUR)
		for i in range(0, len(title_lst)):
			title_off = dst.tell()
			dst.write(title_lst[i].split('|')[0].encode('utf-8') + b'\x00')
			dst.seek(16 + i * 8)
			dst.write(int2byte(title_off))
			dst.seek(0, os.SEEK_END)
		for i in range(1, len(lines)):
			rows = lines[i].rstrip('\r\n').split(',')
			for j in range(0,len(rows)):
				data_off = dst.tell()
				if title_type[j] == 1:
					dst.seek(data_start + (i - 1) * len(title_lst) * 4 + j * 4, os.SEEK_SET)
					dst.write(int2byte(int(rows[j])))
				elif title_type[j] == 5:
					dst.seek(data_start + (i - 1) * len(title_lst) * 4 + j * 4, os.SEEK_SET)
					dst.write(int2byte(data_off))
					dst.seek(0, os.SEEK_END)
					dst.write(rows[j].replace('[\\n]','\n').encode('utf-8') + b'\x00')
				else:
					print('未知的title类型 type:%d'%(title_type[j]))
					os.system('pause')
				dst.seek(0, os.SEEK_END)

def main():
	print('project：Helheim-DATE A LIVE 莲Dystopia')
	print('用于导入导出databin的数据')
	print('by Destinyの火狐 2022.07.16\n')
	if len(sys.argv) != 3:
		help()
	else:
		if sys.argv[1] == '-d':
			dump()
		elif sys.argv[1] == '-i':
			pack()
		else:
			help()

if __name__ == '__main__':
	main()