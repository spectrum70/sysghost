#!/bin/sh
#
# magic udevd init sequence
# (C) 2025 kernel-space.org

source /etc/sysghost/lib.sh

# Add here any manual module loading (modprobe stuff).
load_modules()
{
	#modprobe module_a
	#modprobe module_b
	udevadm trigger
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

#load_modules
retrigger_all_uevents
