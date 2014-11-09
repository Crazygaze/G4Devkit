@echo off
CALL :normalise "%~dp0\..\..\samples\apcpuos\apcpuos.apcpuwsp"
start %~dp0\APCPUSim.exe workspace="%TempDir%" servermanager=127.0.0.1:28000 opensimulator %*

:normalise
SET "TempDir=%~f1"
GOTO :EOF