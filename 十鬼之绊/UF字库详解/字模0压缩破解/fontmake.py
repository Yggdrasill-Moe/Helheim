#!/usr/bin/env python
#coding: utf-8

import sys


inputFilename = sys.argv[1]
outputFilename = sys.argv[2]

fp_output = open(outputFilename, 'wb')
fp_input = open(inputFilename, 'rb')
x = 0


apple = fp_input.read()
while x*0x109 <= len(apple):
    fp_output.write('\x80\x00' + 
                    apple[0x109 * x:0x109 * (x+1)])
    x += 1

fp_output.close()
fp_input.close()