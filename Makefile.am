if WITH_DOXYGEN
doxygen: 
	$(DOXYGEN) Doxyfile;
endif

pkgconfigdir := $(libdir)/pkgconfig
pkgconfig_DATA := libgds.pc

EXTRA_DIST := debian Doxyfile

SUBDIRS := src doc
