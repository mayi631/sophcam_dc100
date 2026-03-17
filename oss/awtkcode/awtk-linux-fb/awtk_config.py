import os
import platform
import shutil

OS_NAME = platform.system()

def joinPath(root, subdir):
  return os.path.normpath(os.path.join(root, subdir))

def lcd_devices_is_egl(lcd_devices):
  if lcd_devices =='egl_for_fsl' or lcd_devices =='egl_for_x11' or lcd_devices =='egl_for_gbm' :
    return True
  return False

CWD=os.path.normpath(os.path.abspath(os.path.dirname(__file__)));

TK_LINUX_FB_ROOT = CWD
TK_ROOT          = joinPath(TK_LINUX_FB_ROOT, '../awtk')
TK_SRC           = joinPath(TK_ROOT, 'src')
TK_3RD_ROOT      = joinPath(TK_ROOT, '3rd')
# GTEST_ROOT       = joinPath(TK_ROOT, '3rd/gtest/googletest')

BUILD_DIR        = joinPath(TK_LINUX_FB_ROOT, 'build')
BIN_DIR          = joinPath(BUILD_DIR, 'bin')
LIB_DIR          = joinPath(BUILD_DIR, 'lib')
VAR_DIR          = joinPath(BUILD_DIR, 'var')
AWTK_PORT        = joinPath(TK_LINUX_FB_ROOT, './awtk-port')

# TK_DEMO_ROOT     = joinPath(TK_ROOT, 'demos')

# LCD_DIR        = joinPath(TK_LINUX_FB_ROOT, 'awtk-port/lcd_linux')
# INPUT_DIR      = joinPath(TK_LINUX_FB_ROOT, 'awtk-port/input_thread')

# lcd devices
LCD_DEVICES='fb'
# LCD_DEVICES='drm'
# LCD_DEVICES='egl_for_fsl'
# LCD_DEVICES='egl_for_x11'
# LCD_DEVICES='egl_for_gbm'

NANOVG_BACKEND=''
VGCANVAS='NANOVG'
if LCD_DEVICES =='fb' or LCD_DEVICES =='drm' :
  LCD='LINUX_FB'
  NANOVG_BACKEND='AGGE'
elif lcd_devices_is_egl(LCD_DEVICES) :
  LCD='FB_GL'
  NANOVG_BACKEND='GLES2'

INPUT_ENGINE='null'
#INPUT_ENGINE='spinyin'
#INPUT_ENGINE='t9'
#INPUT_ENGINE='t9ext'
#INPUT_ENGINE='pinyin'

COMMON_CCFLAGS=' -DHAS_STD_MALLOC -DHAS_STDIO -DHAS_FAST_MEMCPY -DWITH_VGCANVAS -DLINUX'
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DLOAD_ASSET_WITH_MMAP=1 -DWITH_SOCKET=1 -DWITH_NULL_IM'
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_ASSET_LOADER -DWITH_FS_RES -DHAS_GET_TIME_US64=1 '
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DSTBTT_STATIC -DSTB_IMAGE_STATIC -DWITH_STB_IMAGE -DWITH_STB_FONT'
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_LCD_CLEAR_ALPHA -DWITHOUT_WINDOW_ANIMATORS -DWITHOUT_DIALOG_HIGHLIGHTER -DWITHOUT_WIDGET_ANIMATORS'
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_BITMAP_BGRA'

if LCD_DEVICES =='fb' :
  COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_NANOVG_AGGE -DWITH_LINUX_FB '
elif LCD_DEVICES =='drm' :
  COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_NANOVG_AGGE -DWITH_LINUX_DRM '
elif lcd_devices_is_egl(LCD_DEVICES) :
  COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_GPU_GL -DWITH_GPU_GLES2 -DWITH_GPU -DWITH_LINUX_EGL '



if INPUT_ENGINE == 't9':
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_IME_T9 '
elif INPUT_ENGINE == 't9ext' :
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_IME_T9EXT'
elif INPUT_ENGINE == 'pinyin' :
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_IME_PINYIN '
elif INPUT_ENGINE == 'spinyin' :
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_IME_SPINYIN '
elif INPUT_ENGINE == 'null' :
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_IME_NULL '

GRAPHIC_BUFFER='default'
#GRAPHIC_BUFFER='jzgpu'
#if GRAPHIC_BUFFER == 'jzgpu':
#  COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_JZGPU'

#only for c compiler flags
COMMON_CFLAGS=''
COMMON_CFLAGS=COMMON_CFLAGS+' -std=gnu99 '

OS_LIBS=[]
OS_LIBPATH=[]
OS_CPPPATH=[]
OS_LINKFLAGS=''
OS_SUBSYSTEM_CONSOLE=''
OS_SUBSYSTEM_WINDOWS=''
OS_FLAGS='-Wall -Os -fno-strict-aliasing '
#OS_FLAGS='-g -Wall -Os -mfloat-abi=hard -fno-strict-aliasing '

