#!/bin/sh
mkdir bin
for src in $(ls *.c)
do
	name=${src%.c}
	make "TARG=$name"
	rm ${name}.o
	mv $name bin
done
