SRCDIR                    = .
DLL                       = setupapi.dll

### Common settings
CFLAGS                    =
CEXTRA                    = -m32 -mincoming-stack-boundary=2
LIBRARIES                 = -lusb-1.0

ifeq ($(hotplug),udev)
CFLAGS                    += -DUDEV
LIBRARIES                 += -ludev
endif



### setupapi.dll sources and settings
setupapi_dll_MODULE        = $(DLL)
setupapi_dll_C_SRCS        = setupapi.c
setupapi_dll_LDFLAGS       = -shared -m32 $(setupapi_dll_MODULE:.dll=.spec)
setupapi_dll_OBJS          = $(setupapi_dll_C_SRCS:.c=.o)


### Tools
CC = winegcc


### Generic targets
all: $(DLL)


### Build rules
.PHONY: all clean dummy


# Implicit rules
.c.o:
	$(CC) -c $(CFLAGS) $(CEXTRA) -o $@ $<


# Rules for cleaning
clean: 
	$(RM) $(setupapi_dll_C_SRCS:.c=.o) $(DLL).so


### Target specific build rules
$(setupapi_dll_MODULE): $(setupapi_dll_OBJS)
	$(CC) $(setupapi_dll_LDFLAGS) -o $@ $(setupapi_dll_OBJS) $(LIBRARIES)


