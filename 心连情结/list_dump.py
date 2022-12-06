# -*- coding: utf-8 -*-
import os
import sys
import io
import csv

def main():
	src = open('org.txt','r',encoding='utf16')
	str_lst = []
	c = src.read(1)
	while c:
		str_lst.append('%02x%02x\n'%(c.encode('utf16')[3],c.encode('utf16')[2]))
		c = src.read(1)
	dst = open('FontDat_list.txt','w')
	dst.write('# charmap table: %d\n'%(len(str_lst)))
	dst.write('# -----------------------------\n')
	for row in str_lst:
		dst.write(row)
main()