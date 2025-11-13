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
	ip link set dev ${1} up

	net_wait ${1}
	if [ $? = 0 ]; then
		success
	else
		error
	fi
}


