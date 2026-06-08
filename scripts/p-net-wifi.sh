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
if [ "${state_rfkill}" != "ok" ]; then
	error
	exit 1
fi

# Wait iface to be available
net_wait ${1}

# Set iface up
/usr/bin/ip link set ${1} up

# Start wpa_supplicant, run it in background and check state
/usr/bin/wpa_supplicant -qq -Dnl80211 -i${1} -c/etc/wpa_supplicant.conf 2>&1 > /dev/null &

# Waiting for connect
while [ true ]; do
	sleep 0.5
	if [[ ! -e /var/run/wpa_supplicant ]]; then
		continue
	fi
	wpa_status=$(wpa_cli status | grep wpa_state | cut -d"=" -f2)
	if [[ $wpa_status == *"COMPLETED"* ]]; then
		/usr/sbin/dhclient ${1}
		exit 0
	fi
done

echo "cannot connect to wifi network, exiting"

exit 1
