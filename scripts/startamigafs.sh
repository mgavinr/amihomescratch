#!/bin/bash
# ------------------------------------------- #
# Changelog ================================= #
# startamigafs.sh Sun Apr 12 21:35:45 IST 2020 by Gavin Rogers
# Fri Apr 24 17:04:45 IST 2020 - change backup location
# Fri Apr 24 17:04:45 IST 2020 - change rsync options
# 
# ABOUT ===================================== #
# A bash script for starting curlftpfs to an amiga, optionally rsync files locally for backups.
# A symbolic link to the curlftpfs location is created in ~/machinename for the last ftp sesssion.
# Backups are stored in ~/network/machinename/.location
# The actual curlftpfs files are also there, but it's handier to use the symlink
# 
# If you dont see the contents of the amiga's remote directory it's likely curlftpfs has failed.
# You may want to run the commands without screen manually then to see why.
#
# REQUIREMENTS=============================== #
# Unix
# screen arp curlftpfs packages
# firewall open for port 21 20
# tested on ubuntu
#
# Amiga
# AmiTcp ftpd service running
# Unix compatible ls in c:ls and/or AmiTcp:bin/ls
#
# USAGE====================================== #
# ./startamigafs.sh -h
# ./startamigafs.sh -u username:password -m amigahostname -s RAM:
# ------------------------------------------- 1

# Fixed options
HIGHLIGHT="\033[1;33m"
NOHIGHLIGHT="\033[0m"
export HELP="Usage:\n  ./startamiga.sh -s RAM:\n  ./startamiga.sh -s SYS:\n  ./startamiga.sh -s SYS:\n  ./startamiga.sh --rsync --login username:password --machine amiga.host.name\n"
export SESSION=`whoami`
export SESSIONSET=0
export RSYNC=0
export DIR=""
export RSYNC_DAY=`date | sed 's/ /-/g' | sed -e s'/\(^..........\).*$/\1/g'`
export RSYNC_MONTH=`date | sed 's/ /-/g' | sed -e s'/^....\(...\).*$/\1/g'`
export RSYNC_YEAR=`date | sed 's/ /-/g' | sed -e s'/.*-//g'`
export RSYNC_LATEST="latest"

# Changeable options
echo -en "$0: ${HIGHLIGHT} 1. arp guessing machine: ${NOHIGHLIGHT}"
export MACHINE=`arp | grep amiga | awk '{print $1}'`
echo "found \"$MACHINE\""
export LOGIN=`whoami`:`whoami`
export SESSION_HOME=/home/`whoami`/network/${MACHINE}/
export RSYNC_OPTS="--progress --human-readable --checksum -vr"
export RSYNC_OPTS="--progress --human-readable --checksum -vr"
export RSYNC_OPTS="-avAXEWSlHh --no-compress --info=progress2"
# a archive, v verbose, A file access perms, X more As, E more As, W whole file no using the delta, l symlinks preservewd, H hardlinks, h human readable maybe add --fake-super remove sparse
export CURL_OPTS="-o nonempty"
export RSYNC_OPTS="-avAXEWlHh --no-compress --info=progress2"
export RSYNC_DU=0   # want some more du info? use 1
export TIME="time"  # want some more time info? use time
export TIME=""
export RSYNC_SKIP="\
apps.old\
AExplorer\
backup\
AmiSSL\
AWeb_APL\
Cinemaware\
"

# ------------------------------------------- #
# Parse Command line 
# ------------------------------------------- #
while [[ $# -gt 0 ]]
do
key="$1"
case $key in
    -l|--login)
    LOGIN="$2"
    shift # past argument
    shift # past value
    ;;
    -s|--session)
    SESSION="$2"
    SESSIONSET=1
    shift # past argument
    shift # past value
    ;;
    -m|--machine)
    MACHINE="$2"
    shift # past argument
    shift # past value
    ;;
    -r|--rsync)
    RSYNC=1
    shift # past argument
    ;;
    -h|--help)
    echo -e $HELP
    exit 0
    ;;
