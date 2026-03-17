Run

prepare vo.yuv

```
python resize_jpg.py ../samples/data/cat.jpg 1280 720 cat_720p.jpg
ffmpeg -i cat_720p.jpg -pix_fmt yuv420p vo.yuv
```

run

```
sample_dsi
mapi_disp_demo 0 vo.yuv
mapi_disp_demo 2 vo.yuv vo.yuv
```
