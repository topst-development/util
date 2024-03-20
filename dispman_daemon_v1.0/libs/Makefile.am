SUBDIRS = \
		  libiic \
		  libhdmi \
		  libcec \
		  libhpd \
		  libedid \
		  libphy \
		  libddc \
		  libsrm \
		  libsmp

#----------------Makefile.common-----------------
#TARGET = $(TARGET_MACH)
#include $(TOP_DIR)/../Makefile.common
#
#ifeq ($(LINUX_PLATFORM_KERNELDIR),)
#$(error No project specified.  Use "source init-linux-platform-env [BUILDDIR]" or "source $LINUX_PLATFORM_BUILDDIR config-build-env")
#endif
#
#CFLAGS += -I../../
#CFLAGS += -fPIC
#
#OUT_LIBS_S  = $(TOP_DIR)/_a
#OUT_OBJS    = $(TOP_DIR)/_o

#----------------Makefile-----------------
#TOP_DIR = .
#include $(TOP_DIR)/Makefile.common
#
#DIRS = libiic libcec libddc libedid libphy libhdmi libhpd libsmp libsrm
#
#ifeq ($(TARGET_MACH), tcc896x)
#HDMILIBNAME := libtcchdmi.so.tcc896x.0.0
#endif
#
#ifeq ($(TARGET_MACH), tcc897x)
#HDMILIBNAME := libtcchdmi.so.tcc897x.0.0
#endif
#
#ifeq ($(TARGET_MACH), tcc893x)
#HDMILIBNAME := libtcchdmi.so.tcc893x.0.0
#endif
#
#ifeq ($(TARGET_MACH), tcc892x)
#HDMILIBNAME := libtcchdmi.so.tcc892x.0.0
#endif
#
#all:
#    mkdir -p $(OUT_LIBS_S)
#    mkdir -p $(OUT_OBJS)
#    @for dir in $(DIRS); do \
#        make -w -C $$dir all || exit 1; \
#    done
#    $(CC) -shared -Wl,-export-dynamic,-soname,libtcchdmi.so.0 -o $(HDMILIBNAME)  $(OUT_OBJS)/*.o
#    \ln -sf $(HDMILIBNAME) libtcchdmi.so.0
#    \ln -sf $(HDMILIBNAME) libtcchdmi.so
#    \cp -a libtcchdmi.so* ../
#
#clean:
#    @for dir in $(DIRS); do \
#        make -w -C $$dir clean || exit 1; \
#    done
#    \rm -rf $(OUT_LIBS_S)
#    \rm -rf $(OUT_OBJS)
#
#cleanall:
#    make clean
#    \rm -rf $(OUT_LIBS_S)
#    \rm -rf $(OUT_OBJS)
#    \rm -f ./libtcchdmi.so*
