INCLUDES = -I..
AM_CFLAGS = -Wall -Werror -O2

check_PROGRAMS = \
	gds_test_array \
	gds_test_ptr_array \
	gds_test_list \
	gds_test_radix_tree \
	gds_test_tokenizer \
	gds_test_fifo \
	gds_test_memory
TESTS = $(check_PROGRAMS)

gds_test_array_SOURCES = main.c
gds_test_array_CFLAGS = -D_GDS_CHECK_ARRAY_
gds_test_array_LDADD = ../libgds/libgds.la

gds_test_ptr_array_SOURCES = main.c
gds_test_ptr_array_CFLAGS = -D_GDS_CHECK_PTR_ARRAY_
gds_test_ptr_array_LDADD = ../libgds/libgds.la

gds_test_list_SOURCES = main.c
gds_test_list_CFLAGS = -D_GDS_CHECK_LIST_
gds_test_list_LDADD = ../libgds/libgds.la

gds_test_radix_tree_SOURCES = main.c
gds_test_radix_tree_CFLAGS = -D_GDS_CHECK_RADIX_TREE_
gds_test_radix_tree_LDADD = ../libgds/libgds.la

gds_test_tokenizer_SOURCES = main.c
gds_test_tokenizer_CFLAGS = -D_GDS_CHECK_TOKENIZER_
gds_test_tokenizer_LDADD = ../libgds/libgds.la

gds_test_fifo_SOURCES = main.c
gds_test_fifo_CFLAGS = -D_GDS_CHECK_FIFO_
gds_test_fifo_LDADD = ../libgds/libgds.la

gds_test_memory_SOURCES = main.c
gds_test_memory_CFLAGS = -D_GDS_CHECK_MEMORY_
gds_test_memory_LDADD = ../libgds/libgds.la