esac
done
export SESSION_HOME=/home/`whoami`/network/${MACHINE}/
SESSIONNAME=`echo $SESSION | sed 's/://g' | sed 's#/##g'`
if [ $SESSIONSET -gt 0 ]
then
    AMIGALOCATION=${SESSION}
else
    AMIGALOCATION=""
fi
SESSIONNAME=`echo $SESSIONNAME | tr "[:upper:]" "[:lower:]"`


# ------------------------------------------- #
# check
# ------------------------------------------- #
# ------------------------------------------- 2
[[ -z "$MACHINE" ]] && { echo "Amiga hostname is not found, MACHINE=" ; exit 1; }
echo -en "$0: ${HIGHLIGHT} 2. ping checking machine $MACHINE: ${NOHIGHLIGHT}"
ping -c2 $MACHINE 1>/dev/null 2>/dev/null
SUCCESS=$?
if [ $SUCCESS -eq 0 ]
then
  echo "$MACHINE has replied"
  export MACHINE_INFO=`arp $MACHINE | sed 's/[()]//g'`
else
  echo "$MACHINE didn't reply"
  export MACHINE_INFO=`arp $MACHINE | sed 's/[()]//g'`
fi

# ------------------------------------------- #
# start screen and curlftpfs
# ------------------------------------------- #
# ------------------------------------------- 3
# trap ctrl-c and call ctrl_c()
trap ctrl_c INT

function ctrl_c() {
    echo "** $* Trapped CTRL-C"
    exit 1
}
# cleanup
echo -en "$0: ${HIGHLIGHT} 3. cleanup last screen session: ${NOHIGHLIGHT} `date` ftp $MACHINE"
screen -ls ftp${MACHINE}$SESSIONNAME | grep ftp${MACHINE}$SESSIONNAME | awk '{print $1}' | xargs -I{} screen -XS {} kill
sudo umount -f ${SESSION_HOME}/ftp_$SESSIONNAME
sleep 1

# start
echo -en "$0: ${HIGHLIGHT} 3. starting screen session: ${NOHIGHLIGHT} `date` ftp $MACHINE"
echo "screen -d -m -S ftp${MACHINE}$SESSIONNAME bash -c 'bash'"
mkdir -p ${SESSION_HOME}/ftp_$SESSIONNAME
mkdir -p ${SESSION_HOME}/backup/ftp_$SESSIONNAME
mkdir -p ${SESSION_HOME}/archive/.ftp_$SESSIONNAME
export AMIGA=${SESSION_HOME}/ftp_$SESSIONNAME
echo "screen -d -m -S ftp${MACHINE}$SESSIONNAME bash -c 'bash'"
echo "screen -R ftp${MACHINE}$SESSIONNAME"
screen -d -m -S ftp${MACHINE}$SESSIONNAME bash -c 'bash'
screen -S ftp${MACHINE}${SESSIONNAME} -p 0 -X stuff "curlftpfs $CURL_OPTIONS -s ftp://${LOGIN}@${MACHINE}/${AMIGALOCATION} ${SESSION_HOME}/ftp_$SESSIONNAME\r"
rm -f ~/${MACHINE} && ln -s ${SESSION_HOME}/ftp_$SESSIONNAME ~/${MACHINE}
screen -S ftp${MACHINE}${SESSIONNAME} -p 0 -X stuff "cd ${SESSION_HOME}/ftp_$SESSIONNAME\r"
screen -S ftp${MACHINE}${SESSIONNAME} -p 0 -X stuff "alias amiga='cd $AMIGA'\r"
screen -S ftp${MACHINE}${SESSIONNAME} -p 0 -X stuff "clear\r"
screen -S ftp${MACHINE}${SESSIONNAME} -p 0 -X stuff "echo FTP ${MACHINE}:${SESSION} established to ${MACHINE_INFO} && ls\r"
screen -R ftp${MACHINE}${SESSIONNAME}

