#!/bin/sh
#
# sysghost command sequence, for fast boot
# (C) 2025 kernel-space.org

source /etc/sysghost/lib-net.sh

setup_backlight() {
	chown root:video /sys/class/backlight/nv_backlight/brightness
	chown root:video /sys/class/backlight/nv_backlight/max_brightness
	chmod g+w /sys/class/backlight/nv_backlight/brightness
	chmod g+r /sys/class/backlight/nv_backlight/max_brightness
	cat /sys/class/backlight/nv_backlight/max_brightness > \
		/sys/class/backlight/nv_backlight/brightness
}

setup_network() {
	NS="1.1.1.1"

	sysctl -w net.ipv4.ping_group_range="0 1000" > /dev/null

	net_start lo 127.0.0.1/24 up

	# wifi can take seconds, run it in parallel
	/etc/sysghost/p-net-wifi.sh wlo1 &

	# setup dns
	echo "nameserver ${NS}" > /etc/resolv.conf
}

setup_backlight
setup_network

start_service "/usr/sbin/metalog --daemonize"
start_service "/usr/sbin/pcscd --disable-polkit"
