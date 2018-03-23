# Install

## 1. Install dependencies

- build-essential
- libv4l
- libx11
- libxext
- libjpeg
- libpng
- zlib

**Example on ubuntu 16.04 :**
```
$ sudo apt update
$ sudo apt install build-essential
$ sudo apt install libv4l-dev
$ sudo apt install libx11-dev
$ sudo apt install libxext-dev
$ sudo apt install libjpeg8-dev
$ sudo apt install libpng16-dev
$ sudo apt install zlib1g-dev
```

**Note :** For libpng, try installing "libpng-dev" instead if above command fails

## 2. Build / Reset

### 2.1. Build and install mmstreamer
```
$ git clone https://github.com/BoubacarDiene/mmstreamer.git mmstreamer_sources
$ cd mmstreamer_sources/
$ make all install
```

**Note** :
Please, see [TROUBLESHOOT](TROUBLESHOOT.md) if you encounter problems compiling mmstreamer.

### 2.2. Clean build directory
```
$ cd mmstreamer_sources/
$ make clean-all
```
or
```
$ make mrproper-all
```

### 2.3. Clean a particular module
```
$ cd mmstreamer_sources/
$ make -f build/Makefile.<xxx> clean-<xxx>
```
or
```
$ make -f build/Makefile.<xxx> mrproper-<xxx>

<xxx> : vid, deps, net, gfx, ... respectively for "Videos", "Dependencies", "Network", "Graphics", ...
```
