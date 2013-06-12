CLEANFILES = *.d *.so *-rdict.cxx *-rdict.h
MAINTAINERCLEANFILES = Makefile.in

# EXTRA_DIST = some extra files, e.g. test-data

lib_LTLIBRARIES = libfroast.la

libfroast_la_SOURCES = \
	util.cxx \
	logging.cxx \
	block_allocator.cxx vjson.cxx \
	BranchManager.cxx \
	File.cxx \
	JSON.cxx \
	Selector.cxx \
	Settings.cxx \
	TH1Tools.cxx \
	TreeMapperSel.cxx

libfroast_la_headers = \
	util.h \
	logging.h \
	block_allocator.h vjson.h \
	BranchManager.h \
	File.h \
	JSON.h \
	Selector.h \
	Settings.h \
	TH1Tools.h \
	TreeMapperSel.h

pkginclude_HEADERS = $(libfroast_la_headers)

bin_PROGRAMS = froast

noinst_PROGRAMS =

froast_SOURCES = froast.cxx
froast_LDADD = libfroast.la
# froast_LDFLAGS = -static


libfroast_la_SOURCES += libfroast-rdict.cxx

libfroast-rdict.cxx: $(libfroast_la_headers) froast-LinkDef.h
	$(ROOTCINT) -f $@ -c -p $(CPPFLAGS) $(CXXFLAGS) $+

rootmapdir = $(libdir)
rootmap_DATA = .libs/libfroast.rootmap

.libs/libfroast.rootmap: libfroast.la | $(libfroast_la_headers) froast-LinkDef.h
	$(RLIBMAP) -o $@ -l .libs/libfroast@SHLIBEXT@ -c $|
