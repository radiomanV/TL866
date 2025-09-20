CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -Wno-unused-parameter
LDFLAGS ?= -lpthread
WINCC   ?= i686-w64-mingw32-gcc
WINRC   ?= i686-w64-mingw32-windres

PKG_CONFIG := $(shell which pkg-config 2>/dev/null)

# LibUSB detection
ifeq ($(strip $(PKG_CONFIG)),)
  LIBUSB_CFLAGS :=
  LIBUSB_LIBS   := -lusb-1.0
else
  LIBUSB_CFLAGS := $(shell $(PKG_CONFIG) --cflags libusb-1.0 2>/dev/null)
  LIBUSB_LIBS   := $(shell $(PKG_CONFIG) --libs   libusb-1.0 2>/dev/null)
  ifeq ($(strip $(LIBUSB_LIBS)),)
    LIBUSB_LIBS := -lusb-1.0
  endif
endif

# usb-broker
BIN := usb-broker
SRC := broker.c
HDR := protocol.h

RES_RC  := resource.rc
RES_RES := resource.res


# shim.dll
DLL             := shim.dll
DLL_SRCS        := shim.c
DLL_OBJS        := $(DLL_SRCS:.c=.o)
DLL_CFLAGS      := -O2 -s -DWIN32 -D_WIN32_WINNT=0x0601 -Wall -Wextra -Wno-unused-parameter
DLL_LDFLAGS     := -shared -Wl,--kill-at -Wl,--enable-stdcall-fixup
DLL_LIBS        := -lws2_32 -lshlwapi -lgdi32 -luxtheme

# launcher.exe
LAUNCHER        := launcher.exe
LAUNCHER_SRCS   := launcher.c
LAUNCHER_OBJS   := $(LAUNCHER_SRCS:.c=.o)
LAUNCHER_CFLAGS := -O2 -s -DWIN32 -D_WIN32_WINNT=0x0601 -Wall -Wextra
LAUNCHER_LDFLAGS:= -municode -mconsole
LAUNCHER_LIBS   := -lshlwapi


.PHONY: all broker shim launcher windows clean

all: broker shim launcher

broker: $(BIN)

$(BIN): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(LIBUSB_CFLAGS) -o $@ $(SRC) $(LDFLAGS) $(LIBUSB_LIBS)

windows: shim launcher

shim: $(DLL)
launcher: $(LAUNCHER)

$(DLL): $(DLL_OBJS) $(RES_RES)
	$(WINCC) $(DLL_LDFLAGS) -o $@ $(DLL_OBJS) $(RES_RES) $(DLL_LIBS)

$(LAUNCHER): $(LAUNCHER_OBJS)
	$(WINCC) $(LAUNCHER_LDFLAGS) -o $@ $(LAUNCHER_OBJS) $(LAUNCHER_LIBS)

%.o: %.c
	$(WINCC) $(WCCFLAGS) -c $< -o $@

shim.o:     WCCFLAGS := $(DLL_CFLAGS)
launcher.o: WCCFLAGS := $(LAUNCHER_CFLAGS)

$(RES_RES): $(RES_RC) resource.h
	$(WINRC) -i $(RES_RC) -O coff -o $(RES_RES)

clean:
	rm -f $(BIN) *.o $(DLL) $(LAUNCHER) $(RES_RES)
