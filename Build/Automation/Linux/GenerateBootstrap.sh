#!/bin/bash
set -e

cd ../../../Build/Bootstrap/
./Build.Linux.sh
cd ../../Build/Automation/Linux
cp ../../../Build/Bootstrap/ProjectFiles/MicroBuild/Shipping_x64/bin/MicroBuild ../../../Binaries/Linux/Shipping_x64/MicroBuild.bootstrap 
../../../Binaries/Linux/Shipping_x64/MicroBuild.bootstrap generate make ../../Config/Workspace.ini

