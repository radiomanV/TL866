# Makefile for setupapi.dll
#
# By default, invoking make will build the setupapi.dll using LibUsb hotplug events.
# Using 'make hotplug=udev' will use the udev library for hotplug events.
# This Makefile will compile the setupapi.dll linking against the system installed
# libusb shared library. If this is not desired see the accompanying script which
# will invoke this Makefile using different arguments for static libusb linking.

# Configuration
CC          = winegcc
DLL         = setupapi.dll
CFLAGS      = -m32 -mincoming-stack-boundary=2
LDFLAGS     = -shared $(DLL:.dll=.spec)
LIBS        = -lusb-1.0

# Conditional hotplug settings
ifeq ($(hotplug),udev)
    CFLAGS    += -DUDEV
    LIBS      += -ludev
endif

# Source files and object files
SRC         = setupapi.c

# Targets
.PHONY: all clean

all: $(DLL)

# Compilation & Linking
$(DLL): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	mv $(DLL).so $(DLL)

# Cleanup
clean:
	$(RM) $(DLL)
