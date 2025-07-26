#!/bin/sh

source /etc/sysghost/lib.sh

net_wait() {
	for i in $(seq 1 30); do
		if [ "$(ifconfig -a -s | grep ${1})" != "" ]; then
			return 0
		fi
		sleep 0.2
	done

	return 1
}

net_start() {
	step "net: starting iface ${1} ... "

	ip addr add ${2} dev ${1}
	ip link set dev lo up

	net_wait ${1}
	if [ $? = 0 ]; then
		success
	else
		error
	fi
}

net_start_wifi() {
	# we may have a post-up rule, second call to dhclient will just fail
	# silently
	state_rfkill=""

	step "net: waiting rfkill module activation ... "
	for i in $(seq 1 3); do
 		sleep 0.2
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
	success

 	/bin/rfkill unblock wlan

        step "net: starting ${1} ... "
	# waiting for coming visible
	net_wait ${1}

	/usr/bin/wpa_supplicant -qq -Dnl80211 -i${1} -c/etc/wpa_supplicant.conf 2>&1 > /dev/null &
	for i in $(seq 1 10); do
		sleep 2
		wpa_status=$(wpa_cli  status | grep wpa_state | cut -d"=" -f2)
		if [[ $wpa_status == *"COMPLETED"* ]]; then
		        dhclient "${1}" > /dev/null
			success
			return 0
 		fi
	done

	error
	return 1
}
