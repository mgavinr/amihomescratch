#!/bin/bash
# ----------------------------------------------------- #
# History
# ----------------------------------------------------- #
# Changelog:
# Sat Oct 24 23:40:23 IST 2020
# I couldn't use the old script anymore hence this one
# Readme:
# This mounts an Amiga FTP server on unix filesystem
# and does some sortof backups if you want via rsync
# Author: Gavin
# ----------------------------------------------------- #
# Settings
# ----------------------------------------------------- #
MYKILLARG1=
MYKILLARG2=
HIGHLIGHT="\033[1;33m"
NOHIGHLIGHT="\033[0m"
MPSLIST=/tmp/mpslist
MPSLIST1=/tmp/mpslist1
PSLIST=/tmp/pslist
PSLIST1=/tmp/pslist1
# trap ctrl-c and call ctrl_c()
trap ctrl_c INT
function ctrl_c() {
    echo "** $* Trapped CTRL-C"
    exit 1
}

# ----------------------------------------------------- #
# myask <varname> "question question" <default>
# ----------------------------------------------------- #
function myask {
    local varname=$1
    local question=$2
    local default=$3
    echo -e -n "${HIGHLIGHT}$question [$default]:${NOHIGHLIGHT} "
    read -r -ei "$default" $varname
    eval printout=\$$varname
    echo "DEBUG: setting $varname=$printout"
}

# ----------------------------------------------------- #
# mycurl 
# ----------------------------------------------------- #
function mycurl {
    echo "e.g. blank ram: SYS: Apps: MyWork:"
    myask aAMIGA_DIR "DIRECTORY?" ""
    #export aAMIGA_DIR2=`echo $aAMIGA_DIR | sed 's#/#_#g' | sed 's#:##g'`
    export aAMIGA_DIR2=`echo $aAMIGA_DIR | sed 's#:##g'`
    echo "e.g. amigablack amigawood"
    myask aAMIGA_HOST "AMIGA HOSTNAME?" "amigablack"
    echo "e.g. `whoami`:`whoami`"
    myask aAMIGA_LOGIN "FTP LOGIN?" "`whoami`:`whoami`"
    export aAMIGA_LOGIN_USER=`echo $aAMIGA_LOGIN | sed 's/:.*$//g'`
    if [[ "$aAMIGA_DIR" == "" ]]; then
      myask aUNIX_TOPDIR "Local storage:" "${AFNETWORK}/${aAMIGA_HOST}/${aAMIGA_LOGIN_USER}"
      export aAMIGA_DIRT=${aAMIGA_LOGIN_USER}
    else
      myask aUNIX_TOPDIR "Local storage:" "${AFNETWORK}/${aAMIGA_HOST}/${aAMIGA_DIR2}"
      export aAMIGA_DIRT=${aAMIGA_DIR2}
    fi
    ping -c2 ${aAMIGA_HOST}
    SUCCESS=$?
    if [ $SUCCESS -eq 0 ]
    then
        echo "${aAMIGA_HOST} has replied"
    else
        echo "ERROR ${aAMIGA_HOST} didn't reply"
        echo "ERROR ${aAMIGA_HOST} didn't reply" >> $AF_TEXT_ERRORS
        cat $AF_TEXT_ERRORS | sort | uniq  >> ${AF_TEXT_ERRORS}.1
        mv ${AF_TEXT_ERRORS}.1 ${AF_TEXT_ERRORS}
    fi
    echo -e "\n${HIGHLIGHT}Starting CURLFTPS for ${aUNIX_TOPDIR}${NOHIGHLIGHT}"
    rm nohup.out
    echo "curlftpfs -o nonempty -s ftp://${aAMIGA_LOGIN}@${aAMIGA_HOST}/${aAMIGA_DIR} ${aUNIX_TOPDIR}"
    mkdir -p ${aUNIX_TOPDIR}
    #nohup curlftpfs -o nonempty -s ftp://${aAMIGA_LOGIN}@${aAMIGA_HOST}/${aAMIGA_DIR} ${aUNIX_TOPDIR}
    curlftpfs -o nonempty -s ftp://${aAMIGA_LOGIN}@${aAMIGA_HOST}/${aAMIGA_DIR} ${aUNIX_TOPDIR}
    SUCCESS=$?
    if [ $SUCCESS -eq 0 ]
    then
        mylistcurl
        echo -e "\n>>>>>>>>>>>>>>>>>>>>>>\nThe following files are now available here:"
        echo -e "${HIGHLIGHT}`ls -latrd ${aUNIX_TOPDIR}`${NOHIGHLIGHT}"
        cd ${aUNIX_TOPDIR}/ && ls -F
        echo -e "\n>>>>>>>>>>>>>>>>>>>>>>\nMOUNTED ${aAMIGA_DIRT} okay! .. See:\n${HIGHLIGHT}cd ${aUNIX_TOPDIR}/${NOHIGHLIGHT}"
      echo -e "\n>>>>>>>>>>>>>>>>>>>>>>\nThe following directories are mounted:\n`mount | grep curl | awk '{print $3}'`"
    else
        echo "ERROR: curl command failed, rerunning to get logs:"
        curlftpfs -o nonempty -s ftp://${aAMIGA_LOGIN}@${aAMIGA_HOST}/${aAMIGA_DIR} ${aUNIX_TOPDIR} > nohup.out 2>&1
        sleep 2
        echo "ERROR: curl command failed [`cat nohup.out | tr -d '\n'`]"
        echo "ERROR: curl command failed [`cat nohup.out | tr -d '\n'`]" >> $AF_TEXT_ERRORS
        cat $AF_TEXT_ERRORS | sort | uniq  >> ${AF_TEXT_ERRORS}.1
        mv ${AF_TEXT_ERRORS}.1 ${AF_TEXT_ERRORS}
    fi
}

