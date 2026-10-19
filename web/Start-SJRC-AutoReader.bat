@echo off
setlocal
cd /d "%~dp0"

where py >nul 2>nul
if not errorlevel 1 (
    py -3 -c "import sys" >nul 2>nul
    if not errorlevel 1 goto use_launcher
)

where python >nul 2>nul
if not errorlevel 1 (
    python -c "import sys" >nul 2>nul
    if not errorlevel 1 goto use_python
)

echo Python 3 is required to start the local web server.
echo Install Python 3, then double-click this file again.
pause
exit /b 1

:use_launcher
py -3 "%~dp0serve_web.py"
exit /b %errorlevel%

:use_python
python "%~dp0serve_web.py"
exit /b %errorlevel%
