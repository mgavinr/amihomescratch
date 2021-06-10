#!/usr/bin/expect
puts " "
puts "EXPECT STOP SCRIPT:"
# if you don't list the expect string it is possible the script will be killed before it gets a chance to finish
# as interact is not running or doing anything, i am guessing systemd is to blame, as interact via bash cmdline is aok
set timeout 20

if { $argc eq 3 } {
  set killarg1 [lindex $argv 0]
  set killarg2 [lindex $argv 1]
  set killarg3 [lindex $argv 2]
  puts stdout "Killing curl mount with value: host $killarg1 mount $killarg3"
  puts stdout "spawn $env(AFHOME)/bin/afnet-script.sh --killarg $killarg1 $killarg3"
  spawn $env(AFHOME)/bin/afnet-script.sh --killarg $killarg1 $killarg3
  expect "Stopping curl command" {
    interact
  }
} else {
  # kills the first one
  spawn $env(AFHOME)/bin/afnet-script.sh -k
  interact
}
