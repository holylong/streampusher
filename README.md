# 使用基于ffmpeg的推流器

## build
```
    vcpkg install ffmpeg:x64-windows-static opencv:x64-windows-static
    build_ninja_vcpkg.bat

    or install opencv self 设置你自己的opencv目录
    build_ninja.bat
```

## use
```
    CapturePusher.exe
```

参考obs代码使用的画质

H.264有四种画质级别,分别是baseline, extended, main, high：
1、Baseline Profile：基本画质。支持I/P 帧，只支持无交错（Progressive）和CAVLC；
2、Extended profile：进阶画质。支持I/P/B/SP/SI 帧，只支持无交错（Progressive）和CAVLC；(用的少)
3、Main profile：主流画质。提供I/P/B 帧，支持无交错（Progressive）和交错（Interlaced）， 也支持CAVLC 和CABAC 的支持；
4、High profile：高级画质。在main Profile 的基础上增加了8x8内部预测、自定义量化、 无损视频编码和更多的YUV 格式；

H.264 Baseline profile、Extended profile和Main profile都是针对8位样本数据、4:2:0格式(YUV)的视频序列。
在相同配置情况下，High profile（HP）可以比Main profile（MP）降低10%的码率。 
根据应用领域的不同，Baseline profile多应用于实时通信领域，Main profile多应用于流媒体领域，High profile则多应用于广电和存储领域。


high 转 main
```
fmpeg -i high.mp4 -profile:v main -level 3.0 main.mp4
```