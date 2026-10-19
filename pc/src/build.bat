@echo off
chcp 65001 >nul
cd /d "%~dp0"
windres app.rc -o app.res.o || goto :err
g++ -O2 -std=c++17 -municode -mwindows -static -o SJRC_AutoReader.exe main.cpp app.res.o ^
    -lwinhttp -lgdiplus -lcomctl32 -lole32 -luuid -lgdi32 -luser32 -lshell32 -lws2_32 || goto :err
echo.
echo   build ok: SJRC_AutoReader.exe
exit /b 0
:err
echo.
echo   build failed
exit /b 1
