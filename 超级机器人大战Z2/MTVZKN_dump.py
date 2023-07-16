# -*- coding:utf-8 -*-
#用于导出图鉴文件的文本
#by Darkness-TX 2023.04.15
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

def FormatString(string, count):
	res = "○%08d○%s\n●%08d●%s\n\n"%(count, string, count, string)
	#res = "○%08d○%s\n\n"%(count, string)
	return res

def main():
	fn_lst = walk(sys.argv[1])
	for fn in fn_lst:
		print(fn)
		src = open(fn,'rb')
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
		num = byte2int(tmp.read(4))
		size = byte2int(tmp.read(4))
		str_lst = []
		while tmp.tell() < filesize:
			block_name = tmp.read(4)
			if block_name.decode('932') == 'DSIZ':
				block_size = byte2int(tmp.read(4))
			elif block_name.decode('932') == 'DATA':
				block_size = byte2int(tmp.read(4))
				filesize = block_size + 0x20
			elif block_name.decode('932') == 'LorR':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
			elif block_name.decode('932') == 'VOIC':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
			elif block_name.decode('932') == 'HEIT':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
			elif block_name.decode('932') == 'WEIT':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
			elif block_name.decode('932') == 'LOOK':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
			elif block_name.decode('932') == 'PRDC':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				str_lst.append(buff.decode('932').replace('\n','↙'))
			elif block_name.decode('932') == 'WORD':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				str_lst.append(buff.decode('932').replace('\n','↙'))
			elif block_name.decode('932') == 'SRCE':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				str_lst.append(buff.decode('932').replace('\n','↙'))
			elif block_name.decode('932') == 'RBTN':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				str_lst.append(buff.decode('932').replace('\n','↙'))
			elif block_name.decode('932') == 'PLTN':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				str_lst.append(buff.decode('932').replace('\n','↙'))
			elif block_name.decode('932') == 'DSCR':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				str_lst.append(buff.decode('932').replace('\n','↙'))
			elif block_name.decode('932') == 'DSC2':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				str_lst.append(buff.decode('932').replace('\n','↙'))
			elif block_name.decode('932') == 'CHFN':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				str_lst.append(buff.decode('932').replace('\n','↙'))
			elif block_name.decode('932') == 'CHNN':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				str_lst.append(buff.decode('932').replace('\n','↙'))
			elif block_name.decode('932') == 'ACTR':
				block_size = byte2int(tmp.read(4))
				buff = tmp.read(block_size)
				str_lst.append(buff.decode('932').replace('\n','↙'))
			else:
				print(hex(tmp.tell()),block_name)
				os.system('pause')
				exit()
		tmp.close()
		os.remove('tmp.bin')
		if len(str_lst) != 0:
			dst = open(fn[:-4]+'.txt','w',encoding='utf16')
			for i in range(0,len(str_lst)):
				dst.write(FormatString(str_lst[i],i))

if '__main__' == __name__:
	main()