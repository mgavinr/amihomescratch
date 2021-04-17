#!/bin/bash
OUTDIR=/home/gavinr/network.backup_new
HOSTNAME=amigablack
DIRNAME=gavinr
RSYNCO="-zavh" # compress archive 
HIGHLIGHT="\033[1;33m"
NOHIGHLIGHT="\033[0m"

function backup {
  #EXCLUDE_FILE=`basename $1`.exclude.txt
  EXCLUDE_FILE="${HOSTNAME}.exclude.txt"
  rsync $RSYNCO --size-only --stats --exclude-from="$EXCLUDE_FILE" --progress $1 ${OUTDIR}
}

for dir in `ls -C1d $HOSTNAME/* | grep -v Apps`
do
  echo " "
  echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>"
  echo -e "${HIGHLIGHT}Backup directory ${dir}${NOHIGHLIGHT} to ${OUTDIR}"
  backup $dir
done
