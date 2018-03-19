# mmstreamer

## 0. Getting started

- [INSTALL](INSTALL.md) : Prepare your environment and build mmstreamer project
- [HOWTO](HOWTO.md)     : Run mmstreamer + update configuration files

## 1. Description

mmstreamer is a v4l2-based media streamer written in C that currently allows to stream captured
video frames through inet and/or unix sockets.

Any client (HTTP browser, VLC, ...) from any device (mobile, computer) can be used to connect
to its server(s).

However, mmstreamer provides its own "Clients" module which can also be used to get video signal
from a remote streamer and display it locally using "Graphics" module.

The application aims to be an easy-to-use application that can be customized to suit your own needs.

### 1.1. Modules

mmstreamer currently includes 4 independent modules :

  - **Videos**   : To capture frames from video devices
  - **Servers**  : To stream frames using unix or inet socket
  - **Clients**  : To get video stream from local or remote streamer
  - **Graphics** : To locally display video

### 1.2. Configuration

It has been designed to make configuring above modules as simple as possible. Unless you have very
specific needs, the following xml resources should be sufficient to use it :

| File | Description |
| --- | --- |
| [Main.xml](res/drawer2/Main.xml) | Choose which modules to launch and how to keep app alive |
| [Videos.xml](res/drawer2/configs/Videos.xml) | Configure video devices and outputs |
| [Servers.xml](res/drawer2/configs/Servers.xml) | Configure servers |
| [Clients.xml](res/drawer2/configs/Clients.xml) | Configure clients |
| [Graphics.xml](res/drawer2/configs/Graphics.xml) | Customize UI and locally display captured video |
| [Colors.xml](res/drawer2/common/Colors.xml) | Define colors |
| [Fonts.xml](res/drawer2/common/Fonts.xml) | List fonts files |
| [Images.xml](res/drawer2/common/Images.xml) | List images files |
| [Strings.xml](res/drawer2/common/Strings.xml) | Define strings |

### 1.3. Control

mmstreamer engine can currently be controlled in two different ways :
- By user using graphics elements on the UI (See [Graphics.xml](res/drawer2/configs/Graphics.xml))
- By an external library without any user action

Such libraries are listed in [Main.xml --> Controllers](res/drawer2/Main.xml) and are simply called
"controllers". See [mmcontroller project](https://github.com/BoubacarDiene/mmcontroller) to get some
ideas about creating a controller.


### 1.4. Usage

A simple usage of mmstreamer is to make it behave like a HTTP streamer by streaming
video signal from your personal computer's webcam toward browsers.

To reach this goal, "Videos" module continuously captures frames from camera which it
can locally display using "Graphics" module  before they are sent to connected clients
through "Servers" module.

As mentioned earlier, all kinds of clients are supported : Browsers, Applications (VLC, ...)
Connections can also be initiated from any type of devices (computers, mobiles).

Finally, note that mmstreamer can have different other usages. Your mind is its only
limit. For example, by enabling "Clients" and "Graphics" / "Servers" modules ("Videos"
disabled), it is possible to get video from a remote streamer (E.g. vlc - See below)
and display it locally and/or stream it over another network interface.
```
 cvlc v4l2:///dev/video0:chroma=mjpg:width=640:height=480 --sout '#std{access=http{mime
 =multipart/x-mixed-replace;boundary=--7b3cc56e5f51db803f790dad720ed50a},mux=mpjpeg,dst
 =127.0.0.1:9090/webcam}'
```
## 2. Goal

The initial purpose of mmstreamer project was to help me have a better understanding
of v4l2 API. It was only composed of one .c file which captured video signal from my
personal webcam and made it available to HTTP clients.

Because I think it can be useful to many people, I will make mmstreamer evolve by 
gradually adding new features taking advantage of possibilities offered by v4l2 API
(among others) :

- [x] Webcam video capture
- [x] Simultaneously capture from several video devices
- [x] Stream video frames through inet and/or unix sockets
- [x] Locally display video on a customizable UI
- [x] Get frames from remote video streamers as a client
- [x] Make mmstreamer extensible and controllable from [external libraries](https://github.com/BoubacarDiene/mmcontroller)
- [ ] Play and stream local video files of any formats
- [ ] Video recording
- [ ] Motion detection
- [ ] Support of TV tuners
- [ ] Encryption to secure the data before transmission

## 3. License

mmstreamer is distributed under the GNU GENERAL PUBLIC LICENSE (GPL) version 2.
Please, see [<http://www.gnu.org/licenses/>](<http://www.gnu.org/licenses/>) for more details.

## 4. Maintenance and support

I do all this on my free time without being paid or asking for any donations. Thus, bug
fixing and updates mainly depend on my availibility.

However, I will soon try to write and provide all necessary documentation so as to make
mmstreamer easier to understand.

## 5. Example : A simple UI designed using [Graphics.xml](res/drawer2/configs/Graphics.xml)

In this example,
- Video capture is performed from my personal computer's webcam using "Videos" module
- Captured frames are displayed locally at top-left of video area by "Graphics" module
- Frames are then streamed over a local inet network using "Servers" module
- "Clients" module connects to that server and displays received data at bottom-right of video area
- "Stop" / "Start" buttons can be used to stop/start client
- It's also possible to take screenshot (Everything diplayed saved to /tmp/mmstreamer/screenshot_xxxx.png)
  and selfy (Last frame displayed in top-left video area is saved to /tmp/mmstreamer/picture_xxxx.png)
- ...

![alt text](https://raw.githubusercontent.com/BoubacarDiene/mmstreamer/master/ScreenshotByDrawer2.png "This is a very simple example of what you could get using mmstreamer")
