##
##  This file is for system specific configurations. For instance, on
##  some systems the path to zlib needs to be added. Example:
##
##  CFLAGS += -I/usr/local/include
##  LFLAGS += -L/usr/local/lib

ifeq ($(TOOLCHAIN),emmake)
CFLAGS += -s USE_ZLIB=1
LFLAGS += -s USE_ZLIB=1
endif
