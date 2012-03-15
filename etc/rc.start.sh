#!/bin/sh

# Variables
FSTAB=/etc/fstab
HOSTNAME=skua
KEYMAP=de

# Functions
deletetmp() {
  echo Cleaning temporary files
  rm -rf /tmp/*
  rm -rf /var/run/*
}

hostname() {
  echo Setting hostname $HOSTNAME
  echo $HOSTNAME > /proc/sys/kernel/hostname
}

loadkeys() {
  echo Loading keymap $KEYMAP
  keymap /share/keymaps/$KEYMAP.map
}

loglevel() {
  echo $@ > /proc/sys/kernel/printk
}

mountall() {
  echo Mounting $FSTAB entries
  grep -v -e '^#' -e 'noauto' $FSTAB | while read line
  do
    read -r dev targ type opts <<EOF
$line
EOF
    if [ $targ = / ]; then
      opts=$opts,remount
    elif [ $targ = swap ]; then
      swapon $dev
      continue
    fi
    mount -o $opts -t $type $dev $targ
  done
}

mountpseudo() {
	echo Mounting pseudo filesystems
	mount -o nosuid,noexec,nodev -t proc proc /proc
	mount -o nosuid,noexec,nodev -t sysfs sys /sys
	mount -o mode=0755,nosuid -t devtmpfs dev /dev
}

run() {
  echo Starting $1
  eval "$@"
}

# Sequence
echo Initializing system...
mountpseudo
loglevel 3 4 1 3
mountall
deletetmp
hostname
loadkeys
run syslogd
#run uevd
#run acpid 
