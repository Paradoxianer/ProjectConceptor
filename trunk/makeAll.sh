#!/bin/sh
echo $0
cd $1
for File in *
do
	if  [ -d   "$File" ];  then
		cd "$File"
		makeAll.sh $1"$File"
		pwd
		make clean
		make
		cd ..
	fi
# later automatically make a symbolic link to	cd obj.x86
done
