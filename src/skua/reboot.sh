#!/bin/sh

if [ $# -gt 0 ]; then
  echo "usage: reboot" 1>&2
  exit 1
fi
kill -s INT 1
exit $?
