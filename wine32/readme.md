> ⚠️ **Warning**
>
> This wrapper is no longer maintained. It targets 32-bit Wine environments and is provided *as-is* for users   
> on systems/Wine versions that still support 32-bit.   
> For the actively maintained replacement, see the new wrapper [here](https://github.com/radiomanV/TL866/raw/refs/heads/master/wine64#) 

# Simple low level winelib usb wrapper for the Minipro TL866A/CS, TL866II+, Xgecu T48, T56 and T76 programmers.


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

2. Run `make` to compile the `setupapi.dll` using `libusb` library for hotplug events.
You can also use `libudev` to compile the `setupapi.dll` using `libudev` library for handling hotplug events which can    
be useful if `libusb` hotplug events arenot available in your OS by running `make hotplug=udev`   

3. Copy the resulted  `setupapi.dll` file in the Minipro/Xgpro folder


#### Debugging:
Running this in a terminal session:    
```nohighlight
TL_DEBUG=1 wine ./Xgpro.exe or TL_DEBUG=1 wine ./Xgpro.exe 2>/dev/null
```   
will dump all usb communication to the console which can be very useful when something goes wrong.   
The `2>/dev/null` will redirect the `stderr` to `/dev/null` thus cancelling the wine debugging messages which can be   
very annoying sometime.   

You can also use `gdb` GNU debugger for debugging:   
```nohighlight
WINEDEBUG=fixme-all,-ALL WINELOADERNOEXEC=1 gdb -q --args wine ./Xgpro.exe
(gdb) run
```

## Issues on some Linux distros
Some Linux distros will compile its system libraries with [SSE](https://en.wikipedia.org/wiki/Streaming_SIMD_Extensions) instruction set enabled to increase the performance.   
This is a good thing but, using a `Libusb` compiled with SSE will crash our wine wrapper library because the SSE intruction   
set requires the memory address to be 16-byte aligned and, our `setupapi.dll` uses 4 bytes stack alignment because   
the 32bit Windows software is also using this 4 byte alignment. See issue [#51](https://github.com/radiomanV/TL866/issues/51).      

This is the case for Arch/ManjaroGentoo and its derivatives and perhaps other distros i have not tested yet.   
In this case we must compile our Libusb and use it to link the `setupapi.dll` against it.   

**_This is for Arch/Manjaro users, Gentoo users please adapt the needed packages as i don't test it in Gentoo._**   

First install the necessary tools:
```
sudo pacman -S autoconf automake libtool git base-devel lib32-libusb
```

Then grab these git repos:
```
mkdir build_temp && cd build_temp
git clone https://github.com/radiomanV/TL866.git && cd TL866/wine
git clone https://github.com/libusb/libusb.git && cd libusb
```
Now configure and build the static 32bit `libUsb`:
```
./bootstrap.sh  && ./configure CFLAGS="-m32 -mstackrealign -fPIC" --prefix="$(dirname "$(pwd)")" --disable-shared && make install && cd ../
```
And finally (assuming the wine is installed) compile our `setupapi.dll`  against the static `LibUsb`:
```
make CFLAGS="-Iinclude/ -m32 -mstackrealign" LIBS="-Wl,--whole-archive lib/libusb-1.0.a -Wl,--no-whole-archive -ludev"
```
You can automate this by using the accompanying `build_static.sh` script    
which will invoke all above commands and build the statically linked `setupapi.dll`   
```
./build_static.sh
```

If all was okay we should have the `setupapi.dll` in the current directory.   
You can always download the already compiled [setupapi.dll](https://github.com/radiomanV/TL866/raw/refs/heads/master/wine32/setupapi.dll) from this git repository, but sometimes depending on your Linux distro   
and `glib` version it will not work and you must recompile it.   
