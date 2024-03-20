FEATURE =
if HAVE_MKIMAGE
  FEATURE += mkimage tcc_crc
endif

if HAVE_SFMC_MKIMAGE
  FEATURE += tcc802x-sfmc-mkimage
endif

if HAVE_SPLASH
  FEATURE += splash_tool
endif

if HAVE_UPDATE_SFMC
  FEATURE += update-tool-sfmc
endif

if HAVE_SECURE
  FEATURE += tcsb
endif

if HAVE_DISPMAN
  FEATURE += @DISPMAN_DIR@
endif

SUBDIRS = $(FEATURE)
