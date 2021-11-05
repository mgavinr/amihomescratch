#!/bin/bash
echo "----------------------------------------------------------------------"
echo "o  Installing systemd files"
echo "----------------------------------------------------------------------"
sudo cp ../service/*root.service /etc/systemd/system/
sudo systemctl daemon-reload
echo "----------------------------------------------------------------------"
echo "o  Start"
echo "----------------------------------------------------------------------"
sudo systemctl start afnet-ui-root.service
sudo systemctl start afnet-root.service
echo "----------------------------------------------------------------------"
echo "o  Status - UI"
echo "----------------------------------------------------------------------"
systemctl status afnet-ui-root.service
echo "----------------------------------------------------------------------"
echo "o  Status - SERVICE"
echo "----------------------------------------------------------------------"
systemctl status afnet-root.service
echo "----------------------------------------------------------------------"
echo "o  Status - TAIL SERVICE"
echo "----------------------------------------------------------------------"
journalctl -u afnet-root -f
