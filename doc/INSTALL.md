# Table of contents
- [Build mmstreamer inside docker](#build-mmstreamer-inside-docker)
  - [Generate docker image](#generate-docker-image)
  - [Run container and generate executable](#run-container-and-generate-executable)
- [Build mmstreamer outside docker](#build-mmstreamer-outside-docker)
  - [Install dependencies on your host machine](#install-dependencies-on-your-host-machine)
  - [Generate and install executable](#generate-and-install-executable)
- [Clean build directory](#clean-build-directory)

## Build mmstreamer inside docker

### Generate docker image
```
$ cd mmstreamer_sources/
$ docker build -t  mmstreamer-image ci/
```

### Run container and generate executable
```
cd mmstreamer_sources/
docker run --privileged -it -u $(id -u) --rm -v $(pwd):/workdir mmstreamer-image:latest
make all install
```

## Build mmstreamer outside docker

### Install dependencies on your host machine

These packages are those used in the [docker image](../ci/Dockerfile). If you encounter any issue (compilation failure due to missing packages or a runtime issue), please check the Dockerfile to make sure no new package has been added afterward but not reported here.

- build-essential
- libgl1-mesa-dev
- libgl1-mesa-glx
- libjpeg8-dev
- libpng16-dev
- libv4l-dev
- libx11-dev
- libxext-dev
- zlib1g-dev

**Note :** For libpng, try "libpng-dev" instead if installing libpng16-dev fails.

### Generate and install executable
```
git clone https://github.com/BoubacarDiene/mmstreamer.git mmstreamer_sources
cd mmstreamer_sources/
make all install
```

**Note** :
Please, see [TROUBLESHOOT](TROUBLESHOOT.md) if you encounter problems compiling mmstreamer.

## Clean build directory
```
cd mmstreamer_sources/
make clean OR make mrproper
```
