modules_flags = -export_dynamic -avoid-version -module

plugindir = $(PLUGINDIR)/youtube
plugin_LTLIBRARIES = libyoutube.la

uidir = $(plugindir)
ui_DATA = youtube.ui

plugin_in_files = youtube.idol-plugin.in

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

libyoutube_la_SOURCES = idol-youtube.c
libyoutube_la_LDFLAGS = $(modules_flags)
libyoutube_la_LIBADD = \
	$(LIBGDATA_LIBS)	\
	$(LIBSOUP_LIBS)
libyoutube_la_CPPFLAGS = $(common_defines)

libyoutube_la_CFLAGS = \
	$(DEPENDENCY_CFLAGS)	\
	$(LIBGDATA_CFLAGS)	\
	$(LIBSOUP_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(DBUS_CFLAGS)		\
	$(AM_CFLAGS)		\
	-I$(top_srcdir)/	\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/plugins

EXTRA_DIST = $(plugin_in_files) $(ui_DATA)

CLEANFILES = $(plugin_DATA) $(BUILT_SOURCES)
DISTCLEANFILES = $(plugin_DATA)

