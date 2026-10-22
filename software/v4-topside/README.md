## ROV-V4-wired-joystick

### 控制
打开潜器电源开关后，连接手柄，运行`pc_client`文件，连接完成后，可以控制潜器。如果想要查看画面。
### ffmpeg使用说明（默认不是）
在终端内输入
`ffplay -fflags nobuffer -flags low_delay -sync video -framedrop -probesize 32 -analyzeduration 0 -protocol_whitelist file,udp,rtp,tcp,https,crypto -i stream.sdp`
可以查看相机画面。
### GStreamer使用说明（默认是）
在终端内输入
`gst-launch-1.0 -v udpsrc port=5600 caps="application/x-rtp, media=video, encoding-name=H264, payload=96, clock-rate=90000" ! rtpjitterbuffer latency=10 drop-on-latency=true ! rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false`
可以查看相机画面
### 调试
如果想要接收数据后进行绘图，启动`pc_client_plot`程序，只能进行单个数据的绘制