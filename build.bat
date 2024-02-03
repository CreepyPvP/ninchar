@echo off
REM del game.exe >nul
cd build
msbuild /nologo /v:q game.sln
move Debug\game.exe ..\game.exe >nul
