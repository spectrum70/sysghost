#!/bin/sh
#
# magic udevd sequence, for fast boot
# (C) 2025 kernel-space.org

source /etc/sysghost/lib.sh

# Dynamically plugged devcices are not in devtmpfs
# but added by uevent kernel->udevd. Since for now we are
# not usuing udev, loading them here.

load_early_cmds()
{
	# Any early command here.
}


# Considering using systemd-udevd, that is declared as working
# standalone uevent menager, this is needed to retrigger kernel
# uevents issued before udevd has been started.
retrigger_all_uevents()
{
        udevadm trigger --action=add --type=subsystems
        udevadm trigger --action=add --type=devices
        udevadm settle
}

load_early_cmds
retrigger_all_uevents

