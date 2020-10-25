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
    echo "e.g. answers are: host: amigablack, amigawood, directory: RAM: SYS: "
    myask aAMIGA_DIR "DIRECTORY?" ""
    export aAMIGA_DIR2=`echo $aAMIGA_DIR | sed 's#/#_#g' | sed 's#:##g'`
    myask aAMIGA_HOST "AMIGA HOSTNAME?" "amigablack"
    myask aAMIGA_LOGIN "FTP LOGIN?" "`whoami`:`whoami`"
    myask aUNIX_TOPDIR "Local storage:" "/home/`whoami`/network/${aAMIGA_HOST}/"
    ping -c2 ${aAMIGA_HOST}
    SUCCESS=$?
    if [ $SUCCESS -eq 0 ]
    then
        echo "${aAMIGA_HOST} has replied"
    else
        echo "${aAMIGA_HOST} didn't reply"
        exit 1
    fi
    echo -e "\n${HIGHLIGHT}Starting CURLFTPS for ${aUNIX_TOPDIR}${NOHIGHLIGHT}"
    rm nohup.out
    echo "nohup curlftpfs -o nonempty -s ftp://${aAMIGA_LOGIN}@${aAMIGA_HOST}/${aAMIGA_DIR} ${aUNIX_TOPDIR}/${aAMIGA_DIR2}"
    mkdir -p ${aUNIX_TOPDIR}/${aAMIGA_DIR2}
    nohup curlftpfs -o nonempty -s ftp://${aAMIGA_LOGIN}@${aAMIGA_HOST}/${aAMIGA_DIR} ${aUNIX_TOPDIR}/${aAMIGA_DIR2}
    sleep 2
    cat nohup.out
    mylistcurl
    echo -e "${HIGHLIGHT}`ls -latrd ${aUNIX_TOPDIR}/${aAMIGA_DIR2}`${NOHIGHLIGHT}"
    ls -latrd ${aUNIX_TOPDIR}/${aAMIGA_DIR2}/*
    echo -e "\n${HIGHLIGHT}cd ${aUNIX_TOPDIR}/${aAMIGA_DIR2}${NOHIGHLIGHT}"
    echo -e "\n${HIGHLIGHT}source ~/.${aAMIGA_HOST}${NOHIGHLIGHT}"
    echo "cd ${aUNIX_TOPDIR}/${aAMIGA_DIR2}" > ~/.${aAMIGA_HOST}
}

# ----------------------------------------------------- #
# myrsync 
# ----------------------------------------------------- #
function myrsync {
    local PROMPT=$1
    myfirstcurl
    myask aUNIX_BACKUP "Where to backup to" "/home/`whoami`/backup_new/${FIRST_HOST}/"
    mkdir -p $aUNIX_BACKUP
    myask aUNIX_ARCHIVE "Where to archive to" "/home/`whoami`/archive_new/${FIRST_HOST}/"
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
    local aKILL=1
    sed -n ${aKILL}p $PSLIST > $PSLIST1
    cat $PSLIST1 | awk '{print $2}'
    kill -INT `cat $PSLIST1 | awk '{print $2}'`
    sleep 2
    umount `cat /tmp/pslist1 | sed 's/.* //g'`
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
    echo $FIRST_PS
    echo $FIRST_MOUNT
    echo $FIRST_DIR
    echo $FIRST_HOST
}

# ----------------------------------------------------- #
# main
# ----------------------------------------------------- #
if [[ $# -eq 0 ]]
then
    mylistcurl
fi
while [[ $# -gt 0 ]]
do
    key="$1"
    case $key in
        -n|--new)
        mycurl
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