# ----------------------------------------------------- #
# myrsync 
# ----------------------------------------------------- #
function myrsync {
    local PROMPT=$1
    myfirstcurl
    myask aUNIX_BACKUP "Where to backup to" "${AFNETWORK}.backup_new/${FIRST_HOST}/"
    mkdir -p $aUNIX_BACKUP
    myask aUNIX_ARCHIVE "Where to archive to" "${AFNETWORK}.backup_old/${FIRST_HOST}/"
    mkdir -p $aUNIX_ARCHIVE
    local RSYNC_DAY=`date | sed 's/ /-/g' | sed -e s'/\(^..........\).*$/\1/g'`
    local RSYNC_MONTH=`date | sed 's/ /-/g' | sed -e s'/^....\(...\).*$/\1/g'`
    local RSYNC_YEAR=`date | sed 's/ /-/g' | sed -e s'/.*-//g'`
    local RSYNC_LATEST="latest"
    local RSYNC_OPTS="--progress --human-readable --checksum -vr"
    local RSYNC_OPTS="--progress --human-readable --checksum -vr"
    local RSYNC_OPTS="-avAXEWSlHh --no-compress --info=progress2"
    # a archive, v verbose, A file access perms, X more As, E more As, W whole file no using the delta, l symlinks preservewd, H hardlinks, h human readable maybe add --fake-super remove sparse
    local RSYNC_OPTS="-avAXEWlHh --no-compress --info=progress2"
    local RSYNC_OPTS="-alrW"
    local RSYNC_OPTS="-alr"
    local RSYNC_DU=0   # want some more du info? use 1
    local TIME="time"  # want some more time info? use time
    local TIME=""
    local RSYNC_SKIP="\
apps.old\
AExplorer\
backup\
AmiSSL\
AWeb_APL\
Cinemaware\
"
    #sudo tshark -Q -i any -z io,stat,0,"SUM(frame.len)frame.len && ip.src == 192.168.0.20" &
    TOTAL=`ls -C1d ${FIRST_MOUNT}/* | wc -l`
    let "CTOTAL = 0"
    for f in `ls -C1d ${FIRST_MOUNT}/*`
    do
        basef=`basename $f`
        let "CTOTAL = CTOTAL + 1"
        SKIP=0
        for checkf in `echo $RSYNC_SKIP`
        do
            if [[ "$checkf" == "$basef" ]]; then
                SKIP=1
            fi
        done
        if [[ "$SKIP" != "1" ]]; then
            echo -e "`date` ${HIGHLIGHT} backup $CTOTAL/$TOTAL: ${NOHIGHLIGHT} $basef"
            echo "rsync $RSYNC_OPTS $f $aUNIX_BACKUP/${FIRST_DIR}"
            export STARTDU=$( du -sh $aUNIX_BACKUP/${FIRST_DIR}/`basename $f` 2>/dev/null)
            rsync $RSYNC_OPTS $f $aUNIX_BACKUP/${FIRST_DIR}/ > /dev/null 2>&1
            rsync $RSYNC_OPTS $f $aUNIX_BACKUP/${FIRST_DIR}/ > /dev/null 2>&1
            export ENDDU=$( du -sh $aUNIX_BACKUP/${FIRST_DIR}/`basename $f` 2>/dev/null)
            echo $ENDDU
            echo $STARTDU
            # backup the backups (asap)
            rsync -qvr $aUNIX_BACKUP $aUNIX_ARCHIVE/${RSYNC_YEAR}/ > /dev/null 2>&1
            rsync -qvr $aUNIX_BACKUP $aUNIX_ARCHIVE/${RSYNC_MONTH}/ > /dev/null 2>&1
            rsync -qvr $aUNIX_BACKUP $aUNIX_ARCHIVE/${RSYNC_DAY}/ > /dev/null 2>&1
        else
            echo -e "`date` ${HIGHLIGHT} skip $CTOTAL/$TOTAL: ${NOHIGHLIGHT} $basef"
        fi
    done
}

