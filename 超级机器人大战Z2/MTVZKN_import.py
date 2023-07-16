# -*- coding:utf-8 -*-
#用于导入图鉴文件的文本
#by Darkness-TX 2023.04.20
import struct
import os
import sys
import io

def walk(adr):
	mylist=[]
	for root,dirs,files in os.walk(adr):
		for name in files:
			adrlist=os.path.join(root, name)
			if adrlist[-4:] == '.bin':
				mylist.append(adrlist)
	return mylist

def byte2int(byte):
	return struct.unpack('L',byte)[0]

def int2byte(num):
	return struct.pack('L',num)

def main():
	tbl = open('tbl.txt','r',encoding='utf16')
	dicts = {}
	for rows in tbl:
		row = rows.rstrip('\r\n').split('=')
		if len(row) == 3:
			row[1] = '='
		dicts[row[1]]=int(row[0],16)
	fn_lst = walk(sys.argv[1])
	dirname = sys.argv[1] + '.BIN_unpack'
	if os.path.exists(dirname) == False:
		os.mkdir(dirname)
	for fn in fn_lst:
		print(fn)
		src = open(fn,'rb')
		dst = open(fn.replace(sys.argv[1],dirname),'wb')
		count = byte2int(src.read(4))
		header_len = byte2int(src.read(4))
		src.seek(4,os.SEEK_CUR)
		filesize = byte2int(src.read(4))
		src.seek(header_len,os.SEEK_SET)
		tmp = open('tmp.bin','wb+')
		data = src.read()
		for i in range(0,len(data)):
			if data[i] == 0 or data[i] == 0x5E:
				tmp.write(struct.pack('B',data[i]))
			else:
				tmp.write(struct.pack('B',data[i] ^ 0x5E))
		tmp.seek(0,os.SEEK_SET)
		magic = tmp.read(8).decode('932')
		if magic != 'ZKANCHAR' and magic != 'ZKANROBO' and magic != 'ZKANKYWD':
			print('文件头不对！magic:' + magic)
			tmp.close()
			os.remove('tmp.bin')
			os.system('pause')
			exit()
		tmpn = open('tmpn.bin','wb+')
		tmpn.write(magic.encode('932'))
		num = byte2int(tmp.read(4))
		tmpn.write(int2byte(num))
		size = byte2int(tmp.read(4))
		tmpn.write(int2byte(size))
		txt = open(fn[:-4]+'.txt','r',encoding='utf16')
		txt_lst = txt.readlines()
		byte_lst = []
		for i in range(0,len(txt_lst)):
			if txt_lst[i][0] == '●':
				row = txt_lst[i].rstrip('\r\n')[10:].replace('\\n','\n')
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
				byte_lst.append(w)
		i = 0
		while tmp.tell() < filesize:
			block_name = tmp.read(4)
			if block_name.decode('932') == 'DSIZ':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				tmpn.write(int2byte(block_size))
			elif block_name.decode('932') == 'DATA':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				tmpn.write(int2byte(block_size))
				filesize = block_size + 0x20
			elif block_name.decode('932') == 'LorR':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				tmpn.write(int2byte(block_size))
				buff = tmp.read(block_size)
				tmpn.write(buff)
			elif block_name.decode('932') == 'VOIC':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				tmpn.write(int2byte(block_size))
				buff = tmp.read(block_size)
				tmpn.write(buff)
			elif block_name.decode('932') == 'HEIT':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				tmpn.write(int2byte(block_size))
				buff = tmp.read(block_size)
				tmpn.write(buff)
			elif block_name.decode('932') == 'WEIT':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				tmpn.write(int2byte(block_size))
				buff = tmp.read(block_size)
				tmpn.write(buff)
			elif block_name.decode('932') == 'LOOK':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				tmpn.write(int2byte(block_size))
				buff = tmp.read(block_size)
				tmpn.write(buff)
			elif block_name.decode('932') == 'PRDC':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				block_size = len(byte_lst[i])
				tmpn.write(int2byte(block_size))
				tmpn.write(byte_lst[i])
				i += 1
			elif block_name.decode('932') == 'WORD':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				block_size = len(byte_lst[i])
				tmpn.write(int2byte(block_size))
				tmpn.write(byte_lst[i])
				i += 1
			elif block_name.decode('932') == 'SRCE':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				block_size = len(byte_lst[i])
				tmpn.write(int2byte(block_size))
				tmpn.write(byte_lst[i])
				i += 1
			elif block_name.decode('932') == 'RBTN':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				block_size = len(byte_lst[i])
				tmpn.write(int2byte(block_size))
				tmpn.write(byte_lst[i])
				i += 1
			elif block_name.decode('932') == 'PLTN':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				block_size = len(byte_lst[i])
				tmpn.write(int2byte(block_size))
				tmpn.write(byte_lst[i])
				i += 1
			elif block_name.decode('932') == 'DSCR':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				block_size = len(byte_lst[i])
				tmpn.write(int2byte(block_size))
				tmpn.write(byte_lst[i])
				i += 1
			elif block_name.decode('932') == 'DSC2':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				block_size = len(byte_lst[i])
				tmpn.write(int2byte(block_size))
				tmpn.write(byte_lst[i])
				i += 1
			elif block_name.decode('932') == 'CHFN':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				block_size = len(byte_lst[i])
				tmpn.write(int2byte(block_size))
				tmpn.write(byte_lst[i])
				i += 1
			elif block_name.decode('932') == 'CHNN':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				block_size = len(byte_lst[i])
				tmpn.write(int2byte(block_size))
				tmpn.write(byte_lst[i])
				i += 1
			elif block_name.decode('932') == 'ACTR':
				tmpn.write(block_name)
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				block_size = len(byte_lst[i])
				tmpn.write(int2byte(block_size))
				tmpn.write(byte_lst[i])
				i += 1
			else:
				print(hex(tmp.tell()),block_name)
				os.system('pause')
				exit()
		data_size = tmpn.tell() - 0x20
		tmpn.seek(0x1C,os.SEEK_SET)
		tmpn.write(int2byte(data_size))
		tmpn.seek(0x14,os.SEEK_SET)
		tmpn.write(int2byte(data_size + 8))
		tmpn.seek(0,os.SEEK_END)
		if (tmpn.tell() % 0x10) != 0:
			for i in range(0,0x10 - (tmpn.tell() % 0x10)):
				tmpn.write(b'\x00')
		tmpn.seek(0,os.SEEK_SET)
		zkan_data = tmpn.read()
		tmpn.close()
		os.remove('tmpn.bin')
		tmp.close()
		os.remove('tmp.bin')
		dst.write(int2byte(count))
		dst.write(int2byte(header_len))
		dst.seek(4,os.SEEK_CUR)
		dst.write(int2byte(len(zkan_data)))
		dst.write(int2byte(len(zkan_data)))
		dst.seek(header_len,os.SEEK_SET)
		for i in range(0,len(zkan_data)):
			if zkan_data[i] == 0 or zkan_data[i] == 0x5E:
				dst.write(struct.pack('B',zkan_data[i]))
			else:
				dst.write(struct.pack('B',zkan_data[i] ^ 0x5E))

if '__main__' == __name__:
	main()