modules_flags = -export_dynamic -avoid-version -module

plugindir = $(PLUGINDIR)/skipto
uidir = $(plugindir)
plugin_LTLIBRARIES = libskipto.la

plugin_in_files = skipto.idol-plugin.in

%.idol-plugin: %.idol-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.idol-plugin.in=.idol-plugin)
ui_DATA = skipto.ui

common_defines = \
	-D_REENTRANT					\
	-DDBUS_API_SUBJECT_TO_CHANGE			\
	-DMATELOCALEDIR=\""$(datadir)/locale"\"	\
	-DMATECONF_PREFIX=\""/apps/idol"\"		\
	-DDATADIR=\""$(datadir)"\"			\
	-DLIBEXECDIR=\""$(libexecdir)"\"		\
	-DBINDIR=\""$(bindir)"\"			\
	-DIDOL_PLUGIN_DIR=\""$(libdir)/idol/plugins"\"\
	-DG_LOG_DOMAIN=\""Idol"\"			\
	$(DISABLE_DEPRECATED)

libskipto_la_SOURCES = \
	idol-skipto-plugin.c	\
	idol-skipto-plugin.h	\
	idol-time-entry.c	\
	idol-time-entry.h	\
	idol-skipto.c		\
	idol-skipto.h
libskipto_la_LDFLAGS = $(modules_flags)
libskipto_la_CPPFLAGS = \
	$(common_defines)		\
	-I$(top_srcdir)/src/backend

libskipto_la_CFLAGS = 			\
	$(DEPENDENCY_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(DBUS_CFLAGS)			\
	$(AM_CFLAGS)			\
	-I$(top_srcdir)/		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/src/plugins	\
	-I$(srcdir)/

EXTRA_DIST = $(plugin_in_files) $(ui_DATA)

CLEANFILES = $(plugin_DATA) $(BUILT_SOURCES)
DISTCLEANFILES = $(plugin_DATA)
