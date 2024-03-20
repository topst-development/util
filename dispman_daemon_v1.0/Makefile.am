CC = @CC@
CPP = @CPP@

AM_CFLAGS = -I. \
			-I$(top_srcdir)/dispman_daemon_v1.0 \
			-Ilibs \
			-I$(top_srcdir)/dispman_daemon_v1.0/libs \
			-Iutils \
			-I$(top_srcdir)/dispman_daemon_v1.0/uitls \
			-I@KERNEL_DIR@ \
			-I@KERNEL_DIR@/arch/arm/mach-@MACH_TYPE@/include \
			-I@KERNEL_DIR@/arch/arm/mach-@MACH_TYPE@/include/mach \
			-I@KERNEL_DIR@/arch/arm/plat-tcc/include
AM_CPPFLAGS = $(AM_CFLAGS) -fno-exceptions
AM_CPPLIBS = -lpthread -Wl,-rpath,.

DEFS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT -D__EXPORTED_HEADERS__

SUBDIRS = libs

bin_PROGRAMS = dispman_daemon
dispman_daemon_SOURCES = \
			hdmi.cpp \
			component.cpp \
			composite.cpp \
			extenddisplay.cpp \
			utils/properties.c
dispman_daemon_LDADD = \
			libs/libiic/libiic.a \
			libs/libhdmi/libhdmi.a \
			libs/libcec/libcec.a \
			libs/libhpd/libhpd.a \
			libs/libedid/libedid.a \
			libs/libphy/libphy.a \
			libs/libddc/libddc.a \
			libs/libsrm/libsrm.a \
			libs/libsmp/libsmp.a

### Do not force strip since yocto do do_strip
#dispman_daemon_LDFLAGS = -s
