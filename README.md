<p align="center">
	  <a href="https://github.com/spectrum70/badges/"><img alt="" src="https://badgen.net/badge/Open%20Source%20%3F/Yes%21/blue?icon=github"></a>
	  <a href="https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html"><img alt="" src="https://img.shields.io/badge/License-GPL_v2-blue.svg"></a>
	  <a href="https://GitHub.com/spectrum70/sysghost/graphs/commit-activity"><img alt="" src="https://img.shields.io/badge/Maintained%3F-yes-green.svg"></a><br>
	  <a href="https://GitHub.com/spectrum70/sysghost/starrers"><img alt="" src="https://img.shields.io/github/stars/spectrum70/sysghost" /></a>
	  <a href="https://github.com/spectrum70/sysghost?"><img alt="" src="https://img.shields.io/github/followers/spectrum70.svg?style=social&label=Follow&maxAge=2592000"></a></br></br>
      <a href="https://awesome.re"><img alt="" src="https://awesome.re/badge.svg"></a>
</p>
<p align="center">
  <img src="res/sysghost.png" width="100%" title="hover text">
</p>
<p align="center"><b> A simple init system</b></p>

A **fast** and **basic** init system, written in C with a very simple design, features kept at minimum to leave to the user more freedom customizing the boot strategy
as preferred. C code should be clear and well readable, and easy to modify or extended as needed. The **sysghost** init is, at the current stage, meant to be installed together with systemd,
since some services as systemd-udevd are used by default. Code can be easily modified to avoid totally any systemd stuff.
```diff
! Note: versions < 1.00 are alpha, experimental, don't expect too much from them.
+ Note: without systemd, some comfortable features of your system will be missing.
```
(C) 2025 - Angelo Dureghello - kernelspace <angelo@kernel-space.org>

## Build
```
autoreconf -fi
./configure
make
sudo make install
```

## Usage

## Boot using sysghost
Just add sysghost to the kernel command line, as
```
"root=UUID=3d1c53ef-a4f0-4226-a87c-fb1ac155553b rw loglevel=14 audit=0 init=/usr/local/bin/sysghost"
```

<p align="center">
  <img src="res/boot.png" width="70%" title="hover text">
</p>

### Configuration
The sysghost init system is designed to launch a minimal set of services just after the early
initializations as udevd, and mount. The list of services is actually harcoded, to keep
the sequential boot process as fast as possible.</br>
Many distro as default install systemd, that comes together with other Red Hat stuff, sysghost actually
uses such tools as default in the init process (udevd, dbus, cupsd, ...).

Edit and change this list as needed in **src/launcher.c**:
```
/*
 * Main idea was to hardcode the services here.
 */
void lanucher_step_run_services()
{
	int i;
	char list[][MAX_ENTRY] = {
		{"/bin/seatd -l silent -g seat >/dev/null"},
		{"/usr/sbin/sshd"},
		{"/usr/bin/cupsd"},
		{"/usr/bin/avahi-daemon -D"},
		{0},
	};

	for (i = 0; *list[i]; i++) {
		exec_daemon(list[i]);
	}
}     
```

Then, sysghost uses some minimal scripts to setup system configuration. They are located in
```
ls -al /etc/sysghost
drwxr-xr-x   2 root root  4096 30 mar 20.41 .
drwxr-xr-x 144 root root 12288 30 mar 10.54 ..
-rwxr-xr-x   1 root root  1170 30 mar 20.49 commands.sh
-rw-r--r--   1 root root   210 24 mar 22.19 lib.sh
-rwxr-xr-x   1 root root   579 30 mar 10.32 udevd.sh
```
The **lib.sh** is a common library included in each script. Check sample scripts installed and customize them as needed.

### Device manager
By default, sysghost uses udevd (systemd-udevd) as a device manager, that is generally installed in the system. 
For this case, udevd.sh script is processed. As default, all devices should be added properly without
any change to the script.

### command.sh scripts
Add here whatever additional configuration to be performed at boot. Additional services can be executed form here.

### Boot sequence
```
                     ________________________                  
                    |     udevd, udevd.sh    |      actually, systemd-udevd is the default/only device manager               
                    | get all uevents,       |      src/launcher.c, etc/sysghost/udevd.sh
                    | modprobe, add devices  |
                    | apply rules            | 
                    |________________________|
                                 |
                     ________________________                  
                    |      mount (fstab)     |     src/launcher.c
                    |________________________|
                                 |
                     ________________________                  
                    |     run system dbus    |     src/launcher.c    
                    |________________________|
                                 |
                     ________________________                  
                    |       command.sh       |     etc/sysghost/command.sh              
                    |________________________|
                                 |
                     ________________________
                    |      run_services()    |     src/launcher.c, lanucher_step_run_services()
                    |________________________|
                                 |
                     ________________________
                    |     creates virtual    |     src/launcher.c
                    |         consoles       |     creates 4 virtual consoles
                    |________________________|
                                 |
                     ________________________
                    |          login         |     src/launcher.c
                    |________________________|
```


### Shutdown
By default, sysdown just power down the pc.
```
$ sudo sysdown

$ sysdown --help                                                                                           ✔ 
Usage: sysdown [option]
Example: ./sysdown -r
Options:
  -h,  --help        this help
  -r,  --reboot      reboot system
  -V,  --version     program version
```

### Unit test
Unit test can be executed by:
```
sudo unit/sysunit
```

### To avoid systemd/redhat tools, suggested alternatives
**metalog** for logging, logs in /var/log/everything/current,<br>
**uam** usb automount without polkit/systemd (build it from sources),<br>


### Credits
If you liked this init, you can send an email to <angelo@kernel-space.org> to say thanks,
so that i know someone appreciated it.

