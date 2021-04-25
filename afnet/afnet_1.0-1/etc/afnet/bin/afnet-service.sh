#!/bin/bash
source `dirname $0`/../afnet.conf
IFS=$'\n'

# trap ctrl-c and call ctrl_c()
trap ctrl_c INT

function ctrl_c() {
  echo "** Trapped CTRL-C"
  for i in {1..5}
  do
    eval "$AFHOME/$AF_BIN_STOP"
  done
  exit 1
}

running="1"
rm -f $AFHOME/$AF_TEXT_SHUTDOWN
rm -f $AFHOME/$AF_TEXT_RUNNING
touch $AFHOME/$AF_TEXT_RUNNING
echo "$0 starting loop"
while [ $running -eq 1 ]
do
  # --------------------------------
  for line in `diff -b -B $AFHOME/$AF_TEXT_START $AFHOME/$AF_TEXT_RUNNING | grep -v '#' | grep '^<' | sed 's/^. //g'`
  do
    grep "^${line}$" $AFHOME/$AF_TEXT_STOP > /dev/null 2>&1
    RESULT=$?
    if [ $RESULT -eq 1 ]; then
      echo "Starting FTP $line"
      eval "$AFHOME/$AF_BIN_START $line"
      RESULT=$?
      if [ $RESULT -eq 0 ]; then
        echo $line >> $AFHOME/$AF_TEXT_RUNNING
      fi
    fi
  done

  # --------------------------------
  for line in `cat $AFHOME/$AF_TEXT_STOP | grep -v '#'`
  do
    grep "^${line}$" $AFHOME/$AF_TEXT_RUNNING > /dev/null 2>&1
    RESULT=$?
    if [ $RESULT -eq 0 ]; then
      echo "Stopping FTP $line"
      eval "$AFHOME/$AF_BIN_STOP $line"
      RESULT=$?
      if [ $RESULT -eq 0 ]; then
        sed -i "/${line}/d" $AFHOME/$AF_TEXT_RUNNING
      fi
    fi
  done
  sleep 1
  if [ -f $AFHOME/$AF_TEXT_SHUTDOWN ]; then
    running=0
  fi
  sleep 1
done

echo "$0 stopping normally"
for line in `cat $AFHOME/$AF_TEXT_START`
do
  eval "$AFHOME/$AF_BIN_STOP"
done
echo "$0 stopping"
