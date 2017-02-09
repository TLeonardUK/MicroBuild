mkdir ProjectFiles
cd ProjectFiles

../Premake/premake5.linux gmake --file=../Workspace.premake
make config=shipping_x64 --file=Makefile

cd ..
