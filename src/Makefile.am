CLEANFILES = *.d *.so *_rdict.* *_rdict_* *.rootmap
MAINTAINERCLEANFILES = Makefile.in

# EXTRA_DIST = some extra files, e.g. test-data

lib_LTLIBRARIES = libfroast.la

libfroast_la_SOURCES = \
	util.cxx \
	logging.cxx \
	block_allocator.cxx vjson.cxx \
	BranchManager.cxx \
	File.cxx \
	FroastTools.cxx \
	JSON.cxx \
	Settings.cxx \
	TH1Tools.cxx \
	TreeEntryList.cxx \
	TreeMapperSel.cxx

libfroast_la_headers = \
	util.h \
	logging.h \
	block_allocator.h vjson.h \
	BranchManager.h \
	File.h \
	FroastTools.h \
	JSON.h \
	Settings.h \
	TH1Tools.h \
	TreeEntryList.h \
	TreeMapperSel.h

pkginclude_HEADERS = $(libfroast_la_headers)

bin_PROGRAMS = froast

noinst_PROGRAMS =

froast_SOURCES = froast.cxx
froast_LDADD = libfroast.la
# froast_LDFLAGS = -static


rootmapdir = $(libdir)
rootmap_DATA =


libfroast_la_SOURCES += libfroast_rdict.cxx
rootmap_DATA += .libs/libfroast.rootmap

if WITH_CLING

rootmap_DATA += .libs/libfroast_rdict.pcm

libfroast_rdict.cxx: $(libfroast_la_headers) froast_LinkDef.h
	$(ROOTCLING) -f $@.tmp -s libfroast@SHLIBEXT@ -rml libfroast@SHLIBEXT@ -rmf libfroast.rootmap -c $(CPPFLAGS) $(CXXFLAGS) -I$(includedir) $+
	@# Some magic to prefix header names with "$(PACKAGE)/", and only that, in dictionary and rootmap:
	$(GREP) -F -v '"'"`pwd`"'/",' $@.tmp | $(SED) 's|"\([^"]*/\)\?\([^/"]*[.]h\)",|"'$(PACKAGE)/'\2",| ; s|\\"\([^"]*/\)\?\([^/"]*[.]h\)\\"\\n"|\\"'$(PACKAGE)/'\2\\"\\n"|' > $@ && $(RM) $@.tmp
	$(SED) 's|\(header \+\)\([^ ].*/\)\?\([^ ].*[.]h\)|\1'$(PACKAGE)/'\3|' -i libfroast.rootmap

.libs/libfroast.rootmap: libfroast.la
	cp libfroast.rootmap libfroast_rdict.pcm .libs/

.libs/libfroast_rdict.pcm: .libs/libfroast.rootmap

else # not WITH_CLING

libfroast_rdict.cxx: $(libfroast_la_headers) froast_LinkDef.h
	$(ROOTCINT) -f $@ -c -p $(CPPFLAGS) `echo $(CXXFLAGS) | sed 's/\(^\|\s\)\(-std=[A-Za-z0-9+]\+\)\($$\|\s\)/\1\3/g'` $+

.libs/libfroast.rootmap: libfroast.la | $(libfroast_la_headers) froast_LinkDef.h
	$(RLIBMAP) -o $@ -l .libs/libfroast@SHLIBEXT@ -c $|

endif # WITH_CLING
