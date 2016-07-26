cd ../Build/Bootstrap
./GenerateProject.Linux.sh
cd ./ProjectFiles
make
cd ../..
../Binaries/microbuild_linux_debug_x86_64 generate make Config/Workspace.ini

