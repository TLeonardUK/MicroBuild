@echo off

set OriginalDir=%CD%
mkdir ProjectFIles
cd ProjectFiles

..\Premake\premake5.windows.exe vs2015 --file=..\Workspace.premake 
if %ERRORLEVEL% NEQ 0 GOTO ErrorHandler

CALL "%VS140COMNTOOLS%\vsvars32.bat"
if %ERRORLEVEL% EQU 0 GOTO GotVars
CALL "%VS120COMNTOOLS%\vsvars32.bat"
if %ERRORLEVEL% EQU 0 GOTO GotVars
CALL "%VS110COMNTOOLS%\vsvars32.bat"
if %ERRORLEVEL% EQU 0 GOTO GotVars

GOTO ErrorHandler

:GotVars
msbuild MicroBuild.sln /t:rebuild /p:Configuration=Release /p:Platform=x64
if %ERRORLEVEL% NEQ 0 GOTO ErrorHandler
msbuild MicroBuild.sln /t:rebuild /p:Configuration=Release /p:Platform=Win32
if %ERRORLEVEL% NEQ 0 GOTO ErrorHandler

cd %OriginalDir%
CMD /C EXIT 0

:ErrorHandler
cd %OriginalDir%
CMD /C EXIT %ERRORLEVEL%