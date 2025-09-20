# Xgpro/Minipro Cross-Architecture USB Wrapper (Broker + Shim)

 **What is this?**  
A modern, cross-architecture replacement for the legacy 32-bit [WineLib wrapper](https://github.com/radiomanV/TL866/tree/master/wine32#).  
It splits the old `setupapi.dll` logic into:   

- **A native USB broker** (Linux, 64-bit or 32-bit, any arch) that talks to LibUSB.   

- **A native Windows shim DLL** that runs inside the Windows app (Xgpro/Minipro) and proxies USB calls to the broker.   
  
  **Why?**  

- Works on **x86_64, aarch64, armhf**, etc.  

- No 32-bit Wine and 32-bit libs requirement.  

- Avoids SSE/stack alignment pitfalls from the legacy approach.  

- Cleaner debugging and isolation.

---

## Supported programmers

- **Minipro TL866A/CS**
- **XGecu TL866II+**
- **XGecu T48 / T56 / T76**

## Features

- **Cross-arch**: broker builds natively for your host (x86_64, arm64, armhf, etc.)
- **Broker/Shim split**: minimal 32-Bit native Windows DLL
- **Socket protocol**: shim to broker IPC over local TCP socket
- **Hotplug**: libusb hotplug events
- **Verbose tracing**: single env var to dump full TX/RX
- **Drop-in**: works with stock `Xgpro.exe` / `Xgpro_T76.exe` / `Minipro.exe`

---

## Requirements

### Host (Linux)

- `libusb-1.0` (runtime; `-dev` for building broker)

- `wine` or `wine64` (only runtime, no `-dev` packages needed)   

- Build tools: `gcc`, `i686-w64-mingw32-gcc`, `make`, `pkg-config`
  
  ```bash
  # Debian / Ubuntu / Mint
  sudo apt install -y wine winetricks libusb-1.0-0
  sudo apt install -y build-essential pkg-config libusb-1.0-0-dev \
      mingw-w64 gcc-mingw-w64-i686
  
  # Arch / Manjaro
  sudo pacman -S --needed wine winetricks libusb
  sudo pacman -S --needed gcc make pkgconf mingw-w64-gcc
  
  # Fedora / RHEL
  sudo dnf install -y wine winetricks libusb1
  sudo dnf install -y gcc make pkgconfig libusb1-devel \
      mingw32-gcc mingw32-binutils mingw32-headers mingw32-crt
  
  # openSUSE 
  sudo zypper in -y wine winetricks libusb-1_0-0
  sudo zypper in -y gcc make pkg-config libusb-1_0-devel mingw32-gcc
  ```

### Wine / App

- App: `Xgpro.exe`, `Xgpro_T76.exe`, or `Minipro.exe`
- The **shim DLL** must be placed alongside the app (see install)

---

## Install

### 1) Clone git repository

```bash
git clone https://github.com/radiomanV/TL866.git && cd TL866
```

### 2) Udev rules (recommended)

```bash
sudo cp ./udev/* /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

This grants non-root access to the programmers.

### 3) Build the broker (native)

```bash
cd wine64
make broker    # produces usb-broker
```

### 4) Build the Windows component

```bash
make windows   # produces launcher.exe and shim.dll
```

#### A simple make will compile all three components above.

```bash
make
```

You can use the provided `shim.dll`, `launcher.exe`, and `usb-broker`.   
`shim.dll` and `launcher.exe` do not need recompilation (they’re prebuilt natively), but  
`usb-broker` may need to be rebuilt depending on your Linux distribution or architecture.

### 5) Place the shim, launcher and usb-broker next to the app

Copy the resulting files `usb-broker` `shim.dll`  `launcher.exe`  and (optionally) `run.sh`to the application directory.    
Mark `usb-broker` and `run.sh` as executable:

```bash
chmod +x usb-broker run.sh
```

---

## Quick start

```
# Minimal, without any script:
cd /path/to/app/
./usb-broker --quiet & wine launcher.exe 2>/dev/null

# Or you can use the provided run.sh script, which can be customized via
# explicit environment variables:
BROKER_PORT=35866
APP_PATH="/path/to/launcher.exe"
TARGET_EXE="Xgpro.exe"
SHIM_DLL="shim.dll"
USB_BROKER_PATH="/path/to/xgecu-usb-broker"
WINECMD="wine64"

# This will start the broker (if needed) on the default port 35866 and
# run the launcher with the default Xgpro.exe name.
run.sh

# This will overide the default Xgpro.exe name:
TARGET_EXE=Minipro.exe ./run.sh
TARGET_EXE=Xgpro_T76.exe ./run.sh

# This will overide the default IPC port:
BROKER_PORT=35000 ./run.sh
```

- The script will:
  - start the **broker** (if not running),
  - launch Wine with the shimmed app,
  - wire them together over the configured socket.

Works without a 32-bit prefix; you can use `~/.wine64`, default prefix or any Wine prefix you prefer.

The USB broker can also be run standalone for debugging. It accepts the following arguments:   

```
--port <N>     (listens on port N)
--quiet        (reduced logging)
--no-exit      (do not exit when there are no clients)
```

The USB broker supports multiple concurrent connections and can handle up to 64 USB devices.  
Unless `--no-exit` is specified, the USB broker will automatically exit after a timeout.

---

## Configuration (env vars)

These are read by `run.sh` and/or the shim:

- `BROKER_PORT`: TCP port for shim-broker IPC (default: `35866`)
- `APP_PATH`: path to the Windows app launcher (optional)
- `TARGET_EXE`: the Windows binary to run (e.g. `Xgpro.exe`)
- `SHIM_DLL`: shim file name in the app folder (e.g. `shim.dll`)
- `USB_BROKER_PATH`: full path to the broker binary
- `WINECMD`: `wine` or `wine64` (default: `wine`)
- `TL_DEBUG`: set to `1` for verbose USB/protocol logs

---

## Troubleshooting

- **Device not found / permission denied**
  
  - Recheck udev rules, unplug/replug, verify you’re in the correct groups (e.g., `plugdev` on some distros).

- **Shim cannot find broker**
  
  - Verify `BROKER_PORT` and that broker is running; try `USB_BROKER_PATH` absolute path.

- **Socket conflicts**
  
  - Change `BROKER_PORT`.

- **Black toolbar when theme is active under Wine**
  
  - Install native comctl32 v5.80 or disable the Wine theme.   
    
    ```bash
    winetricks comctl32 && wineboot
    ```

---

## Security & privacy

- The broker runs locally and exposes a local port (loopback only).
- No network access beyond the local IPC channel.

---
