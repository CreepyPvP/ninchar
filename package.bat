@echo off

del game.exe >nul
cd build
msbuild /nologo /p:Configuration=Release /v:q game.sln

move Release\game.exe ..\game.exe >nul
cd ..
REM tar.exe -a -c -f release.zip game.exe audio shader assets
REM tar.exe -a -c -f release.zip game.exe shader
