### Simple low level winelib usb wrapper for the Minipro TL866A/CS, TL866II+, Xgecu T48, T56 and T76 programmers


#### Installing the udev rules
Add the following rule to the udev subsystem:
`sudo cp ./udev/* /etc/udev/rules.d/ && sudo udevadm control --reload-rules && sudo udevadm trigger`


#### How to install:
1. Install wine and libusb-1.0 packages
2. Copy the provided setupapi.dll file in the Minipro or Xgpro folder
3. Run the Minipro.exe or Xgpro.exe using wine
You should install the 32 bit version of LibUsb and create a 32 bit wone prefix.   


#### How to compile:
1. Install wine, wine-devel, libusb-devel-1.0, libudev-devel packages
2. Run make
3. Rename the setup.dll.so file as setupapi.dll and copy this file in the Minipro/Xgpro folder

You should install the 32 bit version of the libusb library (Debian users libusb-1.0-0:i386, Arch users lib32-libusb)


#### Debugging
Run this in a terminal session: `TL_DEBUG=1 wine ./Xgpro.exe` or `TL_DEBUG=1 wine ./Xgpro.exe 2>/dev/null`  
