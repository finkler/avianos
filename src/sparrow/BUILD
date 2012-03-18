#!/bin/sh
mkdir bin 2>/dev/null
for src in $(ls *.c)
do
	name=${src%.c}
	make TARG=$name
	rm ${name}.o
	mv $name bin
done
for src in $(ls *.sh)
do
	name=${src%.sh}
	cp $src bin/$name
  chmod +x bin/$name
done
