if WITH_MEMORY_DEBUG
  FLAG_MEMORY_DEBUG=-DHAVE_MEMORY_DEBUG
else
  FLAG_MEMORY_DEBUG=
endif


INCLUDES = -I..
AM_CFLAGS = -Wall -Werror -O2 -L/usr/lib $(FLAG_MEMORY_DEBUG)

lib_LTLIBRARIES = libgds.la
libgds_la_LDFLAGS = -no-undefined -release $(LT_RELEASE) -version-info $(LT_VERSION)
libgds_la_LIBADD = -lm -lz #-lpthread
libgdsinclude_HEADERS = \
	array.h \
	assoc_array.h \
	bit_vector.h \
	bloom_hash.h \
	bloom_filter.h \
	cli.h \
	cli_commands.h \
	cli_ctx.h \
	cli_params.h \
	dllist.h \
	enumerator.h \
	fifo.h \
	gds.h \
	hash.h \
	hash_utils.h \
	libgds-config.h \
	list.h \
	log.h \
	patricia-tree.h \
	memory.h \
	radix-tree.h \
	rand.h \
	sequence.h \
	sha1.h \
	stack.h \
	str_util.h \
	tokenizer.h \
	tokens.h \
	types.h \
	xml.h 

libgdsincludedir = $(includedir)/libgds

libgds_la_SOURCES =\
	array.c \
	assoc_array.c \
	bit_vector.c \
	bloom_hash.c \
	bloom_filter.c \
	cli.c \
	cli_ctx.c \
	cli_params.c \
	dllist.c \
	dllist.h \
	enumerator.c \
	enumerator.h \
	fifo.c \
	gds.c \
	gds.h \
	hash.c \
	hash_utils.c \
	list.c \
	log.c \
	memory.c \
	memory_debug.c \
	patricia-tree.c \
	patricia-tree.h \
	radix-tree.c \
	radix-tree.h \
	rand.c \
	sequence.c \
	sha1.c \
	stack.c \
	str_util.c \
	tokenizer.c \
	tokens.c \
	xml.c \
	array.h \
	assoc_array.h \
	bit_vector.h \
	bloom_hash.h \
	bloom_filter.h \
	cli.h \
	cli_commands.h \
	cli_ctx.h \
	cli_params.h \
	fifo.h \
	hash.h \
	hash_utils.h \
	list.h \
	log.h \
	memory.h \
	memory_debug.h \
	rand.h \
	sequence.h \
	sha1.h \
	stack.h \
	str_util.h \
	tokenizer.h \
	tokens.h \
	xml.h \
	types.h
