#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([UTILS], [1.0.0], [limdh3@telechips.com])
AM_INIT_AUTOMAKE([foreign subdir-objects])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_CPP
AC_PROG_RANLIB
AM_PROG_AS
# Add daoying for display daemon v2
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])
#

AC_PREFIX_DEFAULT(/usr)

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
#AC_FUNC_MALLOC
#AC_CHECK_FUNCS([memset select strchr strrchr strstr])
AC_CHECK_FUNCS([clock_gettime gettimeofday malloc realloc memset munmap select strchr strerror strrchr strstr])

### TC-UPDATE-TOOL for SFMC Option. ###
AC_ARG_ENABLE([tc-update-tool-sfmc],
    AC_HELP_STRING([--enable-tc-update-tool-sfmc], [Enable tc-update-tool-sfmc packages]))

AS_IF([test "x$enable_tc_update_tool_sfmc" = "xyes"], HAVE_UPDATE_SFMC=1, HAVE_UPDATE_SFMC=0)
AC_SUBST(HAVE_UPDATE_SFMC)
AM_CONDITIONAL([HAVE_UPDATE_SFMC], [test "x$HAVE_UPDATE_SFMC" = x1])
AS_IF([test "x$HAVE_UPDATE_SFMC" = "x1"], AC_DEFINE([HAVE_UPDATE_SFMC], 1, [HAVE UPDTAE SFMC?]))

### TC-MAKE-IMAGE Option. ###
AC_ARG_ENABLE([tc-make-image],
    AC_HELP_STRING([--enable-tc-make-image], [Enable tc-make-image packages]))

AS_IF([test "x$enable_tc_make_image" = "xyes"], HAVE_MKIMAGE=1, HAVE_MKIMAGE=0)
AC_SUBST(HAVE_MKIMAGE)
AM_CONDITIONAL([HAVE_MKIMAGE], [test "x$HAVE_MKIMAGE" = x1])
AS_IF([test "x$HAVE_MKIMAGE" = "x1"], AC_DEFINE([HAVE_MKIMAGE], 1, [HAVE MKIMAGE?]))

### TCC802x-SFMC-MAKE-IMAGE Option. ###
AC_ARG_ENABLE([sfmc-make-image],
    AC_HELP_STRING([--enable-sfmc-make-image], [Enable tcc802x-smfc-make-image packages]))

AS_IF([test "x$enable_sfmc_make_image" = "xyes"], HAVE_SFMC_MKIMAGE=1, HAVE_SFMC_MKIMAGE=0)
AC_SUBST(HAVE_SFMC_MKIMAGE)
AM_CONDITIONAL([HAVE_SFMC_MKIMAGE], [test "x$HAVE_SFMC_MKIMAGE" = x1])
AS_IF([test "x$HAVE_SFMC_MKIMAGE" = "x1"], AC_DEFINE([HAVE_SFMC_MKIMAGE], 1, [HAVE SFMC MKIMAGE?]))

### TC-SPLASH-TOOL Option. ###
AC_ARG_ENABLE([tc-splash-tool],
    AC_HELP_STRING([--enable-tc-splash-tool], [Enable tc-splash-tool packages]))

AS_IF([test "x$enable_tc_splash_tool" = "xyes"], HAVE_SPLASH=1, HAVE_SPLASH=0)
AC_SUBST(HAVE_SPLASH)
AM_CONDITIONAL([HAVE_SPLASH], [test "x$HAVE_SPLASH" = x1])
AS_IF([test "x$HAVE_SPLASH" = "x1"], AC_DEFINE([HAVE_SPLASH], 1, [HAVE SPLASH?]))

### TC SECURE BOOT Option. ###
AC_ARG_ENABLE([tc-secure-boot],
    AC_HELP_STRING([--enable-tc-secure-boot], [Enable tc-secure-boot packages]))

AS_IF([test "x$enable_tc_secure_boot" = "xyes"], HAVE_SECURE=1, HAVE_SECURE=0)
AC_SUBST(HAVE_SECURE)
AM_CONDITIONAL([HAVE_SECURE], [test "x$HAVE_SECURE" = x1])
AS_IF([test "x$HAVE_SECURE"= "x1"], AC_DEFINE([HAVE_SECURE], 1, [HAVE SECURE?]))

#----------------------------------------------------------------------------------
# TC DISPLAY MANAGER Option
# =========================
# --enable-tc-dispman
# --enable-tc-dispman-no-daemon	: Not used
#
AC_ARG_ENABLE([tc-dispman],
    AC_HELP_STRING([--enable-tc-dispman], [Enable tc-dispman packages]))
AS_IF([test "x$enable_tc_dispman" = "xyes"], HAVE_DISPMAN=1, HAVE_DISPMAN=0)
AC_SUBST(HAVE_DISPMAN)
AM_CONDITIONAL([HAVE_DISPMAN], [test "x$HAVE_DISPMAN" = x1])
AS_IF([test "x$HAVE_DISPMAN" = "x1"], AC_DEFINE([HAVE_DISPMAN], 1, [HAVE DISPMAN?]))

