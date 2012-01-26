modules_flags = -export_dynamic -avoid-version -module

plugindir = $(PLUGINDIR)/media-player-keys
plugin_LTLIBRARIES = libmedia_player_keys.la

plugin_in_files = media-player-keys.idol-plugin.in

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

libmedia_player_keys_la_SOURCES = idol-media-player-keys.c $(BUILT_SOURCES)
libmedia_player_keys_la_LDFLAGS = $(modules_flags)
libmedia_player_keys_la_LIBADD = $(MEDIA_PLAYER_KEYS_LIBS)
libmedia_player_keys_la_CPPFLAGS = $(common_defines)

libmedia_player_keys_la_CFLAGS = \
	$(DEPENDENCY_CFLAGS)	\
	$(MEDIA_PLAYER_KEYS_CFLAGS) \
	$(WARN_CFLAGS)		\
	$(DBUS_CFLAGS)		\
	$(AM_CFLAGS)		\
	-I$(top_srcdir)/	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/plugins

BUILT_SOURCES =			\
	idol-marshal.h		\
	idol-marshal.c

#Rule to generate the marshal files
idol-marshal.c: idol-marshal.list
	$(AM_V_GEN) @GLIB_GENMARSHAL@ --prefix=idol_marshal $< --header --body > $@

idol-marshal.h: idol-marshal.list
	$(AM_V_GEN) @GLIB_GENMARSHAL@ --prefix=idol_marshal $< --header > $@

EXTRA_DIST = idol-marshal.list $(plugin_in_files)

CLEANFILES = $(plugin_DATA) $(BUILT_SOURCES)
DISTCLEANFILES = $(plugin_DATA)
