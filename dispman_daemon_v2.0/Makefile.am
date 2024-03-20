CC = @CC@
CPP = @CPP@

AM_CFLAGS = -I. \
			-I$(top_srcdir)/dispman_daemon_v2.0 \
			-Ilibs \
			-I$(top_srcdir)/dispman_daemon_v2.0/libs \
			-Iutils \
			-I$(top_srcdir)/dispman_daemon_v2.0/uitls \
			-Ihdcp \
			-I$(top_srcdir)/dispman_daemon_v2.0/hdcp \
			-I$(top_srcdir)/dispman_daemon_v2.0/composite \
			-I$(top_srcdir)/dispman_daemon_v2.0/component \
			-I@KERNEL_DIR@ \
			-I@KERNEL_DIR@/arch/arm/include \
			-I@KERNEL_DIR@/include/uapi \
			-I@KERNEL_DIR@/include \
			-I@KERNEL_DIR@/drivers/char

AM_CPPFLAGS = $(AM_CFLAGS) -fno-exceptions
AM_CPPLIBS = -lpthread -Wl,-rpath,.

DEFS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_REENTRANT -D__EXPORTED_HEADERS__

SUBDIRS = \
	libs \
	hdcp

bin_PROGRAMS = dispman_daemon
dispman_daemon_SOURCES = \
			composite/composite.cpp \
			component/component.cpp \
			extenddisplay.cpp \
			utils/properties.c \
			utils/bit_operation.c \
			utils/Timers.c

if HAVE_DISPMAN_HDMI
dispman_daemon_SOURCES += hdmi/hdmi_properties.c \
			hdmi/short_video_desc.c \
			hdmi/video_cap_data_block.c \
			hdmi/speaker_alloc_data_block.c \
			hdmi/short_audio_desc.c \
			hdmi/monitor_range_limits.c \
			hdmi/hdmivsdb.c \
			hdmi/hdmiforumvsdb.c \
			hdmi/edid_parser.c \
			hdmi/colorimetry_data_block.c \
			hdmi/hdrstaticmeta_data_block.c \
			hdmi/hdmi_edid.c \
			hdmi/hdmi_hdr.c \
			hdmi/hdmi_lut.c \
			hdmi/hdmi_drv.c \
			hdmi/hdmi_fb.c \
			hdmi/hdmi_v2.cpp \
			cec/hdmi_cec.cpp
			
dispman_daemon_LDADD = libs/libtcchdmi.la \
			hdcp/libHdcpHdmi.la

endif



### Do not force strip since yocto do do_strip
#dispman_daemon_LDFLAGS = -s
