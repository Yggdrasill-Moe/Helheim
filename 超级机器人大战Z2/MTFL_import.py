# -*- coding:utf-8 -*-
#用于导入文件头为MTFLz2_2文件的文本
#by Darkness-TX 2023.03.14
import struct
import os
import sys
import io

def walk(adr):
	mylist=[]
	for root,dirs,files in os.walk(adr):
		for name in files:
			if name[-4:].lower() == '.txt':
				continue
			adrlist=os.path.join(root, name)
			src = open(adrlist,'rb')
			data = src.read()
			src.close()
			if data[:8] == b'MTFLz2_2' and data.find(b'jstr') != -1:
				mylist.append(adrlist)
	return mylist

def byte2int(byte):
	return struct.unpack('L',byte)[0]

def int2byte(num):
	return struct.pack('L',num)

def main():
	fn_lst = walk('EVENT')
	tbl = open('tbl.txt','r',encoding='utf16')
	dicts = {}
	for rows in tbl:
		row = rows.rstrip('\r\n').split('=')
		if len(row) == 3:
			row[1] = '='
		dicts[row[1]]=int(row[0],16)
	for fn in fn_lst:
		print(fn)
		src = open(fn,'rb')
		dst = open(fn.replace('EVENT\\','EVENT_dst\\'),'wb')
		dst.write(src.read(8))
		header_len = byte2int(src.read(4))
		dst.write(int2byte(header_len))
		version = src.read(4)
		dst.write(version)
		filesize = byte2int(src.read(4))#加上header_len才是文件大小
		print('version:%s header_len:0x%X oldfilesize:0x%X'%(version.decode('932'),header_len,filesize+header_len))
		src.seek(header_len,os.SEEK_SET)
		dst.seek(header_len,os.SEEK_SET)
		#总块的信息，文件名带SDEMO的file_block就是sdem，带DLGEV的file_block就是dlev
		#0165的文件是kwrb
		file_block_name = src.read(4)
		file_block_size = byte2int(src.read(4))
		file_block_header_size = byte2int(src.read(4))
		file_block_data_size = byte2int(src.read(4))
		dst.seek(file_block_header_size + header_len,os.SEEK_SET)
		print('\t%s old_block_size:0x%X block_header_size:0x%X old_block_data_size:0x%X'%(file_block_name.decode('932'),file_block_size,file_block_header_size,file_block_data_size))
		src.seek(header_len + file_block_header_size,os.SEEK_SET)
		file_block_data_size = 0
		point_offset = 0
		#开始遍历块
		while src.tell() < file_block_size + header_len:
			pos = src.tell()
			dpos = dst.tell()
			block_name = src.read(4)
			block_size = byte2int(src.read(4))
			block_header_size = byte2int(src.read(4))
			block_data_size = byte2int(src.read(4))
			dst.write(block_name)
			dst.write(int2byte(block_size))
			dst.write(int2byte(block_header_size))
			dst.write(int2byte(block_data_size))
			if block_name.decode('932') == 'evtb':
				dst.seek(dpos + block_header_size,os.SEEK_SET)
				dst.write(src.read(block_data_size))
				file_block_data_size += block_size
			elif block_name.decode('932') == 'evnt':#文本指针
				point_offset = dpos + block_header_size
				block_group_num = byte2int(src.read(4))#每组长度为0x20
				dst.write(int2byte(block_group_num))
				dst.seek(dpos + block_header_size,os.SEEK_SET)
				for i in range(0,block_group_num):
					dst.write(src.read(0x20))
				file_block_data_size += block_size
			elif block_name.decode('932') == 'lkke':#地址长度全记录，只有一个文件有这个块
				point_offset = dpos + block_header_size
				block_group_num = byte2int(src.read(4))
				dst.write(int2byte(block_group_num))
				dst.seek(dpos + block_header_size,os.SEEK_SET)
				dst.write(src.read(block_data_size))
				file_block_data_size += block_size
			elif block_name.decode('932') == 'kywd':
				block_unk1 = byte2int(src.read(4))
				block_unk2 = byte2int(src.read(4))
				dst.write(int2byte(block_unk1))
				dst.write(int2byte(block_unk2))
				dst.seek(dpos + block_header_size,os.SEEK_SET)
				dst.write(src.read(block_data_size))
				file_block_data_size += block_size
			elif block_name.decode('932') == 'jstr':#文本
				buff = src.read(0x0F)#不处理
				src.seek(block_data_size - 0x0F,os.SEEK_CUR)
				dst.seek(dpos + block_header_size,os.SEEK_SET)
				point_dic = {}#key:指针，value:[新offset,新size,指针位置,...]
				point = open(fn+'_point.txt','r',encoding='utf16')
				for rows in point.readlines():
					row = rows.rstrip('\r\n').split('|')
					if int(row[2],16) not in point_dic:
						point_dic[int(row[2],16)] = [int(row[2],16),0]
						point_dic[int(row[2],16)].append(int(row[1],16))
					else:
						point_dic[int(row[2],16)].append(int(row[1],16))
				tmp = open(fn+'.tmp','wb+')
				tmp.write(buff)
				txt = open(fn+'.txt','r',encoding='utf16')
				for rows in txt.readlines():
					if rows[0] == '●':
						row = rows.rstrip('\r\n')[19:].replace('\\n','\n')
						str_off = int(rows[10:18],16)
						if str_off != 0:
							point_dic[str_off][0] = tmp.tell()
							try:
								for ch in row:
									if ch == '\n':
										tmp.write(b'\x0A')
									elif dicts[ch] > 0xFF:
										tmp.write(struct.pack('>H',dicts[ch]))
									elif dicts[ch] <= 0xFF:
										tmp.write(struct.pack('B',dicts[ch]))
							except Exception as inst:
								print(hex(str_off))
								print(row)
								print(inst)
								os.system("pause")
							point_dic[str_off][1] = tmp.tell() - point_dic[str_off][0]
							tmp.write(struct.pack('B',0))
				tmp.seek(-1,os.SEEK_END)
				tmp.truncate()#最后一句话没有00终止符
				block_data_size = tmp.tell()
				block_size = block_data_size + block_header_size
				file_block_data_size += block_size
				tmp.seek(0,os.SEEK_SET)
				dst.write(tmp.read(0x0F))
				buff = tmp.read()
				tmp.close()
				os.remove(fn+'.tmp')
				for i in range(0,len(buff)):
					if buff[i] == 0 or buff[i] == 0x7A:
						dst.write(struct.pack('B',buff[i]))
					else:
						dst.write(struct.pack('B',buff[i] ^ 0x7A))
				dst.seek(dpos,os.SEEK_SET)
				dst.write(block_name)
				dst.write(int2byte(block_size))
				dst.write(int2byte(block_header_size))
				dst.write(int2byte(block_data_size))
				#写指针
				for key in point_dic.keys():
					for i in range(2,len(point_dic[key])):
						dst.seek(point_offset + point_dic[key][i],os.SEEK_SET)
						dst.write(int2byte(point_dic[key][0]))
						if fn.find('MTFLz2_2') != -1:
							dst.write(int2byte(point_dic[key][1]))
				dst.seek(0,os.SEEK_END)
			else:
				print('\t未知的块名:%s'%(block_name.decode('932')))
				os.system('pause')
				dst.write(src.read(block_header_size - 0x10))
				dst.seek(dpos + block_header_size,os.SEEK_SET)
				dst.write(src.read(block_data_size))
				file_block_data_size += block_size
			print('\t\t%s block_size:0x%X block_header_size:0x%X block_data_size:0x%X'%(block_name.decode('932'),block_size,block_header_size,block_data_size))
		block_name = src.read(4)
		if block_name.decode('932') != 'ENDo':
			print('\t未读取到ENDo！offset:0x%X'%(src.tell() - 4))
			os.system('pause')
		else:
			dst.write(block_name)
		dst.seek(header_len,os.SEEK_SET)
		file_block_size = file_block_header_size + file_block_data_size
		dst.write(file_block_name)
		dst.write(int2byte(file_block_size))
		dst.write(int2byte(file_block_header_size))
		dst.write(int2byte(file_block_data_size))
		dst.seek(0x10,os.SEEK_SET)
		dst.write(int2byte(file_block_size))
		dst.seek(0,os.SEEK_END)
		buff = src.read(1)
		while buff != b'\x00' and buff != b'':
			dst.write(buff)
			buff = src.read(1)
		#MTFLz2_2.bin直接就是0x10字节对齐
		if fn.find('MTFLz2_2') != -1:
			if (dst.tell() % 0x10) != 0:
				for i in range(0,0x10 - (dst.tell() % 0x10)):
					dst.write(b'\x00')
		else:
			#有些文件在这里后面还会跟一些非0字节，全部输出，但是文件又是8字节对齐的，
			#那就输出完再对齐，虽然这样做对于某些没有非0字节而是直接对齐的文件就会是再次对齐一次，
			#不过应该没有影响，毕竟只是多了些0而已
			dst.write(src.read())
			if (dst.tell() % 8) != 0:
				for i in range(0,8 - (dst.tell() % 8)):
					dst.write(b'\x00')

if '__main__' == __name__:
	main()