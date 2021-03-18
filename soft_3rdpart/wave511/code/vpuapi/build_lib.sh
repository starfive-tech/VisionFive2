#!/bin/bash
cc=gcc
lib=vdec

#find the path to build
if [ "x$1" == 'x' ];then
	path="."
else
	path=$1
fi

#find the dir
dirs=$path
for dir in $path/*;do
	if [ -d $dir ];then
		dirs=$dirs" "$dir
	fi
done
echo $dirs

#link path
for inc in $dirs;do
	INC_LD="$INC_LD  -I$inc"
done
echo $INC_LD

for dir in $dirs;do
	for file in $dir/*; do
		if [ ${file##*.} = 'cpp' ] || [ ${file##*.} = 'c' ];then
			$cc $file $INC_LD  -o ${file%.*}.o  -c -fPIC
			obj=$obj" "${file%.*}.o
		fi

	done
done
$cc -shared -fPIC -o lib${lib}.so $obj

rm -rf $obj
