Run

Get test yuv
http://ultravideo.cs.tut.fi/#testsequences

```sh
mapi_vdec_demo -c h264  -i ReadySteadyGo_1920x1080_120fps_420_8bit_YUV.h264
mapi_vdec_demo -c h265  -i ReadySteadyGo_1920x1080_120fps_420_8bit_YUV.h265
mapi_vdec_demo -c jpg  -i ReadySteadyGo_1920x1080_120fps_420_8bit_YUV.jpg
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
