@echo off
chcp 65001 >nul
title magicIoTm - Configurator
cd /d "%~dp0"
echo Starting magicIoTm...
python app.py
pause
