#!/bin/sh
#
# magic command sequence, for fast boot
# (C) 2025 kernel-space.org

source /etc/sysghost/lib.sh

net_wait()
{
	step "net: waiting for ${1} ... "
	for i in $(seq 1 50); do
		if [ "$(ifconfig -a -s | grep ${1})" != "" ]; then
			success
			return
		fi
		sleep 0.2
	done
	error
}

setup_network()
{
        ETH_IF="enp12s0"
        WIFI_IF="wlo1"
        NS="1.1.1.1"

        mmsg "net: setup network"

        sysctl -w net.ipv4.ping_group_range="0 1000" > /dev/null

        # setup localhost
        ip addr add 127.0.0.1/24 dev lo
        ip link set dev lo up

        net_wait "${ETH_IF}"
        ip addr add 192.168.0.2/24 dev ${ETH_IF}
        ip link set dev ${ETH_IF} up

        # rfkill unblock wlan
        # net_wait "${WIFI_IF}"

        # mmsg "net: starting wpa_supplciant"
        # /usr/bin/wpa_supplicant -qq -Dnl80211 -i${WIFI_IF} -c/etc/wpa_supplicant.conf 2>&1 > /dev/null &
        # mmsg "net: starting dhclient"
        # dhclient "${WIFI_IF}" > /dev/null

        # setup gateway
        route add default gw 192.168.0.1

        # setup dns
        echo "nameserver ${NS}" > /etc/resolv.conf
}

setup_network

# do anything else here
