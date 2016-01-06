#!/bin/sh

usage() {
  echo "usage: dmesg [-n level]" 1>&2
  exit 1
}

while getopts ":n:" flags
do
  case $flags in
  n)
    if [ $OPTARG -lt 7 ] || [ $OPTARG -gt 0 ]; then
      echo $OPTARG > /proc/sys/kernel/printk
      exit 0
    fi
    echo "dmesg: invalid level $OPTARG" 1>&2
    exit 1;;
  *)
    usage;;
  esac
done
shift $((OPTIND-1))

if [ $# -gt 0 ]; then
  usage
fi
cat /var/log/kernel
exit $?
