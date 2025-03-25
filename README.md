

<p align="center">
  <img src="res/sysghost.png" width="100%" title="hover text">
</p>


<p align="center"><b> A simple init system</b></p>

A **fast** and **easy to configure** init system, written in C with a very simple design, features kept at minimum, can be installed together with systemd.
Versions < 1.00 are alpha, experimental.

(C) 2025 - Angelo Dureghello - kernelspace <angelo@kernel-space.org>

## Build
```
autoreconf -fi
./configure
make
sudo make install
```

## Usage
### Boot using sysghost
Just add sysghost to the kernel command line, as
```
"root=UUID=3d1c53ef-a4f0-4226-a87c-fb1ac155553b rw loglevel=14 audit=0 init=/usr/local/bin/sysghost"
```
### Shutdown
```
sudo sysdiown
```
### Scripts
Please find sample scripts in the scripts directory.
Mandatory names that are actually processed from sysghost are:
```
/etc/sysghost/udevd.sh
/etc/sysghost/commands.sh
```


