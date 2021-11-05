#!/bin/bash
echo "----------------------------------------------------------------------"
echo "o  Stopping"
echo "----------------------------------------------------------------------"
sudo systemctl stop afnet-ui-root.service
sudo systemctl stop afnet-root.service
echo "----------------------------------------------------------------------"
echo "o  Status"
echo "----------------------------------------------------------------------"
systemctl status afnet-ui-root.service
systemctl status afnet-root.service
echo "----------------------------------------------------------------------"
echo "o  Uninstalling systemd files"
echo "----------------------------------------------------------------------"
for file in `ls ../service/*.service`
do
  echo "Uninstalling $file"
  sudo bash -c "rm -f /etc/systemd/user/`basename $file` > /dev/null 2>&1"
  sudo bash -c "rm -f /etc/systemd/system/`basename $file` > /dev/null 2>&1"
done
sudo systemctl daemon-reload
echo "----------------------------------------------------------------------"
echo "o  Status"
echo "----------------------------------------------------------------------"
systemctl status afnet-ui-root.service
systemctl status afnet-root.service
