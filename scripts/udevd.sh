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
	cat /sys/class/backlight/nv_backlight/max_brightness > \
		/sys/class/backlight/nv_backlight/brightness
}

load_modules()
{
	modprobe amd_atl
	modprobe k10temp
	modprobe mt7921e
	modprobe realtek
	modprobe r8169
	modprobe snd_hda_intel
	modprobe ccp
	modprobe snd_rpl_pci_acp6x
	modprobe i8042
	modprobe bluetooth
	modprobe nfnetlink
	modprobe x_tables
	modprobe rfkill
	modprobe ideapad_laptop
	modprobe i2c_hid_acpi
	modprobe i2c_piix4
	modprobe i2c_smbus
	modprobe i2c_hid
	modprobe mac80211
	modprobe mousedev
	modprobe psmouse
	modprobe serio
	modprobe sparse_keymap
	modprobe typec
	modprobe libps2
	modprobe hid_generic
	modprobe hid_multitouch
	modprobe vivaldi_fmap
	modprobe loop
	modprobe fuse
	modprobe ip_tables

	# Mandatory, udev needs to process all the events
	# related to the manually modprobed modules.
	udevadm trigger
}

setup_devices()
{
	# Works well for arch linux

	mmsg "setup devices ..."

	sleep 1

	cd /dev/

	find . -type d -exec chmod 755 {} +

	# Symlinks
	#ln -s rtc0 rtc
	#ln -s /proc/kcore core
	#ln -s /proc/self/fd fs
	#ln -s /proc/self/fd/2 stderr
	#ln -s /proc/self/fd/0 stdin
	#ln -s /proc/seld/fd/1 stdout

	chown root:video /sys/class/backlight/nv_backlight/brightness
        chown root:video /sys/class/backlight/nv_backlight/max_brightness
        chmod g+w /sys/class/backlight/nv_backlight/brightness
        chmod g+r /sys/class/backlight/nv_backlight/max_brightness

	cd snd
	find . -type f -exec chmod 755 {} +
	find . -type c -exec chown root:audio {} +
	cd ../..

        chown root:tty /dev/vcs*
        chown root:tty /dev/tty*
        chmod 666 /dev/dri/card0
        chmod 666 /dev/dri/render*
        chown root:video /dev/fb0
        chown root:input /dev/input/event*
        chmod 660 /dev/input/event*

	mmsg "setup devices done"
}

load_extra() {
	chown root:video /sys/class/backlight/nv_backlight/brightness
	chmod 664 /sys/class/backlight/nv_backlight/brightness

	chmod 666 /dev/dri/card0
        chmod 666 /dev/dri/render*
        chown root:video /dev/fb0
	chown root:video /dev/dri/card0
	chown root:video /dev/dri/render*
	chown -R root:input /dev/input/*
	chmod 660 /dev/input/e*
	chown root:tty /dev/tty*
	chown root:tty /dev/vcs*
}

load_early_cmds
load_modules

# Use this in case udevd is not used
# setup_devices

# Extra init stuff not properly done bu udevd
load_extra
