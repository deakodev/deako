@echo off

pushd ..
vendor\premake\windows\premake5.exe --file=premake5.lua vs2022
popd
pause