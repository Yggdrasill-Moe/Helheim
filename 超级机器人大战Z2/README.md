# Helheim-超级机器人大战Z2
用于处理Z2再世篇相关类型的游戏
## [Note]
#### [封包]
除了EVENTBIN.BIN有自带单独索引文件外，其他封包文件的索引都在EBOOT.BIN

没啥好办法，找全吧。
#### [字库]
在EBOOT.BIN，0x1E6A74-0x32B274，tile：32*8，长：32*16，高：8*(63+1)，4bpp

编码：889F-9872
#### [文本]
EVENTBIN.BIN：剧情文本和胜利条件文本

OP.BIN：OP中的人名，和作品名图片

SRVC.BIN：战斗对话

DATA000.BIN：开头文本

DATA014.BIN：剧情简介相关

DATA015.BIN：MAP-DATA信息

DATA020.BIN：人物文本

DATA021.BIN：机体文本

EBOOT.BIN：系统文本，存档文本，甚至有图片，没有配转码文件，所以存档文本直接存的UTF-8
#### [图片]
大部分需要修图的都在Z2DATA.BIN、VEFF2DX.BIN、DATA013.BIN，大部分图片是GIM封包，有些是自制格式，简单的图片数据+调色板数据组合
#### [压缩算法]
LZ编码系列，有点意思，但是原版是有DP的，我自己没写带DP的，有空再完善吧
#### [无限狱mod]
需要解包mod的EVENTBIN.BIN，然后配合mod的EBOOT.BIN（命名为WBOOT.BIN）与基础未导入文本的EBOOT.BIN（命名为BOOT.BIN，已附带）
## [New]
ver 1.0

全部完成，同时增加无限狱与BGM修改相关程序

ver 0.7

可解决Z2再世篇的大部分内容汉化