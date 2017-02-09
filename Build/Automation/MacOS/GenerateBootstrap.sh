#!/bin/bash
set -e

cd ../../../Build/Bootstrap/
./Build.MacOS.sh
cd ../../Build/Automation/MacOS
cp ../../../Build/Bootstrap/ProjectFiles/MicroBuild/Shipping_x64/bin/MicroBuild ../../../Binaries/MacOS/Shipping_x64/MicroBuild.bootstrap 
../../../Binaries/MacOS/Shipping_x64/MicroBuild.bootstrap generate xcode ../../Config/Workspace.ini

