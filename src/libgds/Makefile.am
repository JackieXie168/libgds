INCLUDES = -I..
AM_CFLAGS = -Wall -Werror -O2 -L/usr/lib

lib_LTLIBRARIES = libgds.la
libgds_la_LDFLAGS = -no-undefined
libgds_la_LIBADD = -lm -lpcre -lz -lpthread
libgdsinclude_HEADERS = array.h cli.h cli_ctx.h config.h fifo.h list.h log.h memory.h radix-tree.h rand.h regex.h sequence.h stack.h str_util.h tokenizer.h tokens.h types.h xml.h
libgdsincludedir = $(includedir)/libgds

libgds_la_SOURCES =\
	array.c \
	cli.c \
	cli_ctx.c \
	fifo.c \
	list.c \
	log.c \
	memory.c \
	radix-tree.c \
	rand.c \
	regex.c \
	sequence.c \
	stack.c \
	str_util.c \
	tokenizer.c \
	tokens.c \
	xml.c \
	array.h \
	cli.h \
	cli_ctx.h \
	config.h \
	fifo.h \
	list.h \
	log.h \
	memory.h \
	radix-tree.h \
	rand.h \
	regex.h \
	sequence.h \
	stack.h \
	str_util.h \
	tokenizer.h \
	tokens.h \
	xml.h \
	types.h
