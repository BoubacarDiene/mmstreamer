# Changelog : Main changes

## v1.2
```
- Http server: Always use the correct IP address in redirect link
- Allow to disable assert() in release version
```

## v1.1
```
- Http client : Fix deadlock when remote server is brutally stopped
- Http client : Fix video artifacts by skipping incomplete mjpeg frames and those that do
  not contain a boundary
```

## v1.0
```
- Video acquisition from webcam, rear cameras, ...
- Simultaneous support of several cameras
- Stream video signal through inet and/or unix sockets
- Locally display video on a customizable UI
- Easily configurable: some xml files to fill in and that's all!
- Can be used as a client to get frames from remote video streamers
- Extensible and controllable from UI and/or external libraries
- Possible to start/stop each module ("Videos", "Servers", "Clients", "Graphics") anytime
- Take screenshots and/or save content of a video area in 3 different formats: PNG, JPG, BMP
```
