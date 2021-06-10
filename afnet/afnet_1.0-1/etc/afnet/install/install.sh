#!/bin/bash
echo "----------------------------------------------------------------------"
echo "o  Installing systemd files"
echo "----------------------------------------------------------------------"
sudo cp ../service/*.service /etc/systemd/user
systemctl --user daemon-reload
echo "----------------------------------------------------------------------"
echo "o  Status - don't care what this says"
echo "----------------------------------------------------------------------"
systemctl --user status afnet-ui.service
systemctl --user status afnet.service
echo "----------------------------------------------------------------------"
echo "o  Start"
echo "----------------------------------------------------------------------"
systemctl --user start afnet-ui.service
systemctl --user start afnet.service
echo "----------------------------------------------------------------------"
echo "o  Status - should be up"
echo "----------------------------------------------------------------------"
systemctl --user status afnet-ui.service
systemctl --user status afnet.service
