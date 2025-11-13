#!/bin/sh
#
# sysghost command sequence, for fast boot
# (C) 2025 kernel-space.org

source /etc/sysghost/lib-net.sh

setup_anything() {
	# Setup whatever else needed.
}

setup_network() {
	# NS="1.1.1.1"

	# Specific net configurations ...
	# sysctl -w net.ipv4.ping_group_range="0 1000" > /dev/null

	# Using net_start From lib-net
	# net_start lo 127.0.0.1/24 up
	# net start eth0 192.168.0.2/24 up

	# Wifi start can take seconds, p-net-wifi.sh
	# runs it in parallel.
	# /etc/sysghost/p-net-wifi.sh wlo1 &

	# Setup dns
	# echo "nameserver ${NS}" > /etc/resolv.conf
}

# Order below the preferred setup sequence.
setup_anything
setup_network

# Start here additional services.
# start_service "/usr/sbin/metalog --daemonize"
# start_service "/usr/sbin/pcscd --disable-polkit"
