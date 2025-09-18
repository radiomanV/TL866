# TL866
Open source firmware and utilities for Minipro TL866xx series of chip programmers

This project's scope is dealing with the firmware within the TL866 
itself.  It includes software for dumping, reprogramming, and 
manipulating the firmware.  Schematics and discussion of internal 
operations are also here.

Also here you can find a linux USB wrapper for TL866 and TL866II which make these programmers
native software to work with Wine. The wrapper is located in wine folder.  


## Installation of the TL866 Updater on Linux

### Install build dependencies

#### Debian/Ubuntu
```nohighlight
sudo apt-get install build-essential pkg-config git libusb-1.0-0-dev libudev-dev qt5-default qtbase5-dev 
```
If you want to compile with old QT4 replace `qt5-default qtbase5-dev` with `libqt4-dev`


### Checkout source code and compile 
```nohighlight
git clone https://github.com/radiomanV/TL866
cd TL866/TL866_Updater/QT
qmake
make
sudo cp TL866_Updater /usr/local/bin
```

### Udev configuration (recommended)
If you want to access the programmer as a regular user, you'll have to
configure udev to recognize the programmer and set appropriate access
permissions.

```nohighlight
sudo cp udev/*.rules /etc/udev/rules.d/
sudo udevadm trigger
```
You'll also have to add your regular user to the `plugdev` system
group:
```nohighlight
sudo usermod -a -G plugdev YOUR-USER
```
Note that this change will only become effective after your next
login.

#### macOS

Follow instructions for Linux. You'll need pkg-config, libusb, and libqt4 or above through
a package installer. Macports and Homebrew have been used successfully. Udev doesn't
exist for macOS so native IOKit is used instead.

Homebrew instructions:

```nohighlight
brew install qt libusb pkg-config
git clone https://github.com/radiomanV/TL866
cd TL866/TL866_Updater/QT
qmake
make
cp -R TL866_Updater.app /Applications
```

### InfoicDump utility
This utility can dump the `infoic.dll` and `infoic2plus.dll` to an XML format database.    
More information on this here: [InfoicDump](https://github.com/radiomanV/TL866/tree/master/InfoIcDump#infoicdump)


### WineLib wrapper
This library provide an easy way to make Minipro/Xgpro software work in Linux.        
More information on this here: [Wine](https://github.com/radiomanV/TL866/tree/master/wine32#)
