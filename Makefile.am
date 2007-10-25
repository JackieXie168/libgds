if WITH_DOXYGEN
doxygen: 
	$(DOXYGEN) Doxyfile;
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgds.pc

EXTRA_DIST = \
	libgds.spec \
	Doxyfile 			\
	make-libgds-debian.sh 	\
	make-package* 		\
	debian/README.Debian 		\
	debian/compat 		\
	debian/docs 			\
	debian/rules 			\
	debian/shlibs.local 		\
	debian/watch 			\
	debian/template/*.copyright 	\
	debian/template/*.dirs 	\
	debian/template/*.install 	\
	debian/template/*.debhelper 	\
	debian/template/control 	\
	debian/template/changelog 


SUBDIRS = src doc
