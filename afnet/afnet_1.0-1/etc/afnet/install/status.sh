#!/bin/bash
echo " "
echo "----------------------------------------------------------------------"
echo "o  Status - UI"
echo "----------------------------------------------------------------------"
sleep 1
systemctl --user status afnet-ui.service
journalctl --since 09:00 --user -u afnet-ui.service
echo " "
echo "----------------------------------------------------------------------"
echo "o  Status - SERVERS"
echo "----------------------------------------------------------------------"
sleep 1
systemctl --user status afnet.service
journalctl --since 09:00 --user -u afnet.service
echo "----------------------------------------------------------------------"
echo "o  Status - tail SERVERS"
echo "----------------------------------------------------------------------"
sleep 1
journalctl -f --since 09:00 --user -u afnet.service
