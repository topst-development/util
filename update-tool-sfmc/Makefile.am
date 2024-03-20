CC = @CC@
AM_CFLAGS = -I$(top_srcdir)/update-tool-sfmc
AM_CPPLIBS = -lpthread
DEFS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT -D__EXPORTED_HEADERS__

bin_PROGRAMS = tc-update-tool-sfmc
tc_update_tool_sfmc_SOURCES = update.c
