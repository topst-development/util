CC = @CC@
CPP = @CPP@

AM_CFLAGS = -I../ \
			-I$(top_srcdir)/dispman_daemon_v1.0/libs \
			-I../../ \
			-I$(top_srcdir)/dispman_daemon_v1.0 \
			-I@KERNEL_DIR@ \
			-I@KERNEL_DIR@/arch/arm/mach-@MACH_TYPE@/include \
			-I@KERNEL_DIR@/arch/arm/mach-@MACH_TYPE@/include/mach \
			-I@KERNEL_DIR@/arch/arm/plat-tcc/include
AM_CPPFLAGS = $(AM_CFLAGS) -fno-exceptions -fPIC

DEFS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT -D__EXPORTED_HEADERS__

noinst_LIBRARIES = libsrm.a
libsrm_a_SOURCES = libsrm.c


#---------------------------------------------------
#TOP_DIR = ..
#include $(TOP_DIR)/Makefile.common
#
##CFLAGS += -I../../
#
#LIB = libsrm.a
#OBJS = libsrm.o
#
#all: $(LIB)
#
#$(LIB): $(OBJS)
#    $(AR) cru $@ $(OBJS)
#    $(RANLIB) $@
#    cp lib*.o $(OUT_OBJS)/.
#
#clean:
#    rm -f $(LIB) *.o
