mkdir ProjectFiles
cd ProjectFiles

../Premake/premake5.linux gmake --file=../Workspace.premake
make --file=Makefile

cd ..
