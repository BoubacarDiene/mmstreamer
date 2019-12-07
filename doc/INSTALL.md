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
$ make clean
```
or
```
$ make mrproper
```

## 3. Build mmstreamer inside docker

### 3.1. Generate docker image
```
$ cd mmstreamer_sources/
$ docker build -t  mmstreamer-image ci/
```

### 3.2. Use docker image
```
$ cd mmstreamer_sources/
$ docker run -it -u $(id -u) --rm -v $(pwd):/workdir mmstreamer-image:latest
$ make all install
```
