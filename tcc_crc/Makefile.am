CC = @CC@
AM_CFLAGS = -I$(top_srcdir)/tcc_crc
DEFS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT

bin_PROGRAMS = tc_crc
tc_crc_SOURCES = main.c write_crc.c

tccrc_head_dir = ${datadir}/tc-crc
tccrc_head__DATA = head.bin
