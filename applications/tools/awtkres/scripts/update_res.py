import awtk_locator as locator
import update_res_app as updater
import compileall
import sys

awtk_root = sys.argv[2]
app_root = sys.argv[3]
compileall.compile_dir(r'./__pycache__')
updater.run(awtk_root, app_root)
