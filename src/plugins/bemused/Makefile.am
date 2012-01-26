modules_flags = -export_dynamic -avoid-version -module

plugindir = $(PLUGINDIR)/bemused
plugin_LTLIBRARIES = libbemused.la

plugin_in_files = bemused.idol-plugin.in

%.idol-plugin: %.idol-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

noinst_DATA = $(plugin_in_files:.idol-plugin.in=.idol-plugin)

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

libbemused_la_SOURCES = idol-bemused.c
libbemused_la_LDFLAGS = $(modules_flags)
libbemused_la_CPPFLAGS = $(common_defines)
libbemused_la_LIBADD = $(BEMUSED_LIBS)

libbemused_la_CFLAGS =			\
	$(DEPENDENCY_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(BEMUSED_CFLAGS)		\
	$(AM_CFLAGS)			\
	-I$(top_srcdir)/		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/src/plugins	\
	-I$(top_srcdir)/src/backend

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(noinst_DATA) $(BUILT_SOURCES)
DISTCLEANFILES = $(plugin_DATA)
