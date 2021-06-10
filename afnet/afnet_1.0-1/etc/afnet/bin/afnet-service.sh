#!/bin/bash
set -a
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

function check_sleep() {
  for i in {0..60}
  do
    echo "Sleep $i/60 .."
    sleep 1
    if [ -f $AFHOME/$AF_TEXT_SHUTDOWN ]; then
      break
    fi
  done
}

running="1"
rm -f $AFHOME/$AF_TEXT_SHUTDOWN
rm -f $AFHOME/$AF_TEXT_RUNNING
touch $AFHOME/$AF_TEXT_RUNNING
echo "$0 starting loop"
attempt=0
while [ $running -eq 1 ]
do
  # --------------------------------
  lineno=0
  for line in `diff -b -B $AFHOME/$AF_TEXT_START $AFHOME/$AF_TEXT_RUNNING | grep -v '#' | grep '^<' | sed 's/^. //g'`
  do
    lineno=$((lineno+1))
    grep "^${line}$" $AFHOME/$AF_TEXT_STOP > /dev/null 2>&1
    RESULT=$?
    if [ $RESULT -eq 1 ]; then
      attempt=$((attempt+1))
      echo " "
      echo "___________________________________________ SERVICE LOOP START FILE LINE $lineno attempt $attempt"
      echo "Starting FTP $line"
      eval "$AFHOME/$AF_BIN_START $line"
      RESULT=$?
      if [ $RESULT -eq 0 ]; then
        echo "___________________________________________ SERVICE LOOP START FILE LINE $lineno attempt $attempt=RUNNING"
        echo $line >> $AFHOME/$AF_TEXT_RUNNING
      else
        echo "___________________________________________ SERVICE LOOP START FILE LINE $lineno attempt $attempt=FAILED"
      fi
      if [ -f $AFHOME/$AF_TEXT_SHUTDOWN ]; then
        break
      fi
    fi
  done

  # --------------------------------
  lineno=0
  for line in `cat $AFHOME/$AF_TEXT_STOP | grep -v '#'`
  do
    lineno=$((lineno+1))
    grep "^${line}$" $AFHOME/$AF_TEXT_RUNNING > /dev/null 2>&1
    RESULT=$?
    if [ $RESULT -eq 0 ]; then
      echo " "
      echo "___________________________________________ SERVICE LOOP STOP FILE LINE $lineno"
      echo "Stopping FTP $line"
      eval "$AFHOME/$AF_BIN_STOP $line"
      RESULT=$?
      if [ $RESULT -eq 0 ]; then
        sed -i "/${line}/d" $AFHOME/$AF_TEXT_RUNNING
      fi
    fi
  done
  sleep 1
  check_sleep
  if [ -f $AFHOME/$AF_TEXT_SHUTDOWN ]; then
    running=0
  fi
done

echo " "
echo "___________________________________________ SERVICE LOOP"
echo "$0 stopping normally"
for line in `cat $AFHOME/$AF_TEXT_START`
do
  eval "$AFHOME/$AF_BIN_STOP"
done
echo " "
echo "___________________________________________ SERVICE LOOP"
echo "$0 stopping"
