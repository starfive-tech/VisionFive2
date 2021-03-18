#!/bin/bash
cc=gcc
lib=vdec

#
if [ "x$1" == 'x' ];then
	#path=`pwd`
	path="."
else
	path=$1
fi

#
dirs=$path
for dir in $path/*;do
	if [ -d $dir ];then
		dirs=$dirs" "$dir
	fi
done
echo $dirs


#
for inc in $dirs;do
	INC_LD="$INC_LD  -I$inc"
done
echo $INC_LD

for dir in $dirs;do
	for file in $dir/*; do
		if [ ${file##*.} = 'cpp' ] || [ ${file##*.} = 'c' ];then
#			$cc $file $INC_LD  -o ${file%.*}.o  -c -fPIC
			obj=$obj" "${file%.*}.o
		fi

	done
done
echo $obj
#$cc -shared -fPIC -o lib${lib}.so $obj

#rm -rf $obj
