@echo off
chcp 65001 >nul
title magicIoTm - Restart
cd /d "%~dp0"
echo Restarting magicIoTm...

rem Stop the previous server instance: kills its python (from this folder),
rem closes the old server window and waits until port 5005 is released.
call stop.bat

rem Extra safety delay before starting the fresh instance.
timeout /t 2 >nul

rem Start a new server window (same as before).
start "" "%~dp0run.bat"