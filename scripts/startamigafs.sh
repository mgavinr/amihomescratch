#!/bin/bash
# ------------------------------------------- #
# startamigafs.sh Sun Apr 12 21:35:45 IST 2020 by Gavin Rogers
# = Readme =
# What: a bash script to start curlftpfs to an amiga (i.e. mount ftp server as local filsystem), optionally rsync at end will keep a local copy in output .dir
# Unix Requirements: ubuntu, arp screen curlftpfs, firewall to open port 21 20
# Amiga Requirements: AmiTcp ftpd and unix compatible ls in c:ls
# Usage: startamigafs.sh -h
# = Changelog =
# ------------------------------------------- #
export MACHINE=`arp | grep amiga | awk '{print $1}'`
export LOGIN=`whoami`:`whoami`
export HELP="Usage:\n  ./startamiga.sh -s RAM:\n  ./startamiga.sh -s SYS:\n  ./startamiga.sh -s SYS: --rsync\n  ./startamiga.sh --rsync\n  ./startamiga.sh --rsync --login username:password\n"
export SESSION=`whoami`
export SESSIONSET=0
export SESSION_HOME=/home/`whoami`/network/${MACHINE}/
export RSYNC=0
export DIR=""
# rsync new backup dir every
export RSYNC_DAY=`date | sed 's/ /-/g' | sed -e s'/\(^..........\).*$/\1/g'`
export RSYNC_MONTH=`date | sed 's/ /-/g' | sed -e s'/^....\(...\).*$/\1/g'`
export RSYNC_YEAR=`date | sed 's/ /-/g' | sed -e s'/.*-//g'`
export RSYNC_LATEST="latest"
export RSYNC_SKIP="\
apps.old\
AExplorer\
AmiSSL\
AWeb_APL\
Cinemaware\
"
HIGHLIGHT="\033[1;33m"
NOHIGHLIGHT="\033[0m"

# ------------------------------------------- #
# Usage
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
SESSIONNAME=`echo $SESSION | sed 's/://g' | sed 's#/##g'`
if [ $SESSIONSET -gt 0 ]
then
    AMIGALOCATION=${SESSION}
else
    AMIGALOCATION=""
fi
SESSIONNAME=`echo $SESSIONNAME | tr "[:upper:]" "[:lower:]"`

# trap ctrl-c and call ctrl_c()
trap ctrl_c INT

function ctrl_c() {
    echo "** $* Trapped CTRL-C"
    exit 1
}

# ------------------------------------------- #
# check
# ------------------------------------------- #
[[ -z "$MACHINE" ]] && { echo "Amiga hostname is not found, MACHINE=" ; exit 1; }
ping -c1 $MACHINE 1>/dev/null 2>/dev/null
SUCCESS=$?
if [ $SUCCESS -eq 0 ]
then
  echo "$MACHINE has replied"
else
  echo "$MACHINE didn't reply"
  exit 1
fi

# ------------------------------------------- #
# start
# ------------------------------------------- #
mkdir -p ${SESSION_HOME}/ftp_$SESSIONNAME
export AMIGA=${SESSION_HOME}/ftp_$SESSIONNAME
echo -e "${HIGHLIGHT}o Starting screen `date` ftp ${MACHINE} ${NOHIGHLIGHT} ...................."
screen -d -m -S ftp$SESSIONNAME bash -c 'bash'
screen -S ftp$SESSIONNAME -p 0 -X stuff "curlftpfs -o nonempty -s ${LOGIN}@${MACHINE}/${AMIGALOCATION} ${SESSION_HOME}/ftp_$SESSIONNAME\r"
screen -S ftp$SESSIONNAME -p 0 -X stuff "cd ${SESSION_HOME}/ftp_$SESSIONNAME\r"
screen -S ftp$SESSIONNAME -p 0 -X stuff "alias amiga='cd $AMIGA'\r"
screen -S ftp$SESSIONNAME -p 0 -X stuff "clear\r"
screen -S ftp$SESSIONNAME -p 0 -X stuff "echo -n _______________ ${MACHINE}:${SESSION} _________________: && pwd && ls\r"
screen -R

# ------------------------------------------- #
# cleanup
# ------------------------------------------- #
if [ $RSYNC -gt 0 ]
then
    echo -e "${HIGHLIGHT}o Rsync ${NOHIGHLIGHT} .............................."
    for f in `ls -C1d ${SESSION_HOME}/ftp_$SESSIONNAME/*`
    do 
        basef=`basename $f`
        if [[ "$RSYNC_SKIP" != *"$basef"* ]]; then
            if [[ "$f" == *".info"* ]]; then
                rsync --progress --human-readable --checksum -vr $f ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST}/ > /dev/null 2>&1
                rsync -qvr ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST}  ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_YEAR}/ > /dev/null 2>&1
                rsync -qvr ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST}  ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_MONTH}/ > /dev/null 2>&1
                rsync -qvr ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST}  ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_DAY}/ > /dev/null 2>&1
            else
                echo -e "--------------------------------"
                echo -e "Calculating usage for $f.."
                echo -e "`du -sh $f`"
                echo -e "`du -sh ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST}/$basef 2>/dev/null`" 
                echo -e "Backup started on `date` in ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST} for $basef"
                echo -e "Backup started on `date` for $f" >> ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST}/.backup_log
                time rsync --progress --human-readable --checksum -vr $f ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST}/
                rsync -qvr ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST}  ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_YEAR}/
                rsync -qvr ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST}  ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_MONTH}/
                rsync -qvr ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST}  ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_DAY}/
                echo -e "Backup completed on `date` for $f" >> ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST}/.backup_log
                echo -e "Backup completed on `date` in ${SESSION_HOME}/.ftp_${SESSIONNAME}_${RSYNC_LATEST} for $basef"
                echo -e "--------------------------------"
            fi
        else
                echo -e "--------------------------------"
                echo -e "$f skipped"
                echo -e "--------------------------------"
        fi
    done
fi
#echo -e "${HIGHLIGHT}o Screen ended${NOHIGHLIGHT} ............................"
#sudo umount ${SESSION_HOME}/ftp_$SESSIONNAME
#mount | grep curlftpfs
#screen -list
