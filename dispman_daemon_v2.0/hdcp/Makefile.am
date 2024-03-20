
SUBDIRS = libhdcp

AM_CFLAGS = -I. \
		-I$(top_srcdir)/dispman_daemon_v2.0 \
		-I$(top_srcdir)/dispman_daemon_v2.0/hdcp/libhdcp \
		-I$(top_srcdir)/dispman_daemon_v2.0/libs \
		-I@KERNEL_DIR@ \
		-I@KERNEL_DIR@/arch/arm/include \
		-I@KERNEL_DIR@/include/uapi \
		-I@KERNEL_DIR@/include \
		-I@KERNEL_DIR@/drivers/char

AM_CPPFLAGS = $(AM_CFLAGS) -fno-exceptions -fPIC
AM_CPPLIBS = -lpthread -Wl,-rpath,.

DEFS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT -D__EXPORTED_HEADERS__

lib_LTLIBRARIES = libHdcpHdmi.la

libHdcpHdmi_la_SOURCES = \
			hdcpApp.c
libHdcpHdmi_la_LDFLAGS = -version-info 1:0:0
libHdcpHdmi_la_LIBADD =	libhdcp/libhdcp.la

