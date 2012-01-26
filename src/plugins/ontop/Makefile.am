modules_flags = -export_dynamic -avoid-version -module

plugindir = $(PLUGINDIR)/ontop
plugin_LTLIBRARIES = libontop.la

plugin_in_files = ontop.idol-plugin.in

%.idol-plugin: %.idol-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.idol-plugin.in=.idol-plugin)

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

libontop_la_SOURCES = idol-ontop.c
libontop_la_LDFLAGS = $(modules_flags)
libontop_la_LIBADD = $(top_builddir)/lib/libidolscrsaver.la
libontop_la_CPPFLAGS = $(common_defines)

libontop_la_CFLAGS = \
	$(DEPENDENCY_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(DBUS_CFLAGS)		\
	$(AM_CFLAGS)		\
	-I$(top_srcdir)/	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/plugins

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(plugin_DATA) $(BUILT_SOURCES)
DISTCLEANFILES = $(plugin_DATA)
