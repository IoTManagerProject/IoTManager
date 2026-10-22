@echo off
chcp 65001 >nul
title magicIoTm - Stop
cd /d "%~dp0"
echo Stopping magicIoTm...
start /min powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0stop.ps1"
timeout /t 2 >nul
exit /b