# ------------------------------------------- #
# backups
# ------------------------------------------- #
if [ $RSYNC -gt 0 ]
then
    echo -e "$0: ${HIGHLIGHT} 4. screen ended, rsync ${NOHIGHLIGHT}"
    TOTAL=`ls -C1d ${SESSION_HOME}/ftp_$SESSIONNAME/* | wc -l`
    let "CTOTAL = 0"
    for f in `ls -C1d ${SESSION_HOME}/ftp_$SESSIONNAME/*`
    do 
        basef=`basename $f`
        let "CTOTAL = CTOTAL + 1"

        if [[ "$RSYNC_SKIP" != *"$basef"* ]]; then
            if [[ "$f" == *".info"* ]]; then
                rsync $RSYNC_OPTS $f ${SESSION_HOME}/backup/ftp_${SESSIONNAME}/ > /dev/null 2>&1
                rsync -qvr ${SESSION_HOME}/backup/ftp_${SESSIONNAME}  ${SESSION_HOME}/archive/.ftp_${SESSIONNAME}_${RSYNC_YEAR}/ > /dev/null 2>&1
                rsync -qvr ${SESSION_HOME}/backup/ftp_${SESSIONNAME}  ${SESSION_HOME}/archive/.ftp_${SESSIONNAME}_${RSYNC_MONTH}/ > /dev/null 2>&1
                rsync -qvr ${SESSION_HOME}/backup/ftp_${SESSIONNAME}  ${SESSION_HOME}/archive/.ftp_${SESSIONNAME}_${RSYNC_DAY}/ > /dev/null 2>&1
            else
                if [ $RSYNC_DU -eq 1 ]
                then
                    echo -e "$0: ${HIGHLIGHT} 4. rsync disk usage $CTOTAL/$TOTAL: ${NOHIGHLIGHT} $basef"
                    SDATE=`date`
                    echo -e "`du -sh $f`"
                    echo -e "`du -sh ${SESSION_HOME}/backup/ftp_${SESSIONNAME}/$basef 2>/dev/null`" 
                    EDATE=`date`
                    echo -e "::: $EDATE"
                    echo -e "::: $SDATE"
                fi

                echo -e "$0: ${HIGHLIGHT} 4. rsync transfer $CTOTAL/$TOTAL: ${NOHIGHLIGHT} $basef"
                SDATE=`date`
                echo -e "Backup started on `date` for $f" >> ${SESSION_HOME}/backup.log
                echo -e "`du -sh ${SESSION_HOME}/backup/ftp_${SESSIONNAME}/$basef 2>/dev/null`" 
                $TIME rsync $RSYNC_OPTS $f ${SESSION_HOME}/backup/ftp_${SESSIONNAME}/
                rsync -qvr ${SESSION_HOME}/backup/ftp_${SESSIONNAME}  ${SESSION_HOME}/archive/.ftp_${SESSIONNAME}_${RSYNC_YEAR}/
                rsync -qvr ${SESSION_HOME}/backup/ftp_${SESSIONNAME}  ${SESSION_HOME}/archive/.ftp_${SESSIONNAME}_${RSYNC_MONTH}/
                rsync -qvr ${SESSION_HOME}/backup/ftp_${SESSIONNAME}  ${SESSION_HOME}/archive/.ftp_${SESSIONNAME}_${RSYNC_DAY}/
                EDATE=`date`
                echo -e "Backup completed on `date` for $f" >> ${SESSION_HOME}/backup.log
                echo -e "::: $EDATE"
                echo -e "::: $SDATE"
            fi
        else
                echo -e "$0: ${HIGHLIGHT} 4. rsync skip $CTOTAL/$TOTAL: ${NOHIGHLIGHT} $basef"
        fi
    done
fi

# ------------------------------------------- #
# cleanup
# ------------------------------------------- #
echo -e "$0: ${HIGHLIGHT} 4. screen ended, unmounting ${NOHIGHLIGHT} `date` ftp $MACHINE"
sudo umount -f ${SESSION_HOME}/ftp_$SESSIONNAME
mount | grep curlftpfs
screen -list
# ------------------------------------------- 5
echo -e "$0: ${HIGHLIGHT} 5. done ${NOHIGHLIGHT}"
