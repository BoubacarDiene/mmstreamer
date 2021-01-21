# Table of contents
- [Start mmstreamer](#start-mmstreamer)
  - [Inside docker container](#inside-docker-container)
  - [Outside docker container](#outside-docker-container)
- [Connect to inet server](#connect-to-inet-server)
  - [From your local computer](#from-your-local-computer)
  - [From a remote device](#from-a-remote-device)
- [Update configuration files](#update-configuration-files)
- [Export more build options](#export-more-build-options)
- [Extend mmstreamer](#extend-mmstreamer)
- [Build and run unit tests](#build-and-run-unit-tests)

## Start mmstreamer

**Notes:**
- In below sections, <x.y> has to be replaced with the current version of the binary.
- Another path to Main.xml config file can be specified using -f option\
```
  E.g.: $ ./out/mmstreamer/bin/mmstreamer-<x.y> -f /tmp/Main.xml
```
- In case a "bitsPerPixel" different from the active framebuffer's depth is specified in Graphics.xml, mmstreamer will try to change depth. To make it work, "root" permission is required (E.g. Run mmstreamer as root)
- If mmstreamer failed to start, please, see [TROUBLESHOOT](TROUBLESHOOT.md)

### Inside docker container
```
cd mmstreamer_sources/
docker run --privileged -it -u root --rm \
           -v $(pwd):/workdir \
           --volume="$HOME/.Xauthority:/root/.Xauthority:rw" \
           -v="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
           -v /dev:/dev \
           --net=host --env="DISPLAY" \
           mmstreamer-image:latest

RELEASE MODE:

LD_LIBRARY_PATH=./out/mmstreamer/lib ./out/mmstreamer/bin/mmstreamer-<x.y>

DEBUG MODE:

LD_LIBRARY_PATH=./out/mmstreamer/lib ./out/mmstreamer/bin/mmstreamer-<x.y>.dbg
```

### Outside docker container
```
cd mmstreamer_sources/

RELEASE MODE:

sudo LD_LIBRARY_PATH=./out/mmstreamer/lib ./out/mmstreamer/bin/mmstreamer-<x.y>

DEBUG MODE:

sudo LD_LIBRARY_PATH=./out/mmstreamer/lib ./out/mmstreamer/bin/mmstreamer-<x.y>.dbg
```

## Connect to inet server

### From your local computer

Once the mmstreamer executable started, you can open your favorite web browser then type one of the following url:
```
http://127.0.0.1:9090/webcam or http://localhost:9090/webcam
```

### From a remote device

The remote device can be a phone, a different computer, ... connected to the same network as the server. The main change here is the IP address (or domain name) to reach the host machine where the server is running (<server_ip_address> below).
```
http://<server_ip_address>:9090/webcam
```

**Notes :**
- Without the "/webcam" part, you should get an error page containing a redirection link
- To get <server_ip_address>, open a terminal and type "ip addr show" command

## Update configuration files

| File | Description |
| --- | --- |
| [Main.xml](../res/drawer2/Main.xml) | Choose which modules to launch and how to keep app alive |
| [Videos.xml](../res/drawer2/configs/Videos.xml) | Configure video devices and outputs |
| [Servers.xml](../res/drawer2/configs/Servers.xml) | Configure servers |
| [Clients.xml](../res/drawer2/configs/Clients.xml) | Configure clients |
| [Graphics.xml](../res/drawer2/configs/Graphics.xml) | Customize UI and locally display captured video |
| [Colors.xml](../res/drawer2/common/Colors.xml) | Define colors |
| [Fonts.xml](../res/drawer2/common/Fonts.xml) | List fonts files |
| [Images.xml](../res/drawer2/common/Images.xml) | List images files |
| [Strings.xml](../res/drawer2/common/Strings.xml) | Define strings |

## Export more build options

| Option | Value(s) | Description |
| --- | --- | --- |
| DEBUG | gdb | Produce debugging informations for use by gdb |
| DEBUG | asan | Enable address sanitizer |
| DEBUG | secu | Add flags to show security issues |
| LOG_LEVEL | 1, 2, 3 or 4 | Respectively: Error, warning, Info or Debug |

**Notes :**
- For DEBUG option, it is possible to set multiple options simultaneously
  E.g: export DEBUG=gdb,asan,secu
- It is recommended to rebuild the project (make mrproper && make all install)


## Extend mmstreamer
- Implement [Controller.h](../inc/export/Controller.h) - See [mmcontroller project](https://github.com/BoubacarDiene/mmcontroller) for more details
- Update [Main.xml --> Controllers](../res/drawer2/Main.xml) to add your newly created controller

## Build and run unit tests

```
cd mmstreamer_sources/
docker run --privileged -it -u $(id -u) --rm -v $(pwd):/workdir mmstreamer-image:latest

ceedling gcov:all
```

