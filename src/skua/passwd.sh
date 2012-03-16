#!/bin/sh

SHADOW=/etc/shadow
SHADOW_TMP=$SHADOW-

user="root"
if [ $# -eq 1 ]; then
  user=$1
elif [ $# -gt 1 ]; then
  echo "usage: passwd [user]" 1>&2
  exit 1
fi

logname $user &>/dev/null
if [ $? -eq 1 ]; then
  echo "passwd: unknown user $user" 1>&2
  exit 1
fi

echo -n "new password: "
stty -echo
read -r pass1
stty echo
echo
echo -n "repeat password: "
stty -echo
read -r pass2
stty echo
echo
if [ $pass1 != $pass2 ]; then
  echo "password mismatch" 1>&2
  exit 1
fi

pass=$(echo -n $pass1 | sha1sum)
grep -v "^$user" $SHADOW > $SHADOW_TMP
echo $user:$pass::::::: >> $SHADOW_TMP
cat $SHADOW_TMP > $SHADOW
exit $?
