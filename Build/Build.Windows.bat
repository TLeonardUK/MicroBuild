@echo off
REM ------------------------------------------------------------------------------
REM Builds a MicroBuild binary for windows.
REM ------------------------------------------------------------------------------

set BuildDir=%CD%
set BinariesDir=%BuildDir%\..\Binaries
set BootstrapDir=%BuildDir%\Bootstrap

REM ------------------------------------------------------------------------------
REM Generate a bootstrap executable which we will use to build our main binary.
REM ------------------------------------------------------------------------------
echo Building bootstrap ...
cd %BootstrapDir%
CALL Build.Windows.bat
if %ERRORLEVEL% NEQ 0 GOTO ErrorHandler
cd %BuildDir%

REM ------------------------------------------------------------------------------
REM Call the bootstrap to build our project files.
REM ------------------------------------------------------------------------------
echo Building binary project files ...
CALL %BootstrapDir%\ProjectFiles\MicroBuild\Shipping_x64\bin\MicroBuild.exe Generate vs2017 Config\Workspace.ini 
if %ERRORLEVEL% NEQ 0 GOTO ErrorHandler

REM ------------------------------------------------------------------------------
REM Call the bootstrap to build our final binary.
REM ------------------------------------------------------------------------------
echo Building x64 binary ...
CALL %BootstrapDir%\ProjectFiles\MicroBuild\Shipping_x64\bin\MicroBuild.exe Build Config\Workspace.ini MicroBuild -c=Shipping -p=x64 --silent
if %ERRORLEVEL% NEQ 0 GOTO ErrorHandler

echo Building x86 binary ...
CALL %BootstrapDir%\ProjectFiles\MicroBuild\Shipping_x32\bin\MicroBuild.exe Build Config\Workspace.ini MicroBuild -c=Shipping -p=x86 --silent
if %ERRORLEVEL% NEQ 0 GOTO ErrorHandler

REM ------------------------------------------------------------------------------
REM Error handling / Exiting.
REM ------------------------------------------------------------------------------
exit /B 1
goto Finished

:ErrorHandler
echo Failed to build MicroBuild. Build command finished with exit code %ERRORLEVEL%, aborting ...
cd %BuildDir%
exit /B %ERRORLEVEL%
goto Finished

:Finished