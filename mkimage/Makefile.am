AM_CFLAGS = -I$(top_srcdir)/mkimage

mkimagedir = ${bindir}
mkimage_SCRIPTS = lz4demo mkbootimg mkmtdimg qpress

install-data-hook:
	cd $(DESTDIR)$(bindir); \
    mv lz4demo tc-lz4-demo; \
	mv mkbootimg tc-make-bootimg; \
	mv mkmtdimg tc-make-mtdimg; \
	mv qpress tc-qpress;


