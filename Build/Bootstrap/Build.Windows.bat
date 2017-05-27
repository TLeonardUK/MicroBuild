@echo off
REM ------------------------------------------------------------------------------
REM Builds a MicroBuild binary for windows using our premake bootstreap.
REM ------------------------------------------------------------------------------

set BootstrapDir=%CD%
set ProjectFilesDir=%BootstrapDir%\ProjectFiles
set PremakeDir=%BootstrapDir%\Premake

REM ------------------------------------------------------------------------------
REM Build the visual studio project files.
REM ------------------------------------------------------------------------------
echo Generating bootstrap project files ...
if not exist "%ProjectFilesDir%" (
	mkdir %ProjectFilesDir%
)
cd %ProjectFilesDir%
CALL %PremakeDir%\premake5.windows.exe vs2017 --file=..\Workspace.premake 
if %ERRORLEVEL% NEQ 0 GOTO ErrorHandler

REM ------------------------------------------------------------------------------
REM Attempt to get the visual studio environment variables available for us.
REM ------------------------------------------------------------------------------	

REM We only need to do this if we don't already have the envvars set, so just check
REM DevEnvDir which we know is defined by the vsvars files.
if not defined DevEnvDir (
	if exist "%VSINSTALLDIR%\VC\Auxiliary\Build\vcvarsall.bat" (
		CALL "%VSINSTALLDIR%\VC\Auxiliary\Build\vcvarsall.bat"
		if %ERRORLEVEL% EQU 0 GOTO BuildProjectFiles
	)
	if exist "%VS140COMNTOOLS%\vsvars64.bat" (
		CALL "%VS140COMNTOOLS%\vsvars64.bat"
		if %ERRORLEVEL% EQU 0 GOTO BuildProjectFiles
	)
	if exist "%VS120COMNTOOLS%\vsvars64.bat" (
		CALL "%VS120COMNTOOLS%\vsvars64.bat"
		if %ERRORLEVEL% EQU 0 GOTO BuildProjectFiles
	)
	if exist "%VS110COMNTOOLS%\vsvars64.bat" (
		CALL "%VS110COMNTOOLS%\vsvars64.bat"
		if %ERRORLEVEL% EQU 0 GOTO BuildProjectFiles
	)
	GOTO EnvVarsErrorHandler
)

REM ------------------------------------------------------------------------------
REM Build the project files directly using msbuild.
REM ------------------------------------------------------------------------------
:BuildProjectFiles
echo Building x64 bootstrap binary ...
CALL msbuild MicroBuild.sln /t:rebuild /p:Configuration=Shipping /p:Platform=x64
if %ERRORLEVEL% NEQ 0 GOTO MSBuildErrorHandler

echo Building x86 bootstrap binary ...
CALL msbuild MicroBuild.sln /t:rebuild /p:Configuration=Shipping /p:Platform=Win32
if %ERRORLEVEL% NEQ 0 GOTO MSBuildErrorHandler

REM ------------------------------------------------------------------------------
REM Error hanlding / Exiting.
REM ------------------------------------------------------------------------------
cd %BootstrapDir%
exit /B 0
goto Finished

:MSBuildErrorHandler
echo Failed to build bootstrap. msbuild finished with exit code %ERRORLEVEL%, aborting ...
cd %BootstrapDir%
exit /B %ERRORLEVEL%
goto Finished

:EnvVarsErrorHandler
echo Failed to find visual studio installation to build with, aborting ...
cd %BootstrapDir%
exit /B 1
goto Finished

:Finished