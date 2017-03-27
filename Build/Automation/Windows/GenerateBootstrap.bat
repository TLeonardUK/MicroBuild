cd ../../../Build/Bootstrap/
Build.Windows.bat
cd ../../Build/Automation/Windows
cp ..\..\..\Build\Bootstrap\ProjectFiles\MicroBuild\Shipping_x64\bin\microbuild.exe ..\..\..\Binaries\Windows\Shipping_x64\microbuild.bootstrap.exe 
..\..\..\Binaries\Windows\Shipping_x64\microbuild.bootstrap.exe generate vs2015 ../../Config/Workspace.ini

