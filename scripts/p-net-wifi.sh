#!/bin/sh
#
# p-net-wifi.sh

# Parallel sript, message only at the end

source /etc/sysghost/lib-net.sh

# Enable rfkill and wait for unblock applied
/bin/rfkill unblock wlan

state_rfkill=""

for i in $(seq 1 10); do
	sleep 0.1
	lsmod | grep -q rfkill
	if [ $? = 0 ]; then
		state_rfkill="ok"
		break
	fi
done
if [ ${state_rfkill} != "ok" ]; then
	error
	return 1
fi

net_wait ${1}

# Start wpa_supplicant
/usr/bin/wpa_supplicant -qq -Dnl80211 -i${1} -c/etc/wpa_supplicant.conf 2>&1 > /dev/null
# Get ip address now
/usr/sbin/dhclient ${1}

