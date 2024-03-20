CC = @CC@
CPP = @CPP@
CCAS = @CCAS@

AM_CFLAGS = -I../ \
			-I$(top_srcdir)/dispman_daemon_v1.0/libs \
			-I../../ \
			-I$(top_srcdir)/dispman_daemon_v1.0 \
			-I@KERNEL_DIR@ \
			-I@KERNEL_DIR@/arch/arm/mach-@MACH_TYPE@/include \
			-I@KERNEL_DIR@/arch/arm/mach-@MACH_TYPE@/include/mach \
			-I@KERNEL_DIR@/arch/arm/plat-tcc/include
AM_CPPFLAGS = $(AM_CFLAGS) -fno-exceptions -fPIC
AM_CCASFLAGS = -mfpu=neon

DEFS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT -D__EXPORTED_HEADERS__

noinst_LIBRARIES = libsmp.a
libsmp_a_SOURCES = hdmi_sha1.c \
				   smp_add.c \
				   smp_div.c \
				   smp_err.c \
				   smp_exp.c \
				   smp_mod.c \
				   smp_mont.c \
				   smp_mul.c \
				   smp_shift.c \
				   smp_sqr.c \
				   smp_util.c \
				   arm/smp_asm.S


#---------------------------------------------------
#TOP_DIR = ..
#include $(TOP_DIR)/Makefile.common
#
#LIB  = libsmp.a
#OBJS = hdmi_sha1.o \
#       smp_add.o   \
#       smp_div.o   \
#       smp_err.o   \
#       smp_exp.o   \
#       smp_mod.o   \
#       smp_mont.o  \
#       smp_mul.o   \
#       smp_shift.o \
#       smp_sqr.o   \
#       smp_util.o  \
#       arm/smp_asm.o
#
#all: $(LIB)
#
#$(LIB): $(OBJS)
#    $(AR) cru $@ $(OBJS)
#    $(RANLIB) $@
#    cp *.o $(OUT_OBJS)/.
#    cp arm/*.o $(OUT_OBJS)/.
#
#arm/smp_asm.o:
##   arm-linux-as -mno-fpu arm/smp_asm.s -o arm/smp_asm.o
##   $(AS) -mno-fpu arm/smp_asm.s -o arm/smp_asm.o
#    $(AS) -mfpu=neon arm/smp_asm.S -o arm/smp_asm.o
#
#clean:
#    rm -f $(LIB) *.o arm/*.o
