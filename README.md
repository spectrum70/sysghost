[![License: GPL v2](https://img.shields.io/badge/License-GPL_v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![Open Source? Yes!](https://badgen.net/badge/Open%20Source%20%3F/Yes%21/blue?icon=github)](https://github.com/spectrum70/badges/)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://GitHub.com/spectrum70/sysghost/graphs/commit-activity)
[![GitHub branches](https://badgen.net/github/branches/spectrum70/sysghost)](https://github.com/spectrum70/sysghost/)
[![GitHub commits](https://badgen.net/github/commits/spectrum70/sysghost)](https://GitHub.com/spectrum70/sysghost/commit/)
[![GitHub issues](https://img.shields.io/github/issues/spectrum70/sysghost.svg)](https://GitHub.com/spectrum70/sysghost/issues/)
[![GitHub stars](https://badgen.net/github/stars/spectrum70/sysghost)](https://GitHub.com/spectrum70/sysghost/stargazers/)<br>
[![GitHub forks](https://img.shields.io/github/forks/spectrum70/sysghost.svg?style=social&label=Fork&maxAge=2592000)](https://GitHub.com/spectrum70/sysghost/network/)
[![GitHub followers](https://img.shields.io/github/followers/spectrum70.svg?style=social&label=Follow&maxAge=2592000)](https://github.com/spectrum70/sysghost?tab=followers)

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
sudo sysdown
```
### Scripts
Please find sample scripts in the scripts directory.
Mandatory names that are actually processed from sysghost are:
```
/etc/sysghost/udevd.sh
/etc/sysghost/commands.sh
```


