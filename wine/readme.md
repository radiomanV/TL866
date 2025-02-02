## Simple low level winelib usb wrapper for the Minipro TL866A/CS, TL866II+, Xgecu T48, T56 and T76 programmers.


#### Installing the udev rules:
Add the following rules to the udev subsystem:   
```nohighlight
sudo cp ./udev/* /etc/udev/rules.d/ && sudo udevadm control --reload-rules && sudo udevadm trigger  
```
This will grant you permission to run the software as regular user, otherwise you must be a superuser.    

#### How to install:
1. Install `wine`, `libusb` and `libudev` packages   
You should install the 32 bit version of `LibUsb` and `libudev` and create a 32 bit wine prefix:   

```nohighlight
sudo apt install libusb-1.0-0:i386 libudev1:i386   
WINEPREFIX="$HOME/wine32" WINEARCH=win32 wine wineboot      
```

2. Copy the provided setupapi.dll file in the Minipro or Xgpro folder

3. Run the `Minipro.exe`, `Xgpro.exe` or `Xgpro_T76.exe` using `wine`:   
```nohighlight
WINEPREFIX="$HOME/wine32" wine ./Xgpro.exe 2>/dev/null
```
If you already have a default 32 bit `wine` prefix located in `$HOME/.wine` you can use it without creating a new one:   
```nohighlight
wine ./Xgpro.exe 2>/dev/null    
```

#### How to compile:
1. Install `wine`, `wine-devel`, `libusb-1.0-0-dev:i386`, `libudev-dev:i386` packages

2. Run `make hotplug=udev` to compile the `setupapi.dll` using `udev` library for hotplug notifications subsystem.      
Running only `make` will compile the `setupapi.dll` using `libusb` library for handling hotplug events which can    
be useful if `udev` subsystem is not available in your OS.      

4. Rename the compiled `setupapi.dll.so` file as `setupapi.dll` and copy this file in the Minipro/Xgpro folder


#### Debugging:
Running this in a terminal session:    
```nohighligh
TL_DEBUG=1 wine ./Xgpro.exe or TL_DEBUG=1 wine ./Xgpro.exe 2>/dev/null
```   
will dump all usb communication to the console which can be very useful when something goes wrong.   
The `2>/dev/null` will redirect the `stderr` to `/dev/null` thus cancelling the wine debugging messages which can be   
very annoying sometime.   
