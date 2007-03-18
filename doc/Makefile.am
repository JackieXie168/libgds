docdir := $(prefix)/share/doc/libgds

#nobase_dist_doc_DATA := html/*


#if I define nobase_dist_doc_DATA := html/ ... make install will is not functioning
#if I define nobase_dist_doc_DATA := html/* and the html repository is
#empty, make will output an error because there is no target in html/ !
#faking a target is the only solution I found until now as we don't want to
#distribute doxygen in every distribution nor in the svn!

distclean:
	rm html/* -rf; \
	echo "No doxygen documentation generated" > html/index.html