AC_ARG_ENABLE([tc-dispman-no-daemon],
    AC_HELP_STRING([--enable-tc-dispman-no-daemon], [Enable no daemon (tc-dispman)]))
AS_IF([test "x$enable_tc_dispman_no_daemon" = "xyes"], AC_DEFINE(NO_DEAMON))
#AS_IF([test "x$enable_tc_dispman_no_daemon" = "xyes"], HAVE_DISPMAN_NO_DAEMON=1, HAVE_DISPMAN_NO_DAEMON=0)
#AC_SUBST(HAVE_DISPMAN_NO_DAEMON)
#AM_CONDITIONAL([HAVE_DISPMAN_NO_DAEMON], [test "x$HAVE_DISPMAN_NO_DAEMON" = x1])
#AS_IF([test "x$HAVE_DISPMAN_NO_DAEMON"= "x1"], [AC_DEFINE(NO_DEAMON)])

#
# Set ouput display
# =================
# --enable-tc-dispman-hdmi
# --enable-tc-dispman-composite
# --enable-tc-dispman-component
#
AC_ARG_ENABLE([tc-dispman-hdmi],
    AC_HELP_STRING([--enable-tc-dispman-hdmi], [Enable hdmi (tc-diapman)]))
AS_IF([test "x$enable_tc_dispman_hdmi" = "xyes"], [HAVE_DISPMAN_HDMI=1], [HAVE_DISPMAN_HDMI=0])
AS_IF([test "x$HAVE_DISPMAN_HDMI" = "x1"], AC_DEFINE([HAVE_HDMI_OUTPUT], 1, [Daemon using hdmi output?]))
#AC_SUBST(HAVE_DISPMAN_HDMI)
AM_CONDITIONAL([HAVE_DISPMAN_HDMI], [test "x$HAVE_DISPMAN_HDMI" = x1])
#AS_IF([test "x$HAVE_DISPMAN_HDMI"= "x1"], AC_DEFINE([HAVE_HDMI_OUTPUT], 1, [HAVE HDMI?]))

AC_ARG_ENABLE([tc-dispman-composite],
    AC_HELP_STRING([--enable-tc-dispman-composite], [Enable composite (tc-dispman)]))
AS_IF([test "x$enable_tc_dispman_composite" = "xyes"], AC_DEFINE(TCC_OUTPUT_COMPOSITE))
#AS_IF([test "x$enable_tc_dispman_composite" = "xyes"], HAVE_DISPMAN_COMPOSITE=1, HAVE_DISPMAN_COMPOSITE=0)
#AC_SUBST(HAVE_DISPMAN_COMPOSITE)
#AM_CONDITIONAL([HAVE_DISPMAN_COMPOSITE], [test "x$HAVE_DISPMAN_COMPOSITE" = x1])
#AS_IF([test "x$HAVE_DISPMAN_COMPOSITE"= "x1"], AC_DEFINE([TCC_OUTPUT_COMPOSITE], 1, [HAVE COMPOSITE?]))

AC_ARG_ENABLE([tc-dispman-component],
    AC_HELP_STRING([--enable-tc-dispman-component], [Enable component (tc-dispman)]))
AS_IF([test "x$enable_tc_dispman_component" = "xyes"], AC_DEFINE(TCC_OUTPUT_COMPONENT))
#AS_IF([test "x$enable_tc_dispman_component" = "xyes"], HAVE_DISPMAN_COMPONENT=1, HAVE_DISPMAN_COMPONENT=0)
#AC_SUBST(HAVE_DISPMAN_COMPONENT)
#AM_CONDITIONAL([HAVE_DISPMAN_COMPONENT], [test "x$HAVE_DISPMAN_COMPONENT" = x1])
#AS_IF([test "x$HAVE_DISPMAN_COMPONENT"= "x1"], AC_DEFINE([TCC_OUTPUT_COMPONENT], 1, [HAVE COMPONENT?]))

#
# Set HDMI Option
# ================
# --enable-tc-dispman-safelink
#
AC_ARG_ENABLE([tc-dispman-safelink],
    AC_HELP_STRING([--enable-tc-dispman-safelink], [Dispman supports safelink]))
AS_IF([test "x$HAVE_DISPMAN"= "x1"],
	[AS_IF([test "x$enable-tc-dispman-safelink" = "xyes"], [SUPPORT_SAFE_LINK=1], [SUPPORT_SAFE_LINK=0])])
AS_IF([test "x$SUPPORT_SAFE_LINK" = "xyes"], AC_DEFINE(NO_HPD_CEC_EDID, 1, [Not support hdmi gpio?]))
AS_IF([test "x$SUPPORT_SAFE_LINK" = "xyes"], AC_DEFINE(SUPPORT_SAFE_LINK, 1, [Support Safelink?]))

#
# Set ouput display with HDCP
# =================
# --enable-tc-dispman-hdmi-hdcp
#
AC_ARG_ENABLE([tc-dispman-hdmi-hdcp],
	AC_HELP_STRING([--enable-tc-dispman-hdmi-hdcp], [Enable hdcp (tc-diapman)]))
