@echo off
setlocal enabledelayedexpansion
for /D %%s in (*.cpk_unpacked) do (
set name=%%s
cpkmakec.exe !name! !name:~0,16! -align=2048 -code=SJIS -mode=FULL
)
pause