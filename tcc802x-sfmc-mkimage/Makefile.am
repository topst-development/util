CC = @CC@
AM_CFLAGS = -I$(top_srcdir)/tcc802x-sfmc-mkimage -I$(top_srcdir)/tcc802x-sfmc-mkimage/include
AM_CPPLIBS = -lpthread
DEF	= -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT -D__EXPORTED_HEADERS__

bin_PROGRAMS = tcc802x-sfmc-mkimage
tcc802x_sfmc_mkimage_SOURCES = main.c \
							   mkimage.c



