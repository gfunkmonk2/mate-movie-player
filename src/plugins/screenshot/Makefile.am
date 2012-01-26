modules_flags = -export_dynamic -avoid-version -module

plugindir = $(PLUGINDIR)/screenshot
uidir = $(plugindir)
plugin_LTLIBRARIES = libscreenshot.la

plugin_in_files = screenshot.idol-plugin.in

%.idol-plugin: %.idol-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.idol-plugin.in=.idol-plugin)
ui_DATA = gallery.ui mate-screenshot.ui

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

libscreenshot_la_SOURCES = \
	mate-screenshot-widget.c	\
	mate-screenshot-widget.h	\
	idol-screenshot-plugin.c	\
	idol-screenshot-plugin.h	\
	idol-screenshot.c		\
	idol-screenshot.h		\
	idol-gallery.c			\
	idol-gallery.h			\
	idol-gallery-progress.c	\
	idol-gallery-progress.h
libscreenshot_la_LDFLAGS = $(modules_flags)
libscreenshot_la_CPPFLAGS = \
	$(common_defines)		\
	-I$(top_srcdir)/src/backend

libscreenshot_la_CFLAGS = 		\
	$(DEPENDENCY_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(AM_CFLAGS)			\
	-I$(top_srcdir)/		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/src/plugins	\
	-I$(srcdir)/

EXTRA_DIST = $(plugin_in_files) $(ui_DATA)

CLEANFILES = $(plugin_DATA) $(BUILT_SOURCES)
DISTCLEANFILES = $(plugin_DATA)