#for build tslib
#TSLIB_INC_DIR=joinPath(TK_LINUX_FB_ROOT, '3rd/tslib/src')
#TSLIB_LIB_DIR=joinPath(TK_LINUX_FB_ROOT, '3rd/tslib/src/.libs')

#for prebuild tslib
#TSLIB_LIB_DIR='/opt/28x/tslib/lib'
#TSLIB_INC_DIR='/opt/28x/tslib/include'
#TOOLS_PREFIX='/opt/28x/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-linux-'
#TOOLS_PREFIX='/opt/poky/1.7/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-'

TSLIB_LIB_DIR=''
TSLIB_INC_DIR=''
#TOOLS_PREFIX='/opt/v3s/mango/tools/external-toolchain/bin/arm-linux-gnueabi-'
#OS_FLAGS='-std=gnu99 -mthumb -mabi=aapcs-linux -mlittle-endian -fdata-sections -ffunction-sections -mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp '

#for pc build
#SDK_PATH = os.environ['SDK_PATH']
CROSS_COMPILE = os.environ.get('CROSS_COMPILE')
#TOOLS_PREFIX = SDK_PATH + '/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-'
if CROSS_COMPILE is not None:
    TOOLS_PREFIX = CROSS_COMPILE

TOUCHPAD_INCLUDE=[]
FB_TDE_INCLUDE=[]
CSI2D_INCLUDE=[]
CSI2D_LIBPATH=[]
TOUCHPAD = os.environ.get('TOUCHPAD')
RISCV = os.environ.get('RISCV')
SDK_ROOT = os.environ.get('SDK_ROOT')
PLATFORM_DIR = os.environ.get('PLATFORM_DIR')
CVILIB_DIR = os.environ.get('CVILIB_DIR')
if TOUCHPAD == 'y':
    print("touchpad=======================================")
    print(SDK_ROOT)
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DCONFIG_TOUCHPAD_ON '
    TOUCHPAD_INCLUDE =  joinPath(SDK_ROOT, 'components/hal/include')
    print(TOUCHPAD_INCLUDE)
if RISCV == 'y':
    print("RISCV=======================================")
    COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DCONFIG_RISCV_ON ' + ' -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d '
    CSI2D_PATH='oss/csi2d/'
    CSI2D_INCLUDE =  joinPath(SDK_ROOT, CSI2D_PATH + 'include')
    CSI2D_LIBPATH = [joinPath(SDK_ROOT, CSI2D_PATH + 'lib')]
    print(CSI2D_INCLUDE)
    print(CSI2D_LIBPATH)
else:
    COMMON_CFLAGS = COMMON_CFLAGS+'-mfpu=neon -ftree-vectorize -O3 -fopt-info-vec'

BOARD = os.environ.get('BOARD')
if BOARD == 'CV184XH':
    print("BOARD=======================================CV184XH")
    COMMON_CCFLAGS += ' -DBOARD=\\"CV184XH\\" '
else:
    print("BOARD=======================================CV181X")
    COMMON_CCFLAGS += ' -DBOARD=\\"CV181X\\" '

FB_DEPTH = os.environ.get('FB_DEPTH')
if FB_DEPTH == '16':
  print("FB_DEPTH=======================================16")
  COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DFB_DEPTH=16 '
else:
  print("FB_DEPTH=======================================32")
  COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DFB_DEPTH=32 '

FB_FORMAT = os.environ.get('FB_FORMAT')
if FB_FORMAT == 'ARGB4444':
    print("FB_FORMAT=======================================ARGB4444")
    COMMON_CCFLAGS += ' -DFB_FORMAT=\\"ARGB4444\\" '
else:
    print("FB_FORMAT=======================================ARGB888")
    COMMON_CCFLAGS += ' -DFB_FORMAT=\\"ARGB888\\" '

FB_BUFFER = os.environ.get('FB_BUFFER')
if FB_BUFFER == '2':
  print("FB_BUFFER=======================================2")
  COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DFB_BUFFER=2 '
else:
  print("FB_BUFFER=======================================1")
  COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DFB_BUFFER=1 '

FB_TDE_ENABLE = os.environ.get('FB_TDE_ENABLE', 'n').lower()
if FB_TDE_ENABLE == 'y':
  print("FB_TDE_ENABLE=======================================y")
  COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DFB_TDE_ENABLE '
  FB_TDE_INCLUDE =  joinPath(PLATFORM_DIR, 'cvi_mpi/include')
else:
  print("FB_TDE_ENABLE=======================================n")

#TOOLS_PREFIX=''
#TSLIB_LIB_DIR=''
TARGET_ARCH = platform.architecture();

