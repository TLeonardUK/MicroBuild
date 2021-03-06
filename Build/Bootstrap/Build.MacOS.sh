#!/bin/bash 

# ------------------------------------------------------------------------------
# Builds a MicroBuild binary for linux using our premake bootstreap.
# ------------------------------------------------------------------------------

BootstrapDir="$(pwd)"
ProjectFilesDir=$BootstrapDir/ProjectFiles
PremakeDir=$BootstrapDir/Premake

function MakeErrorHandler {
	echo Failed to build bootstrap. make finished with failure exit code, aborting ...
	cd $BootstrapDir
	exit 1
}

# ------------------------------------------------------------------------------
# Build the visual studio project files.
# ------------------------------------------------------------------------------
echo Generating bootstrap project files ...
mkdir $ProjectFilesDir
cd $ProjectFilesDir
$PremakeDir/premake5.macos gmake --file=../Workspace.premake 
if [ $? -ne 0 ]; then MakeErrorHandler; fi

# ------------------------------------------------------------------------------
# Build the project files directly using msbuild.
# ------------------------------------------------------------------------------
echo Building x64 bootstrap binary ...
make clean --file=Makefile
if [ $? -ne 0 ]; then MakeErrorHandler; fi
make config=shipping_x64 --file=Makefile
if [ $? -ne 0 ]; then MakeErrorHandler; fi

echo Building x86 bootstrap binary ...
make clean --file=Makefile
if [ $? -ne 0 ]; then MakeErrorHandler; fi
make config=shipping_x32 --file=Makefile
if [ $? -ne 0 ]; then MakeErrorHandler; fi

# ------------------------------------------------------------------------------
# Error hanlding / Exiting.
# ------------------------------------------------------------------------------
cd $BootstrapDir
exit 0