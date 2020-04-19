FTP
====

A1200 Black:
ADH0: SYS
ADH1: Apps: applications like easy dir vim ww tools/ dpaint
ADH2: Work: gamkes junk and picks art disks

ftp
    Workbench3.1:Samba/usr/gavinr

Unix machine cant do ftp data, but the windows one can

The following is okay

total 14
-rwxrwxrwx  1       -2       52 Nov 28 11:28 hello.txt
-rwxrwxrwx  1       -2      464 Nov 28 09:14 lftp.sh
-rwxrwxrwx  1       -2    12360 Nov 28 09:02 myls

The following is not

-----rwed       1       52 Nov 28 11:28 hello.txt
-----rwed       1      464 Nov 28 09:14 lftp.sh
-----rwed      25    12360 Nov 28 09:02 myls
Dirs:0    Files:3    Blocks:27    Bytes:12876   

AmiTCP FTP
---
  1. Copy ftpd to AmiTCP:Serv/
  2. Edit inetd.conf to look something like this:
     ftp       stream      tcp nowait root AmiTCP:Serv/ftpd ftpd -x -b30
     ftp       stream      tcp nowait root dh1:tcp/bsdftpd/ftpd ftpd -x -b30
     -d,-v  Turns on debug-mode. A lot of info will show up in AmiTCP-log.
     -l     Turns on logging, ftpd will show things like 'user lilja logged
            in from xxxx' in the AmiTCP-log.
     -x     Activates file-transfer logging, the log-file is
            AmiTCP:log/xferlog.
     -bnnn  Specifies how large buffer ftpd should use when sending/receiving files.
            The buffer will be nnn*512 bytes long. Default is nnn=10. This
            was added in order to speed up file-transfers.
     -tnn   Timeout in seconds. ftpd will exit when the user has been idle for
            nn seconds.
     -Tnn   Maximum timeout, the timeout setting must be below nn seconds.
     -u     Set umask (has no affect on Amiga version yet).
     -G     Allow uploads by anonymous users (the default is not to allow anonymous users to upload stuff).

The format of passwd is
username||1000|1000|full username|amiga:homedir|sh
I think you can use passwd then to update the bit between ||
