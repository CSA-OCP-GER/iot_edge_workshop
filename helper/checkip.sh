# checkip.sh
#
# Created by Jose Moreno
#
# This script can be used so that the raspberry pi reports the IP address it gets on its wlan0 interface
#   over a Microsoft Teams channel, so that you dont need to connect with a monitor and a laptop to find
#   it out.
# You can copy this script to the Raspberry Pi in the home directory (/home/pi/) and trigger it from crontab,
#    with an entry: * * * * * /home/pi/checkip.sh >/dev/null 2>&1
#
# The first variable is the URL a webhook that you have created in a channel Microsoft Teams
#
teams_url=https://outlook.office.com/webhook/your-very-long-key
new_ip=$(/sbin/ifconfig wlan0 | grep inet | grep -v inet6 | awk '{print $2}')
old_ip=$(cat /home/pi/old_ip_address.txt)
if [ "$new_ip" != "$old_ip" ]
then
    msg="Hello, my IP address seems to be $new_ip"
    curl -H "Content-Type: application/json" -X POST -d "{'text':'$msg'}" $teams_url >/dev/null 2>&1
    echo $new_ip >/home/pi/old_ip_address.txt
else
    echo 'No address change detected'
fi