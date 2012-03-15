#!/bin/sh

mkdir bin
for src in $(ls *.c)
do
  name=${src%.c}
  make TARG=$name
  mv $name bin/
done

for src in $(ls *.sh)
do
  name=${src%.sh}
  cp $src bin/$name
  chmod +x bin/$name
done

make clean