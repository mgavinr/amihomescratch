#!/usr/bin/expect
set timeout 20
set HOSTNAME [lindex $argv 0]
set USERNAME [lindex $argv 1]
set DIRS [lindex $argv 2]
set DIRS2 [string trimright $DIRS :]
set AFTERV 1000

foreach directory $DIRS {
  spawn $env(AFHOME)/bin/afnet-script.sh -n
  expect "DIRECTORY?" { 
    send $directory
    send "\r"
    after $AFTERV
    expect "AMIGA HOSTNAME?" { 
      send "\x17"
      send ${HOSTNAME}
      send "\r" 
    }
    after $AFTERV
    expect "FTP LOGIN?" { 
      send "\x17"
      send ${USERNAME}
      send "\r" 
    }
    after $AFTERV
    expect "Local storage:" { send "\r" }
    after $AFTERV
    expect "The following directories are mounted:"
    expect "$HOSTNAME"
    expect "$DIRS2"
    interact
  }
}
