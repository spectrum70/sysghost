#!/bin/sh
#
# parallel-net.sh

# parallel sript, message only at the end

source /etc/sysghost/lib-net.sh

/bin/rfkill unblock wlan

# we may have a post-up rule, second call to dhclient will just fail
# silently
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

/usr/bin/wpa_supplicant -qq -Dnl80211 -i${1} -c/etc/wpa_supplicant.conf 2>&1 > /dev/null &

/usr/sbin/dhclient ${1}

