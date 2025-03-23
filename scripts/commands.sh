#!/bin/sh
#
# magic command sequence, for fast boot
# (C) 2025 kernel-space.org

source /etc/sysghost/lib.sh

setup_backlight() {
	chown root:video /sys/class/backlight/nv_backlight/brightness
	chown root:video /sys/class/backlight/nv_backlight/max_brightness
	chmod g+w /sys/class/backlight/nv_backlight/brightness
	chmod g+r /sys/class/backlight/nv_backlight/max_brightness
	cat /sys/class/backlight/nv_backlight/max_brightness > \
		/sys/class/backlight/nv_backlight/brightness
}

net_wait() {
	mmsg "net: waiting for ${1}"
	for i in $(seq 1 50); do
		if [ "$(ifconfig -a -s | grep ${1})" != "" ]; then
			mmsg "net: ${1} found"
			return
		fi
		sleep 0.2
	done
	emsg "net: timeout, skipping"
}

setup_network() {
	ETH_IF="enp4s0"
	WIFI_IF="wlo1"
	NS="1.1.1.1"

	sysctl -w net.ipv4.ping_group_range="0 1000" > /dev/null

	net_wait "${ETH_IF}"
	# ip addr add 192.168.0.4/24 dev ${ETH_IF}
	# ip link set dev enp4s0 up

	rfkill unblock wlan
	net_wait "${WIFI_IF}"

	mmsg "net: starting wpa_supplciant"
	/usr/bin/wpa_supplicant -qq -Dnl80211 -i${WIFI_IF} -c/etc/wpa_supplicant.conf 2>&1 > /dev/null &
	mmsg "net: starting dhclient"
	dhclient "${WIFI_IF}" > /dev/null

	# setup gateway
	# route add default gw 192.168.0.1

	# setup dns
	echo "nameserver ${NS}" > /etc/resolv.conf
}

setup_backlight
setup_network
