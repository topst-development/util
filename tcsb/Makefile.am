tcsbdir = ${bindir}
tcsb_SCRIPTS = tcsb_mkimg tcsb_signtool tcsb_tool_v2

install-data-hook:
	cd $(DESTDIR)$(bindir); \
	mv tcsb_mkimg tcc-tcsb-mkimg; \
	mv tcsb_signtool tcc-tcsb-signtool; \
	mv tcsb_tool_v2 tcc-tcsb-tool-v2;

