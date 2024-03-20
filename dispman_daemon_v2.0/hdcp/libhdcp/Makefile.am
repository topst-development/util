AM_CFLAGS = -I.

AM_CPPFLAGS = $(AM_CFLAGS) -fno-exceptions -fPIC

DEFS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT -D__EXPORTED_HEADERS__

lib_LTLIBRARIES = libhdcp.la

libhdcp_la_SOURCES = \
			libhdcp.c
libhdcp_la_LDFLAGS = -version-info 1:0:0
