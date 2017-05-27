mkdir ProjectFiles
cd ProjectFiles

../Premake/premake5.macos gmake --file=../Workspace.premake
make config=shipping_x64 --file=Makefile

OUT=$?
if [ $OUT -neq 0 ];then
	cd ..
	echo "make returned exit code $OUT, aborting ..."
	exit 1
fi

../Premake/premake5.macos gmake --file=../Workspace.premake
make config=shipping_x86 --file=Makefile

OUT=$?
if [ $OUT -neq 0 ];then
	cd ..
	echo "make returned exit code $OUT, aborting ..."
	exit 1
fi

cd ..
