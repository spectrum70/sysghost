# sysghost init system

![Alt text](res/sysghost.png?raw=true "sysghost")

A simple init system, fully configurable, can live installed together with systemd, 
while not using it.

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
Please find sample scripts in the scripts direvotry.
Mandatory names actually are:
```
/etc/sysghost/udevd.sh
/etc/sysghost/commands.sh
```


