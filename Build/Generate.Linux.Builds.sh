../Binaries/linux_x64/microbuild generate make Config/Workspace.ini 
../Binaries/linux_x64/microbuild build Config/Workspace.ini -c=Shipping -p=x64
../Binaries/linux_x64/microbuild clean Config/Workspace.ini 
../Binaries/linux_x64/microbuild generate make Config/Workspace.ini 
../Binaries/linux_x64/microbuild build Config/Workspace.ini -c=Shipping -p=x86
cp ../Binaries/MicroBuild_Linux_x64_Shipping linux_x64/microbuild
cp ../Binaries/MicroBuild_Linux_x86_Shipping linux_x86/microbuild
git add ../Binaries/linux_x64/microbuild
git add ../Binaries/linux_x86/microbuild

