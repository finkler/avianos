#!/bin/sh

if [ $# -gt 0 ]; then
  echo "usage: halt" 1>&2
  exit 1
fi
kill -s TERM 1
exit $?
