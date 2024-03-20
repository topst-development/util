CC = @CC@
CPP = @CPP@

AM_CFLAGS = -I. \
			-I$(top_srcdir)/dispman_daemon_v2.0 \
			-I$(top_srcdir)/dispman_daemon_v2.0/libs \
			-I$(top_srcdir)/dispman_daemon_v2.0/uitls \
			-I@KERNEL_DIR@ \
			-I@KERNEL_DIR@/arch/arm/include \
			-I@KERNEL_DIR@/include/uapi \
			-I@KERNEL_DIR@/include \
			-I@KERNEL_DIR@/drivers/char

AM_CPPFLAGS = $(AM_CFLAGS) -fno-exceptions -fPIC

DEFS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT -D__EXPORTED_HEADERS__

noinst_LTLIBRARIES = libddc.la
libddc_la_SOURCES = libddc.c
libddc_la_LDFLAGS = -module
