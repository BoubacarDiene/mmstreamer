# Table of contents
- [SDL2_ttf failed to build](#sdl2_ttf-failed-to-build)
- [Linker failed to find libpng](#linker-failed-to-find-libpng)
- [PNG images are not supported](#png-images-are-not-supported)

## SDL2_ttf failed to build

Create a symbolic link to the installed version of libGL.
Example on ubuntu 16.04 64 bits :

```
cd /usr/lib/
sudo ln -s x86_64-linux-gnu/mesa/libGL.so.1 libGL.so
```

## Linker failed to find libpng

Supposing that "-dev" version of libpng is installed and "/usr/lib/x86_64-linux-gnu/libpng.so"
does not exist, create a symbolic link to the installed version of libpng.

Example on ubuntu 17.10 64 bits :

```
cd /usr/lib/x86_64-linux-gnu/
sudo ln -s libpng16.so.16.34.0 libpng.so
```

## PNG images are not supported

An error like **"Drawer : Drawer.c:1206 (initSdlComponents_f) IMG_Init() failed
to load required modules : PNG images are not supported"** means that SDL did not
find installed libpng.

To fix this, refer to point 2) above then 
```
make -f build/Makefile.deps mrproper-dependencies
make all install
```
