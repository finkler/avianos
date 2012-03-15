#!/bin/sh

if [ $# -gt 0 ]; then
  echo "usage: batch" 1>&2
  exit 1
fi

at -q b -m now
exit $?