CC=TOOLS_PREFIX+'gcc',
CXX=TOOLS_PREFIX+'g++',
LD=TOOLS_PREFIX+'g++',
AR=TOOLS_PREFIX+'ar',
RANLIB=TOOLS_PREFIX+'ranlib',
STRIP=TOOLS_PREFIX+'strip',
OS_LIBS = ['stdc++', 'pthread', 'rt', 'm', 'dl']

#for android
#TSLIB_LIB_DIR=''
#TSLIB_INC_DIR=''
#TOOLS_PREFIX='/opt/android-ndk-r20b/toolchains/llvm/prebuilt/linux-x86_64/bin/'
#TOOLS_PREFIX='/Users/jim/android/android-ndk-r21d/toolchains/llvm/prebuilt/darwin-x86_64/bin/'
#CC=TOOLS_PREFIX+'armv7a-linux-androideabi16-clang'
#CXX=TOOLS_PREFIX+'armv7a-linux-androideabi16-clang++'
#LD=TOOLS_PREFIX+'arm-linux-androideabi-ld'
#AR=TOOLS_PREFIX+'arm-linux-androideabi-ar'
#STRIP=TOOLS_PREFIX+'arm-linux-androideabi-strip'
#RANLIB=TOOLS_PREFIX+"arm-linux-androideabi-ranlib"
#OS_LINKFLAGS=' -Wl,--allow-multiple-definition '
#OS_LIBS = ['stdc++', 'm']
#OS_FLAGS='-Wall -Os -DFB_DEVICE_FILENAME=\\\"\"/dev/graphics/fb0\\\"\" '

OS_LINKFLAGS= OS_LINKFLAGS + ' -Wl,-rpath=./bin -Wl,-rpath=./ '

if LCD_DEVICES =='drm' :
  #for drm
  OS_FLAGS=OS_FLAGS + ' -DWITH_LINUX_DRM=1 -I/usr/include/libdrm '
  OS_LIBS=OS_LIBS + ['drm']
elif LCD_DEVICES =='egl_for_fsl':
  #for egl for fsl
  OS_FLAGS=OS_FLAGS + ' -DEGL_API_FB '
  OS_LIBS=OS_LIBS + [ 'GLESv2', 'EGL']
elif LCD_DEVICES =='egl_for_x11' :
  #for egl for fsl
  OS_FLAGS=OS_FLAGS + ' -fPIC '
  OS_LIBS=OS_LIBS + [ 'X11', 'EGL', 'GLESv2' ]
elif LCD_DEVICES =='egl_for_gbm' :
  #for egl for gbm
  OS_CPPPATH += ['/usr/include/libdrm', '/usr/include/GLES2']
  OS_LIBS=OS_LIBS + [ 'drm', 'gbm', 'EGL', 'GLESv2' ]

COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DLINUX -DHAS_PTHREAD -fPIC '
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_DATA_READER_WRITER=1 '
COMMON_CCFLAGS=COMMON_CCFLAGS+' -DWITH_EVENT_RECORDER_PLAYER=1 '
COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DWITH_WIDGET_TYPE_CHECK=1 '

if TSLIB_LIB_DIR != '':
  COMMON_CCFLAGS = COMMON_CCFLAGS + ' -DHAS_TSLIB '

CFLAGS=COMMON_CFLAGS
LINKFLAGS=OS_LINKFLAGS;
LIBPATH=[LIB_DIR, BIN_DIR] + OS_LIBPATH + CSI2D_LIBPATH
CCFLAGS=OS_FLAGS + COMMON_CCFLAGS

# STATIC_LIBS =['awtk_global', 'extwidgets', 'widgets', 'awtk_linux_fb', 'base', 'gpinyin', 'streams', 'conf_io', 'hal', 'csv', 'compressors', 'miniz', 'ubjson', 'tkc_static', 'linebreak', 'mbedtls', 'fribidi']
if RISCV == 'y':
  STATIC_LIBS =['csi2d', 'awtk_global', 'extwidgets', 'widgets', 'base', 'tkc_static']
else:
  STATIC_LIBS =['awtk_global', 'extwidgets', 'widgets', 'base', 'tkc_static']

if TSLIB_LIB_DIR != '':
  SHARED_LIBS=['awtk', 'ts'] + OS_LIBS;
else:
  SHARED_LIBS=['awtk'] + OS_LIBS;

if VGCANVAS == 'NANOVG':
  if LCD_DEVICES =='fb' or LCD_DEVICES =='drm' :
    STATIC_LIBS = STATIC_LIBS + ['nanovg-agge', 'agge', 'nanovg']  + OS_LIBS
    AWTK_DLL_DEPS_LIBS = ['nanovg-agge', 'agge', 'nanovg'] + OS_LIBS
  elif lcd_devices_is_egl(LCD_DEVICES) :
    CCFLAGS += ' -DWITH_NANOVG_GLES2 -DWITH_NANOVG_GL -DWITH_NANOVG_GPU '
    STATIC_LIBS = STATIC_LIBS + ['glad', 'nanovg']  + OS_LIBS
    AWTK_DLL_DEPS_LIBS = ['glad', 'nanovg'] + OS_LIBS

