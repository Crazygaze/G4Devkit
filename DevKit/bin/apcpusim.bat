@echo off
CALL :normalise "%~dp0\..\.."
start "" "%RootDir%\DevKit\bin\APCPUSim.exe" workspace="%RootDir%\samples\apcpuos\apcpuos.apcpuwsp" servermanager=127.0.0.1:28000 opensimulator %*

:normalise
SET "RootDir=%~f1"
GOTO :EOF