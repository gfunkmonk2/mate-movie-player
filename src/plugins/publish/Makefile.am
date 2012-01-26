modules_flags = -export_dynamic -avoid-version -module

plugindir = $(PLUGINDIR)/publish
plugin_LTLIBRARIES = libpublish.la

plugin_in_files = publish.idol-plugin.in

%.idol-plugin: %.idol-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.idol-plugin.in=.idol-plugin)

uidir = $(PLUGINDIR)/publish
ui_DATA = publish-plugin.ui

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

libpublish_la_SOURCES = idol-publish.c
libpublish_la_LDFLAGS = $(modules_flags)
libpublish_la_LIBADD = $(LIBEPC_LIBS)
libpublish_la_CPPFLAGS = $(common_defines)

libpublish_la_CFLAGS = \
	$(DEPENDENCY_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(DBUS_CFLAGS)		\
	$(LIBEPC_CFLAGS)	\
	$(UNIQUE_CFLAGS)	\
	$(AM_CFLAGS)		\
	-I$(top_srcdir)/	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/backend \
	-I$(top_srcdir)/src/plugins

EXTRA_DIST = $(plugin_in_files) $(ui_DATA)

CLEANFILES = $(plugin_DATA) $(BUILT_SOURCES)
DISTCLEANFILES = $(plugin_DATA)
