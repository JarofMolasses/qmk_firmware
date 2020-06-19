i have very little idea what is happening

##Update QMK Core and Submodules:
```
git pull https://github.com/qmk/qmk_firmware.git master
git submodule update --recursive --remote
git push origin master
```

##Add LCD library:
```
cop folder lcd_dprentice and add it to qmk_firmware/lib
```

##Rules.mk
```
add path and build targets:

VPATH += $(LIB_PATH)/lcd_dprentice

SRC += 