AS_IF([test "x$enable_tc_dispman_hdmi_hdcp" = "xyes"], [HAVE_HDCP=1], [HAVE_HDCP=0])
AS_IF([test "x$HAVE_HDCP" = "x1"], AC_DEFINE([TCC_HDCP], 1, [Daemon using hdmi hdcp output?]))
AM_CONDITIONAL([HAVE_HDCP], [test "x$HAVE_HDCP" = x1])

#
# Set kernel path
# ===============
# KERNEL_DIR
#
AC_ARG_VAR(KERNEL_DIR, "Set kernel header path")
##AC_SUBST([KERNEL_DIR], [$(LINUX_PLATFORM_KERNELDIR)])
#AC_SUBST([KERNEL_DIR], [/home/B070366/YOCTO/rel/build/tcc8971lcn/tmp/work-shared/tcc8971-lcn-2.0/kernel-source])

#
# Set machine & board type
# ========================
# MACH_TYPE: if tcc893x, tcc896x, tcc897x then HDMI_V1.4
# BOARD_TYPE: if lcnv2 then -DNO_HPD_CEC_EDID
#
AC_ARG_VAR(MACH_TYPE, "Set machine type")
##AC_SUBST([MACH_TYPE], [$(LINUX_PLATFORM_MACHINE)])
#AC_SUBST([MACH_TYPE], [tcc897x])
AC_ARG_VAR(BOARD_TYPE, "Set board type")
##AC_SUBST([BOARD_TYPE], [$(LINUX_PLATFORM_BOARD)])
#AC_SUBST([BOARD_TYPE], [lcnv2])

AS_IF([test "x$BOARD_TYPE" = "xlcnv2"], AC_DEFINE(NO_HPD_CEC_EDID))

AS_IF([test "x$HAVE_DISPMAN_HDMI" = "x1"], AS_CASE("x$MACH_TYPE",
						  ["xtcc893x"], [HDMI_V1_4=1],
						  ["xtcc896x"], [HDMI_V1_4=1],
						  ["xtcc897x"], [HDMI_V1_4=1],
						  ["xtcc898x"], [HDMI_V2_0=1],
						  ["xtcc899x"], [HDMI_V2_0=1],
						  ["xtcc803x"], [HDMI_V2_0=1],
						  [HDMI_V1_4=1]))

AM_CONDITIONAL([HDMI_V1_3], [test "x$HDMI_V1_3" = x1])
AM_CONDITIONAL([HDMI_V1_4], [test "x$HDMI_V1_4" = x1])
AM_CONDITIONAL([HDMI_V2_0], [test "x$HDMI_V2_0" = x1])

AS_IF([test "x$HDMI_V1_3" = "x1"], AC_DEFINE([HDMI_V1_3], 1, [Makefile define HDMI_V1_4=1]))
AS_IF([test "x$HDMI_V1_4" = "x1"], AC_DEFINE([HDMI_V1_4], 1, [Makefile define HDMI_V1_4=1]))
AS_IF([test "x$HDMI_V2_0" = "x1"], AC_DEFINE([HDMI_V2_0], 1, [Makefile define HDMI_V2_0=1]))

# Build Directory
AS_IF([test "x$HDMI_V2_0" = x1], [DISPMAN_DIR=dispman_daemon_v2.0], [DISPMAN_DIR=dispman_daemon_v1.0])
AC_SUBST(DISPMAN_DIR)

#
# Set default define
# ==================
# -DTELECHIPS
# -DNOT_ANDROID
#
AC_DEFINE(TELECHIPS)
AC_DEFINE(NOT_ANDROID)
#----------------------------------------------------------------------------------


AC_CONFIG_FILES([Makefile
                 dispman_daemon_v1.0/Makefile
                 dispman_daemon_v1.0/libs/Makefile
                 dispman_daemon_v1.0/libs/libcec/Makefile
                 dispman_daemon_v1.0/libs/libddc/Makefile
                 dispman_daemon_v1.0/libs/libedid/Makefile
                 dispman_daemon_v1.0/libs/libhdmi/Makefile
                 dispman_daemon_v1.0/libs/libhpd/Makefile
                 dispman_daemon_v1.0/libs/libiic/Makefile
                 dispman_daemon_v1.0/libs/libphy/Makefile
                 dispman_daemon_v1.0/libs/libsmp/Makefile
                 dispman_daemon_v1.0/libs/libsrm/Makefile
                 dispman_daemon_v2.0/Makefile
                 dispman_daemon_v2.0/hdcp/Makefile
                 dispman_daemon_v2.0/hdcp/libhdcp/Makefile
                 dispman_daemon_v2.0/libs/Makefile
                 dispman_daemon_v2.0/libs/libddc/Makefile
                 dispman_daemon_v2.0/libs/libdrv/Makefile
                 dispman_daemon_v2.0/libs/libedid/Makefile
                 mkimage/Makefile
                 tcc802x-sfmc-mkimage/Makefile
                 splash_tool/Makefile
                 tcc_crc/Makefile
                 tcsb/Makefile
                 update-tool-sfmc/Makefile])
AC_OUTPUT
