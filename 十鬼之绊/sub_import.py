import os
import sys
import struct
import io

def int2byte(num):
	return struct.pack('L',num)

def byte2int(byte):
	return struct.unpack('L',byte)[0]

def walk(adr):
	mylist=[]
	for root,dirs,files in os.walk(adr):
		for name in files:
			adrlist=os.path.join(root, name)
			mylist.append(adrlist)
	return mylist

def main():
	fn_list = walk('sub_bin')
	tbl = open('tbl.txt','r',encoding='utf16')
	dicts = {}
	for rows in tbl:
		row = rows.rstrip('\r\n').split('=')
		if len(rows.rstrip('\r\n').split('=')) == 3:
			dicts['=']=int(row[0],16)
		else:
			dicts[row[1]]=int(row[0],16)
	for fn in fn_list:
		if fn[-4:] == '.txt':
			continue
		print(fn)
		src = open(fn,'rb')
		data = src.read()
		dst = open(fn.replace('sub_bin', 'sub_out'),'wb')
		dst.write(data)
		txt = open(fn.replace('sub_bin','sub_cn') + '.txt','r',encoding='utf16')
		str_list = txt.readlines()
		i = 0
		while i < len(str_list):
			if str_list[i][:2] == 'No':
				text_off = int(str_list[i][14:22],16)
				text_olen = int(str_list[i][27:-2])
				i += 2
				text = str_list[i].rstrip('\r\n')
				text_bin = b''
				for ch in text:
					try:
						if dicts[ch] > 0xFF:
							text_bin += struct.pack('>H',dicts[ch])
						else:
							text_bin += struct.pack('B',dicts[ch])
					except Exception as inst:
						print(text)
				text_nlen = len(text_bin)
				dst.seek(text_off,os.SEEK_SET)
				if text_olen >= text_nlen:
					dst.write(text_bin)
					for j in range(0,text_olen - text_nlen):
						dst.write(b'\x20')
				else:
					if data[text_off + text_nlen + 2] != 0xFF:
						print(hex(text_off),data[text_off + text_olen:text_off + text_nlen + 2])
						print(text)
					else:
						dst.write(text_bin)
				dst.write(b'\x00')
			i += 1

main()