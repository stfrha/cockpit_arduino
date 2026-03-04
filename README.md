
Requires the following libraries installed in Arduino IDE:
https://github.com/stfrha/cockpit_arduino_library
https://github.com/MHeironimus/ArduinoJoystickLibrary


Multiple instances of this Arduino on the same computer requires unique board definitions. Below explains how to do it:

To select local cockpit board definitions: 
- Create the following path: C:\Users\{USER}\Documents\Arduino\hardware\{Your board group name}\avr\
- Copy all files from C:\Users\{USER}\AppData\Local\Arduino15\packages\arduino\hardware\avr to this directory
- Replace the file boards.txt (in this directory) with the file of same name in this repository.
- Change micro1 or micro2 to get a unique name if multiple devices is needed
- Change micro1.name=F18cCockpitDdis to a name of your liking
- change micro1.build.usb_product=" a name of your liking "
- Increase the micro1.build.pid to something uniuqe (increase with one, for instance)

Start Arduino IDE. The boards should now be avaialble in the menu: Tools/Board: xxxx/

