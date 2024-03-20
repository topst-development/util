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

noinst_LIBRARIES = libhdmi.a

if HDMI_V1_3
libhdmi_a_SOURCES = libhdmi_v1_3.c
endif

if HDMI_V1_4
libhdmi_a_SOURCES = libhdmi_v1_4.c
endif


#---------------------------------------------------
#TOP_DIR = ..
#include $(TOP_DIR)/Makefile.common
#
#LIB  = libhdmi.a ../libphy/libphy.a
#
#ifeq ($(HDMIVERSION), HDMI_V1_3)
#OBJS = libhdmi_v1_3.o
#endif
#
#ifeq ($(HDMIVERSION), HDMI_V1_4)
#OBJS = libhdmi_v1_4.o
#endif
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
