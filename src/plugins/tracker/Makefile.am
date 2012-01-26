modules_flags = -export_dynamic -avoid-version -module

plugindir = $(PLUGINDIR)/tracker
plugin_LTLIBRARIES = libtracker.la

plugin_in_files = tracker.idol-plugin.in

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

libtracker_la_SOURCES = idol-tracker.c idol-tracker-widget.h idol-tracker-widget.c
libtracker_la_LDFLAGS = $(modules_flags)
libtracker_la_LIBADD = $(TRACKER_LIBS)
libtracker_la_CPPFLAGS = $(common_defines)

libtracker_la_CFLAGS = \
	$(DEPENDENCY_CFLAGS)	\
	$(TRACKER_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(DBUS_CFLAGS)		\
	$(AM_CFLAGS)		\
	-I$(top_srcdir)/	\
	-I$(top_srcdir)/src	\
	-I$(srcdir)/		\
	-I$(top_srcdir)/src/plugins

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(noinst_DATA) $(BUILT_SOURCES)
DISTCLEANFILES = $(plugin_DATA)
