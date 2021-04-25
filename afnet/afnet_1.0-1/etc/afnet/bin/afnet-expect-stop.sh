#!/usr/bin/expect
set timeout 20

if { $argc eq 3 } {
  set killarg1 [lindex $argv 0]
  set killarg2 [lindex $argv 1]
  set killarg3 [lindex $argv 2]
  puts stdout "Killing curl mount with value: host $killarg1 mount $killarg3"
  spawn $env(AFHOME)/bin/afnet-script.sh --killarg $killarg1 $killarg3
  interact
} else {
  # kills the first one
  spawn $env(AFHOME)/bin/afnet-script.sh -k
  interact
}
