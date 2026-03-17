Run

prepare test yuv file

```
cd install/python
python ./resize_jpg.py ../samples/data/dog.jpg 1920 1080 dog_1080p.jpg
ffmpeg -i dog_1080p.jpg -pix_fmt yuv420p dog_1080p.yuv
```

```sh
$ mapi_vproc_demo 0 dog_1080p.yuv vproc_test_out
Save vproc_out_chn_0.chw, w*h(1280*720), image_size(2764800)
Save vproc_out_chn_1.yuv, w*h(640*480), image_size(462848)
```

got vproc_test_out_chn_0.chw & vproc_test_out_chn_1.yuv


3.result
size:1920x1080

case 0:
case 1:
Perf_vproc_1: 1000 loop, total 7263.18 ms, 7.26 ms per frame
Perf_vproc_2: 1000 loop, total 7196.32 ms, 7.20 ms per frame
case 2:
Perf_vproc_1: 1000 loop, total 14380.64 ms, 14.38 ms per frame
Perf_vproc_2: 1000 loop, total 14381.83 ms, 14.38 ms per frame
case 3:
Perf_iproc: 1000 loop, total 7165.49 ms, 7.17 ms per frame
case 4:
Perf_iproc: 1000 loop, total 7200.48 ms, 7.20 ms per frame
Perf_vproc_1: 1000 loop, total 7284.37 ms, 7.28 ms per frame
case 5:
Perf_iproc: 1000 loop, total 7219.46 ms, 7.22 ms per frame
Perf_vproc_1: 1000 loop, total 14406.35 ms, 14.41 ms per frame
Perf_vproc_2: 1000 loop, total 14409.56 ms, 14.41 ms per frame
case 6:
Perf_rotate: 1000 loop, total 12482.44 ms, 12.48 ms per frame
case 7:
Perf_vproc_1: 1000 loop, total 7387.67 ms, 7.39 ms per frame
Perf_rotate: 1000 loop, total 12898.26 ms, 12.90 ms per frame
case 8:
Perf_iproc: 1000 loop, total 7315.34 ms, 7.32 ms per frame
Perf_rotate: 1000 loop, total 12764.74 ms, 12.76 ms per frame
case 9:
Perf_iproc: 1000 loop, total 7323.39 ms, 7.32 ms per frame
Perf_rotate: 1000 loop, total 13244.82 ms, 13.24 ms per frame
Perf_vproc_1: 1000 loop, total 14609.31 ms, 14.61 ms per frame
Perf_vproc_2: 1000 loop, total 14616.43 ms, 14.62 ms per frame