# OS_WHOLE_ARCHIVE =' -Wl,--whole-archive -lfribidi -lawtk_global -lextwidgets -lwidgets -lawtk_linux_fb -lbase -lgpinyin -ltkc_static -lstreams -lconf_io -lhal -lcsv -lubjson -lcompressors -lmbedtls -lminiz -llinebreak -Wl,--no-whole-archive'
OS_WHOLE_ARCHIVE =' -Wl,--whole-archive -lawtk_linux_fb -lawtk_global -lextwidgets -lwidgets -lbase -ltkc_static -Wl,--no-whole-archive'
LIBS=STATIC_LIBS

CPPPATH=[TK_ROOT,
  TK_SRC,
  TK_3RD_ROOT,
  # LCD_DIR,
  # INPUT_DIR,
  joinPath(TK_SRC, 'ext_widgets'),
  joinPath(TK_SRC, 'custom_widgets'),
  joinPath(TK_ROOT, 'tools'),
  joinPath(TK_3RD_ROOT, 'agge'),
  joinPath(TK_3RD_ROOT, 'agg/include'),
  joinPath(TK_3RD_ROOT, 'nanovg'),
  joinPath(TK_3RD_ROOT, 'nanovg/gl'),
  joinPath(TK_3RD_ROOT, 'nanovg/base'),
  # joinPath(TK_3RD_ROOT, 'mbedtls/include'),
  # joinPath(TK_3RD_ROOT, 'mbedtls/3rdparty/everest/include'),
  joinPath(TK_3RD_ROOT, 'fribidi'),
  joinPath(TK_3RD_ROOT, 'nanovg/base'),
  joinPath(TK_3RD_ROOT, 'libunibreak'),
  TOUCHPAD_INCLUDE,
  CSI2D_INCLUDE,
  FB_TDE_INCLUDE,
  # joinPath(TK_3RD_ROOT, 'gpinyin/include'),
  # joinPath(TK_3RD_ROOT, 'gtest/googletest'),
  # joinPath(TK_3RD_ROOT, 'gtest/googletest/include'),
  ] + OS_CPPPATH

if TSLIB_LIB_DIR != '':
  LIBS = ['ts'] + LIBS
  LIBPATH = [TSLIB_LIB_DIR] + LIBPATH;
  CPPPATH = [TSLIB_INC_DIR] + CPPPATH;

os.environ['LCD'] = LCD
os.environ['LCD_DEVICES'] = LCD_DEVICES
os.environ['TARGET_ARCH'] = 'arm'
os.environ['BIN_DIR'] = BIN_DIR;
os.environ['LIB_DIR'] = LIB_DIR;
os.environ['TK_ROOT'] = TK_ROOT;
os.environ['CCFLAGS'] = CCFLAGS;
os.environ['VGCANVAS'] = VGCANVAS
os.environ['INPUT_ENGINE'] = INPUT_ENGINE;
os.environ['TSLIB_LIB_DIR'] = TSLIB_LIB_DIR;
os.environ['NANOVG_BACKEND'] = NANOVG_BACKEND;
os.environ['TK_3RD_ROOT'] = TK_3RD_ROOT;
# os.environ['GTEST_ROOT'] = GTEST_ROOT;
os.environ['TOOLS_NAME'] = '';
os.environ['GRAPHIC_BUFFER'] = GRAPHIC_BUFFER;
os.environ['WITH_AWTK_SO'] = 'true'

if LCD_DEVICES =='fb' or LCD_DEVICES =='drm' :
  os.environ['NATIVE_WINDOW'] = 'raw';
elif lcd_devices_is_egl(LCD_DEVICES) :
  os.environ['NATIVE_WINDOW'] = 'fb_gl';


os.environ['OS_WHOLE_ARCHIVE'] = OS_WHOLE_ARCHIVE;
os.environ['AWTK_DLL_DEPS_LIBS'] = ';'.join(AWTK_DLL_DEPS_LIBS)
os.environ['STATIC_LIBS'] = ';'.join(STATIC_LIBS)

def has_custom_cc():
    return True

def copySharedLib(src, dst, name):
  src = os.path.join(src, 'build/lib'+name+'.so')
  src = os.path.normpath(src);
  dst = os.path.normpath(dst);

  if os.path.dirname(src) == dst:
      return

  if not os.path.exists(src):
    print('Can\'t find ' + src + '. Please build '+name+'before!')
  else:
    if not os.path.exists(dst):
        os.makedirs(dst)
    shutil.copy(src, dst)
    print(src + '==>' + dst);
