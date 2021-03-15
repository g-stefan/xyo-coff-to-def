@echo off
rem Public domain
rem http://unlicense.org/
rem Created by Grigore Stefan <g_stefan@yahoo.com>

echo -^> sign xyo-coff-to-def

pushd output
for /r %%i in (*.exe) do call grigore-stefan.sign "XYO COFF to DEF" "%%i"
popd
