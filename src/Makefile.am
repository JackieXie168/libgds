SUBDIRS = libgds check

dist-hook:
	(for f in `find $(distdir) | grep -E '\.(c|h|java)$$'`; do \
	echo $$f; \
	sed -E -e '/\$$Id.*\$$/ r $(top_builddir)/LICENCE' $$f > $$f.tmp; \
	mv $$f.tmp $$f; \
	done)
