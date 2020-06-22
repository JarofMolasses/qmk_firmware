# ssopad1.2
Teensy firmware. 

V1.2 Teensy, with underglow (just some LEDs on a perfboard underneath, driven like a backlight)
![V1.2 Teensy, underglow](https://user-images.githubusercontent.com/33560291/85184022-05ea1000-b243-11ea-93e6-429593b7d0d9.jpg)

### Note: on PCB v1.2, column 0 is connected to pin D6 - the built-in LED pin. oops 
The config.h is currently set up for the following mods: 
- cut the trace to pin D6 
- jumper column 0 to new pin: C6 

Keyboard Maintainer: [molasses](https://github.com/JarofMolasses)  

Make example for this keyboard (after setting up your build environment):

    make ssopad1.2:default

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).
