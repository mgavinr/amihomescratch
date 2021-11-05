#!/bin/bash
echo "----------------------------------------------------------------------"
echo "o  Stopping"
echo "----------------------------------------------------------------------"
systemctl --user stop afnet-ui-user.service
systemctl --user stop afnet-user.service

systemctl --user status afnet-ui-user.service
systemctl --user status afnet-user.service
echo "----------------------------------------------------------------------"
echo "o  Uninstalling systemd files"
echo "----------------------------------------------------------------------"
for file in `ls ../service/*.service`
do
  echo "Uninstalling $file"
  sudo bash -c "rm -f /etc/systemd/user/`basename $file` > /dev/null 2>&1"
  sudo bash -c "rm -f /etc/systemd/system/`basename $file` > /dev/null 2>&1"
done
systemctl --user daemon-reload
