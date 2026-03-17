Run

Get test yuv
http://ultravideo.cs.tut.fi/#testsequences

```sh
mapi_venc_demo -c h264 -w 1920 -h 1080 -i ReadySteadyGo_1920x1080_120fps_420_8bit_YUV.yuv
mapi_venc_demo -c h265 -w 1920 -h 1080 -i ReadySteadyGo_1920x1080_120fps_420_8bit_YUV.yuv
mapi_venc_demo -c jpg -w 1920 -h 1080 -i ReadySteadyGo_1920x1080_120fps_420_8bit_YUV.yuv
```

To check encoder result

| file | play |
|---|---|
| ReadySteadyGo.h264 | use StreamEye to play |
| ReadySteadyGo.h265 | $ mv ReadySteadyGo.h265 ReadySteadyGo.265 <br/> $ vlc ReadySteadyGo.265 |

Performance (LPDDR4)

| codec | resolution | time per frame |
|---|---|---|
| 264 | 1920x1080 | 11.0 ms |
| 265 | 1920x1080 | 9.70 ms |
