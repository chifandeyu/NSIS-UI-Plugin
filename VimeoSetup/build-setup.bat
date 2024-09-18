@echo off
set NSIS_DIR=D:\tools\nsis-3.10
python.exe ..\NsisScriptGenerate.py "%~dp0vimeo-template.nsi" "%~dp0App"
"%NSIS_DIR%\makensisw.exe" /DPRODUCT_VERSION=1.1.0.80 "%~dp0\vimeo.nsi"

PAUSE