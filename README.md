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
sudo apt-get install build-essential pkg-config git libusb-1.0-0-dev libqt4-dev
```

### Checkout source code and compile 
```nohighlight
git clone https://github.com/radiomanV/TL866
cd TL866/QT
qmake
make
sudo cp TL866_Updater /usr/local/bin
```

### Udev configuration (recommended)
If you want to access the programmer as a regular user, you'll have to 
configure udev to recognize the programmer and set appropriate access 
permissions.  These are the same rules as are available with 
https://github.com/vdudouyt/minipro.  If you already installed the rules 
there, you don't need to do it again here.

#### Debian/Ubuntu
```nohighlight
sudo cp udev/debian/60-minipro.rules /etc/udev/rules.d/
sudo udevadm trigger
```
You'll also have to add your regular user to the `plugdev` system
group:
```nohighlight
sudo usermod -a -G plugdev YOUR-USER
```
Note that this change will only become effective after your next
login.

#### CentOS 7
```nohighlight
sudo cp udev/centos7/80-minipro.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
```
The CentOS rules file currently make the programmer device writable for 
all users.

#### macOS

Follow instructions for Linux. You'll need libusb and libqt4 through
a package installer. Macports has been used successfully. Udev doesn't
exist for macOS so native IOKit is used instead.
