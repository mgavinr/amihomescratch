#!/bin/bash
echo "----------------------------------------------------------------------"
echo "o  Stopping"
echo "----------------------------------------------------------------------"
systemctl --user stop afnet-ui.service
systemctl --user stop afnet.service
systemctl --user status afnet-ui.service
systemctl --user status afnet.service
echo "----------------------------------------------------------------------"
echo "o  Uninstalling systemd files"
echo "----------------------------------------------------------------------"
for file in `ls ../service/*.service`
do
  echo "Uninstalling $file"
  sudo rm /etc/systemd/user/`basename $file`
done
systemctl --user daemon-reload
