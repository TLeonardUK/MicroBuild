#!/bin/bash 

# ------------------------------------------------------------------------------
# Builds a MicroBuild binary for windows.
# ------------------------------------------------------------------------------

BuildDir="$(pwd)"
BinariesDir=$BuildDir/../Binaries
BootstrapDir=$BuildDir/Bootstrap
VersionNumber=$MB_BUILD_VERSION

if [ "$VersionNumber" == "" ]; then VersionNumber="99.99"; fi

echo BuildDir $BuildDir
echo BinariesDir: $BinariesDir
echo BootstrapDir: $BootstrapDir
echo VersionNumber: $VersionNumber

# ------------------------------------------------------------------------------
# Generate a bootstrap executable which we will use to build our main binary.
# ------------------------------------------------------------------------------
echo Building bootstrap ...
cd $BootstrapDir
./Build.Linux.sh
if [ $? -neq 0 ]; then ErrorHandler; fi
cd $BuildDir

# ------------------------------------------------------------------------------
# Call the bootstrap to build our project files.
# ------------------------------------------------------------------------------
echo Building binary project files ...
$BootstrapDir/ProjectFiles/MicroBuild/Shipping_x64/bin/MicroBuild Generate make Config/Workspace.ini 
if [ $? -neq 0 ]; then ErrorHandler; fi

# ------------------------------------------------------------------------------
# Call the bootstrap to build our final binary.
# ------------------------------------------------------------------------------
echo Building x64 binary ...
$BootstrapDir/ProjectFiles/MicroBuild/Shipping_x64/bin/MicroBuild Build Config/Workspace.ini MicroBuild -c=Shipping -p=x64 --set=MB_VERSION=$VersionNumber --silent
if [ $? -neq 0 ]; then ErrorHandler; fi

echo Building x86 binary ...
$BootstrapDir/ProjectFiles/MicroBuild/Shipping_x32/bin/MicroBuild Build Config/Workspace.ini MicroBuild -c=Shipping -p=x86 --set=MB_VERSION=$VersionNumber --silent
if [ $? -neq 0 ]; then ErrorHandler; fi

# ------------------------------------------------------------------------------
# Error handling / Exiting.
# ------------------------------------------------------------------------------
exit 0

function ErrorHandler {
	echo Failed to build MicroBuild. Build command finished with exit code %ERRORLEVEL%, aborting ...
	cd $BuildDir
	exit 1
}
