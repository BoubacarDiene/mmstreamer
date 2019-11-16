# Howto

## 1. How to start mmstreamer

### 1.1. Launch binary (as root)
```
$ cd mmstreamer_sources/
$ sudo ./out/mmstreamer/bin/mmstreamer-<x.y>
```
or in debug mode :
```
$ sudo ./out/mmstreamer/bin/mmstreamer-<x.y>.dbg
```

**Notes :**
- <x.y> has to be replaced with the current version of the binary.
- Another path to Main.xml config file can be specified using -c option\
```
  E.g.: $ ./out/mmstreamer/bin/mmstreamer-<x.y> -f /tmp/Main.xml
```
- In case a "bitsPerPixel" different from the active framebuffer's depth is specified in
  Graphics.xml, mmstreamer will try to change depth. To make it work, "root" permission
  is required (E.g. Run mmstreamer as root))
- If mmstreamer failed to start, please, see [TROUBLESHOOT](TROUBLESHOOT.md)

### 1.2. Connect to inet server

From your local computer :
```
http://127.0.0.1:9090/webcam or http://localhost:9090/webcam
```
From a remote device (phone, computer, ...) connected to same network as the server :
```
http://<your_ip_address>:9090/webcam
```

**Notes :**
- Without the "/webcam" part, you get an error page containing a redirection link
- To get <your_ip_address>, open a terminal and type "ifconfig" command

## 2. How to update mmstreamer's configuration

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

## 3. How to enable debug mode

### 3.1. Open Makefile.in
```
$ cd mmstreamer_sources/
$ vi build/Makefile.in
```

### 3.2. Update macros
```
DEBUG := yes
LOG_LEVEL := <1 - 4>
```

### 3.3. Relaunch build
```
$ make all install
```

## 4. How to extend mmstreamer
- Implement [Controller.h](../inc/export/Controller.h) - See [mmcontroller project](https://github.com/BoubacarDiene/mmcontroller) for more details
- Update [Main.xml --> Controllers](../res/drawer2/Main.xml) to add your newly created controller

## 5. How to build and run unit tests

### 5.1. Build and load docker image
```
$ cd mmstreamer_sources/
$ docker build -t  mmstreamer-image ci/
$ docker run -it -u $(id -u) --rm -v $(pwd):/workdir mmstreamer-image:latest
```

### 5.2. Run tests
```
$ ceedling

To also display code coverage summary, use:
$ ceedling gcov:all
```

