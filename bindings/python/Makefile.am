## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = idol-python.la

nodist_idol_python_la_SOURCES = idol.c

idol_python_la_SOURCES = override_common.c override_common.h

idol_python_la_LDFLAGS = \
	-module -avoid-version

idol_python_la_LIBADD = \
	$(DEPENDENCY_LIBS)	\
	$(PYTHON_LIBS)		\
	$(PYTHON_EXTRA_LIBS)	\
	$(PYGTK_LIBS)

idol_python_la_CFLAGS = \
	-I$(top_srcdir)			\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/src/plugins	\
	$(DEPENDENCY_CFLAGS)		\
	$(PYGTK_CFLAGS)			\
	$(PYTHON_CFLAGS)		\
	$(AM_CFLAGS)			\
	$(WNOERROR_CFLAGS)

idol.c: idol.defs idol.override
	( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--override $*.override \
		--prefix py$* $(<F) ) > $@ 

BINDING_HEADERS_SRCDIR_IN = \
	src/idol.h				\
	src/idol-cell-renderer-video.h		\
	src/plugins/idol-plugin.h		\
	$(NULL)

BINDING_HEADERS_BUILDDIR_IN = 

BINDING_HEADERS_SRCDIR		:= $(addprefix $(top_srcdir)/,$(BINDING_HEADERS_SRCDIR_IN))
BINDING_HEADERS_BUILDDIR	:= $(addprefix $(top_builddir)/,$(BINDING_HEADERS_BUILDDIR_IN))

regenerate-python-binding:
	$(PYGTK_H2DEF) $(sort $(BINDING_HEADERS_SRCDIR) $(BINDING_HEADERS_BUILDDIR)) > idol.defs.new

BUILT_SOURCES = idol.c

EXTRA_DIST = \
	idol.override		\
	idol.defs

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
