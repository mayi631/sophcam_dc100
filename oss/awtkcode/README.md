# AWTK = Toolkit AnyWhere
# git clone source:
1.awtk:
source:https://github.com/zlgopen/awtk.git
inital clone from:
branch:1.6
commit:f73daba26421c9c3cff5f375bef634a30b3475cc
2.awtk-linux-fb:
source:https://github.com/zlgopen/awtk-linux-fb.git
initial clone from:
branch:remote/origin/master
commit:ae44405624c743931d538e412fdb9817e0cea81a

# HOW TO BUILD FOR TARGET CVI BOARD
   cd awtk-linux-fb
   export SDK_PATH=the root path of SDK(ex./home/garry.xin/GWei)
   scons -j3
The target lib is located in awtk-linux-fb/build/bin

# HOW TO BUILD FOR TARGET LINUX(X86_64bit)
   cd awtk
   scons -j3
The target lib is located in awtk/lib
