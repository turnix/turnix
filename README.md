# Turnix

An operating system just for fun.

![screenshot](/doc/screenshot.png?raw=true)

## Features

* Priority based scheduling, and there are 32 priorities.
* Support keyboard and text mode.
* Support some standard C and UNIX APIs. So it is good for UNIX developers.

## Architectures

* i386

## How to try it

1. Install a 32-bit Linux distribution, such as *Ubuntu*, *Debian*, *Fedora* or *RHEL*.
2. Install the development tools, such as *make*, *gcc*, *binutils* and *qemu*.
3. Install *xorriso* for generating the final iso.
4. Build Turnix with `make`, and run it with `make qemu` if everything is OK.