# ----------------------------------------------------- #
# mylistcurl
# ----------------------------------------------------- #
function mylistcurl {
    echo -e "${HIGHLIGHT}The following curls are running${NOHIGHLIGHT}"
    ps -ef | grep curlftpfs | grep nonempty
    ps -ef | grep curlftpfs | grep nonempty > $PSLIST
    echo -e "${HIGHLIGHT}The following mounts are running${NOHIGHLIGHT}"
    mount | grep curl
}

# ----------------------------------------------------- #
# mykillcurl
# ----------------------------------------------------- #
function mykillcurl {
    #myask aKILL "Which one will I stop" "1"
    if [[ "$MYKILLARG1" == "" ]]; then
      local aKILL=1
      sed -n ${aKILL}p $PSLIST > $PSLIST1
      cat $PSLIST1 | awk '{print $2}'
      export TV1=`cat $PSLIST1 | awk '{print $2}'`
      if [[ "$TV1" != "" ]]; then
        kill -INT `cat $PSLIST1 | head -1 | awk '{print $2}'`
        kill -9 `cat $PSLIST1 | head -1 | awk '{print $2}'`
        sleep 2
        umount `cat /tmp/pslist1 | head -1 | sed 's/.* //g'`
      fi
    else
      cat $PSLIST | grep $MYKILLARG1 | grep $MYKILLARG2 > $PSLIST1
      cat $PSLIST1 | awk '{print $2}'
      export TV1=`cat $PSLIST1 | awk '{print $2}'`
      if [[ "$TV1" != "" ]]; then
        kill -INT `cat $PSLIST1 | head -1 | awk '{print $2}'`
        kill -9 `cat $PSLIST1 | head -1 | awk '{print $2}'`
        sleep 2
        umount `cat /tmp/pslist1 | head -1 | sed 's/.* //g'`
      fi
    fi
}

function mykillcurlsudo {
    #myask aKILL "Which one will I stop" "1"
    local aKILL=1
    sed -n ${aKILL}p $PSLIST > $PSLIST1
    cat $PSLIST1 | awk '{print $2}'
    export TV1=`cat $PSLIST1 | awk '{print $2}'`
    if [[ "$TV1" != "" ]]; then
      kill -INT `cat $PSLIST1 | head -1 | awk '{print $2}'`
      kill -9 `cat $PSLIST1 | head -1 | awk '{print $2}'`
      sudo kill -9 `cat $PSLIST1 | head -1 | awk '{print $2}'`
      sleep 2
      umount `cat /tmp/pslist1 | head -1 | sed 's/.* //g'`
      sudo umount -f `cat /tmp/pslist1 | head -1 | sed 's/.* //g'`
    fi
}

# ----------------------------------------------------- #
# myfirstcurlps
# ----------------------------------------------------- #
function myfirstcurl {
    ps -ef | grep curlftpfs | grep nonempty > $PSLIST
    if [[ `wc -l $PSLIST | awk '{print $1}'` -eq 0 ]]
    then
        echo "Nothing running - no curlftpfs sessions found"
        exit 1
    fi
    sed -n 1p $PSLIST > $PSLIST1
    local AMIGAINFO=`cat $PSLIST1 | sed 's/.*ftp://g' | awk '{print $1}'`
    local UNIXINFO=`cat $PSLIST1 | sed 's/.*ftp://g' | awk '{print $2}'`
    export FIRST_PS=`cat $PSLIST1 | awk '{print $2}'`
    export FIRST_MOUNT=$UNIXINFO
    export FIRST_DIR=`basename $FIRST_MOUNT`
    export FIRST_HOST=`echo $AMIGAINFO | sed 's/.*@//g' | sed 's#/.*$##g'`
    echo " "
    echo "First:"
    echo "PS=$FIRST_PS"
    echo "MOUNT=$FIRST_MOUNT"
    echo "DIR=$FIRST_DIR"
    echo "HOST=$FIRST_HOST"
}

# ----------------------------------------------------- #
# main
# ----------------------------------------------------- #
if [[ $# -eq 0 ]]
then
    echo "No args supplied (try -n), status="
    mylistcurl
    myfirstcurl
fi
while [[ $# -gt 0 ]]
do
    key="$1"
    case $key in
        -n|--new)
        mycurl
        shift
        ;;
        --killarg)
        shift
        export MYKILLARG1="$1"
        shift
        export MYKILLARG2="$1"
        mylistcurl
        mykillcurl
        mylistcurl
        ;;
        --killsudo)
        mylistcurl
        mykillcurlsudo
        mylistcurl
        shift
        ;;
        -k|--kill)
        mylistcurl
        mykillcurl
        mylistcurl
        shift
        ;;
        -r|--rsync)
        myrsync normal
        shift
        ;;
        -p|--rsyncp)
        myrsync prompt
        shift
        ;;
        -h|--help)
        mylistcurl
        shift
        ;;
        *)
        mylistcurl
        shift
        ;;
    esac
done

echo " "
echo " "
