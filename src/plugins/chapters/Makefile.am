modules_flags = -export_dynamic -avoid-version -module

plugindir = $(PLUGINDIR)/chapters
plugin_LTLIBRARIES = libchapters.la

plugin_in_files = chapters.idol-plugin.in

%.idol-plugin: %.idol-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.idol-plugin.in=.idol-plugin)

uidir = $(plugindir)
ui_DATA = chapters-list.ui chapters-edit.ui

common_defines = \
	-D_REENTRANT					\
	-DDBUS_API_SUBJECT_TO_CHANGE			\
	-DMATELOCALEDIR=\""$(datadir)/locale"\"	\
	-DMATECONF_PREFIX=\""/apps/idol"\"		\
	-DDATADIR=\""$(datadir)"\"			\
	-DLIBEXECDIR=\""$(libexecdir)"\"		\
	-DBINDIR=\""$(bindir)"\"			\
	-DIDOL_PLUGIN_DIR=\""$(libdir)/idol/plugins"\"\
	$(DISABLE_DEPRECATED)

libchapters_la_SOURCES = idol-chapters.c idol-cmml-parser.c idol-cmml-parser.h idol-chapters-utils.c idol-chapters-utils.h idol-edit-chapter.c idol-edit-chapter.h
libchapters_la_LDFLAGS = $(modules_flags)
libchapters_la_LIBADD = $(CHAPTERS_LIBS)
libchapters_la_CPPFLAGS = $(common_defines)

libchapters_la_CFLAGS = 		\
	$(DEPENDENCY_CFLAGS)		\
	$(PEAS_CFLAGS)			\
	$(CHAPTERS_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(DBUS_CFLAGS)			\
	$(AM_CFLAGS)			\
	-I$(top_srcdir)/		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/src/backend	\
	-I$(top_srcdir)/src/plugins

EXTRA_DIST = $(plugin_in_files) $(ui_DATA)

CLEANFILES = $(plugin_DATA) $(BUILT_SOURCES)
DISTCLEANFILES = $(plugin_DATA)
