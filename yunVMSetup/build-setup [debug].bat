@echo off
python.exe ..\NsisScriptGenerate.py "%~dp0yunVM-template.nsi" "%~dp0App"
"%NSIS_DIR%\makensisw.exe" /DDEBUG=1 "%~dp0\yunVM.nsi"

PAUSE