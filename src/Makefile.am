SUBDIRS = plugins backend

bin_PROGRAMS = idol idol-video-thumbnailer idol-video-indexer idol-audio-preview
libexec_PROGRAMS =
noinst_LTLIBRARIES =				\
	libidol_player.la			\
	libidol_main.la

common_defines = \
	-D_REENTRANT					\
	-DDBUS_API_SUBJECT_TO_CHANGE			\
	-DMATELOCALEDIR=\""$(datadir)/locale"\"	\
	-DMATECONF_PREFIX=\""/apps/idol"\"		\
	-DDATADIR=\""$(datadir)"\"			\
	-DLIBEXECDIR=\""$(libexecdir)"\"		\
	-DBINDIR=\""$(bindir)"\"			\
	-DGTKBUILDERDIR=\"""\"				\
	$(DISABLE_DEPRECATED)

modules_flags = -export_dynamic -avoid-version -module -no-undefined

# Idol UI ltlibrary (used by browser plugins)

libidol_player_la_SOURCES = \
	idol-statusbar.c	\
	idol-statusbar.h	\
	idol-interface.c	\
	idol-interface.h	\
	idol-fullscreen.c	\
	idol-fullscreen.h	\
	gsd-media-keys-window.c	\
	gsd-media-keys-window.h	\
	idol-time-label.c	\
	idol-time-label.h

libidol_player_la_CPPFLAGS = \
	-I$(srcdir)/backend		\
	-I$(top_srcdir)/		\
	$(common_defines)		\
	-DG_LOG_DOMAIN=\""Idol"\"	\
	$(AM_CPPFLAGS)

libidol_player_la_CFLAGS = \
	$(DEPENDENCY_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(UNIQUE_CFLAGS)	\
	$(AM_CFLAGS)

libidol_player_la_LDFLAGS = \
	$(AM_LDFLAGS) 

# Idol main library (used for main player; separate to allow gtk-doc to be used)

libidol_main_la_SOURCES = \
	$(IDOLMARSHALFILES)		\
	idol-object.c			\
	idol.h				\
	idol-private.h			\
	idol-cell-renderer-video.c	\
	idol-cell-renderer-video.h	\
	idol-video-list.c		\
	idol-video-list.h		\
	idol-preferences.c		\
	idol-preferences.h		\
	idol-dnd-menu.c		\
	idol-dnd-menu.h 		\
	idol-options.c			\
	idol-options.h			\
	idol-playlist.c		\
	idol-playlist.h		\
	eggfileformatchooser.c		\
	eggfileformatchooser.h		\
	egg-macros.h			\
	idol-session.c			\
	idol-session.h			\
	idol-sidebar.c			\
	idol-sidebar.h 		\
	idol-open-location.c		\
	idol-open-location.h 		\
	idol-menu.c			\
	idol-menu.h			\
	idol-uri.c			\
	idol-uri.h			\
	ev-sidebar.c			\
	ev-sidebar.h			\
	idol-subtitle-encoding.c	\
	idol-subtitle-encoding.h	\
	idol-profile.h

libidol_main_la_CPPFLAGS = \
	-I$(top_srcdir)/		\
	-I$(srcdir)/backend		\
	-I$(srcdir)/plugins		\
	-I$(top_builddir)/data		\
	$(common_defines)		\
	-DG_LOG_DOMAIN=\""Idol"\"	\
	$(AM_CPPFLAGS)

libidol_main_la_CFLAGS = \
	$(DEPENDENCY_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(DBUS_CFLAGS)			\
	$(MISSING_PLUGINS_CFLAGS)	\
	$(UNIQUE_CFLAGS)		\
	$(AM_CFLAGS)

libidol_main_la_LDFLAGS = \
	$(AM_LDFLAGS)

libidol_main_la_LIBADD = \
	libidol_player.la		\
	backend/libbaconvideowidget.la	\
	plugins/libidolmodule.la	\
	$(UNIQUE_LIBS)			\
	$(DBUS_LIBS)			\
	$(XVIDMODE_LIBS)		\
	$(XTEST_LIBS)			\
	$(X_LIBS)

if WITH_SMCLIENT
libidol_main_la_SOURCES += \
	eggsmclient.c \
	eggsmclient.h \
	eggsmclient-private.h \
	$(NULL)
libidol_main_la_CFLAGS += $(SMCLIENT_CFLAGS)
libidol_main_la_LIBADD += $(SMCLIENT_LIBS)

if WITH_SMCLIENT_XSMP
libidol_main_la_SOURCES += \
	eggdesktopfile.c \
	eggdesktopfile.h \
	eggsmclient-xsmp.c
libidol_main_la_CPPFLAGS += -DEGG_SM_CLIENT_BACKEND_XSMP
endif
if WITH_SMCLIENT_WIN32
libidol_main_la_SOURCES += eggsmclient-win32.c
endif
if WITH_SMCLIENT_QUARTZ
libidol_main_la_SOURCES += eggsmclient-osx.c
endif
endif

if ENABLE_PYTHON
libidol_main_la_LIBADD += \
	$(PYTHON_LIBS)						\
	$(top_builddir)/bindings/python/idol-python.la
endif

# Idol

IDOLMARSHALFILES = 						\
	idolobject-marshal.c idolobject-marshal.h		\
	idolplaylist-marshal.c idolplaylist-marshal.h		\
	idolvideolist-marshal.c idolvideolist-marshal.h
GLIB_GENMARSHAL=`pkg-config --variable=glib_genmarshal glib-2.0`
BUILT_SOURCES = $(IDOLMARSHALFILES)

idolobject-marshal.h: idolobject-marshal.list Makefile
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --prefix=idolobject_marshal $(srcdir)/idolobject-marshal.list --header > idolobject-marshal.h )
idolobject-marshal.c: idolobject-marshal.h Makefile
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --prefix=idolobject_marshal $(srcdir)/idolobject-marshal.list --header --body > idolobject-marshal.c )
idolplaylist-marshal.h: idolplaylist-marshal.list Makefile
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --prefix=idolplaylist_marshal $(srcdir)/idolplaylist-marshal.list --header > idolplaylist-marshal.h )
idolplaylist-marshal.c: idolplaylist-marshal.h Makefile
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --prefix=idolplaylist_marshal $(srcdir)/idolplaylist-marshal.list --header --body > idolplaylist-marshal.c )
idolvideolist-marshal.h: idolvideolist-marshal.list Makefile
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --prefix=idolvideolist_marshal $(srcdir)/idolvideolist-marshal.list --header > idolvideolist-marshal.h )
idolvideolist-marshal.c: idolvideolist-marshal.h Makefile
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --prefix=idolvideolist_marshal $(srcdir)/idolvideolist-marshal.list --header --body > idolvideolist-marshal.c )

idol_SOURCES = idol.c

idol_CPPFLAGS =			\
	-I$(top_srcdir)/		\
	-I$(srcdir)/backend		\
	$(common_defines)		\
	-DG_LOG_DOMAIN=\""Idol"\"	\
	$(AM_CPPFLAGS)

idol_CFLAGS =				\
	$(UNIQUE_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(DEPENDENCY_CFLAGS)		\
	$(AM_CFLAGS)

idol_LDFLAGS = \
	$(AM_LDFLAGS)

idol_LDADD =						\
	libidol_main.la				\
	$(DEPENDENCY_LIBS)				\
	$(XVIDMODE_LIBS)				\
	$(XTEST_LIBS)					\
	$(X_LIBS)

# Idol video thumbnailer

idol_video_thumbnailer_SOURCES =	\
	idol-video-thumbnailer.c	\
	idol-resources.c		\
	idol-resources.h

idol_video_thumbnailer_CPPFLAGS = \
	-I$(top_srcdir)/				\
	-I$(srcdir)/backend				\
	$(common_defines)				\
	-DG_LOG_DOMAIN=\""IdolVideoThumbnailer"\"	\
	$(AM_CPPFLAGS)

idol_video_thumbnailer_CFLAGS = \
	$(DEPENDENCY_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(AM_CFLAGS)

idol_video_thumbnailer_LDFLAGS = \
	$(AM_LDFLAGS)

idol_video_thumbnailer_LDADD =			\
	backend/libbaconvideowidget.la		\
	$(DEPENDENCY_LIBS)			\
	$(EXTRA_BACKEND_LIBS) 			\
	$(XTEST_LIBS)				\
	$(XVIDMODE_LIBS)			\
	$(X_LIBS)

# Caja Property Page

if HAVE_CAJA

cajadir = $(CAJADIR)
caja_LTLIBRARIES = libidol-properties-page.la

libidol_properties_page_la_SOURCES = 	\
	idol-properties-main.c		\
	idol-properties-view.c		\
	idol-properties-view.h

libidol_properties_page_la_CPPFLAGS =		\
	-I$(srcdir)/backend			\
	-I$(top_srcdir)/			\
	-I$(srcdir)/plugins/properties		\
	-I$(top_builddir)/data			\
	$(common_defines)			\
	-DG_LOG_DOMAIN=\""IdolPropertiesPage"\"\
	$(AM_CPPFLAGS)

libidol_properties_page_la_CFLAGS = \
	$(WARN_CFLAGS)		\
	$(DEPENDENCY_CFLAGS)	\
	$(CAJA_CFLAGS)	\
	$(AM_CFLAGS)

libidol_properties_page_la_LDFLAGS = \
	$(modules_flags)	\
	$(AM_LDFLAGS)

libidol_properties_page_la_LIBADD =				\
	backend/libbaconvideowidget.la				\
	plugins/properties/libbaconvideowidgetproperties.la	\
	libidol_player.la					\
	$(DEPENDENCY_LIBS)					\
	$(CAJA_LIBS)					\
	$(XTEST_LIBS)						\
	$(XVIDMODE_LIBS)					\
	$(X_LIBS)

noinst_PROGRAMS = test-properties-page

test_properties_page_SOURCES =		\
	idol-properties-main.c		\
	idol-properties-view.c		\
	idol-properties-view.h		\
	test-properties-page.c

test_properties_page_CPPFLAGS = 		\
	-I$(srcdir)/backend			\
	-I$(top_srcdir)/			\
	-I$(top_builddir)/data			\
	-I$(srcdir)/plugins/properties		\
	$(common_defines)			\
	$(AM_CPPFLAGS)

test_properties_page_CFLAGS = \
	$(WARN_CFLAGS)		\
	$(DEPENDENCY_CFLAGS)	\
	$(CAJA_CFLAGS)	\
	$(AM_CFLAGS)

test_properties_page_LDFLAGS = \
	$(AM_LDFLAGS)

test_properties_page_LDADD = 					\
	backend/libbaconvideowidget.la				\
	plugins/properties/libbaconvideowidgetproperties.la	\
	libidol_player.la					\
	$(DEPENDENCY_LIBS)					\
	$(CAJA_LIBS)					\
	$(XTEST_LIBS)						\
	$(XVIDMODE_LIBS)					\
	$(X_LIBS)

endif # HAVE_CAJA

# Idol Video Indexer

idol_video_indexer_SOURCES =		\
	idol-video-indexer.c		\
	idol-resources.c		\
	idol-resources.h

idol_video_indexer_CPPFLAGS = \
	-I$(top_srcdir)/			\
	-I$(srcdir)/backend			\
	-I$(top_builddir)/data			\
	$(common_defines)			\
	-DG_LOG_DOMAIN=\""IdolVideoIndexer"\"	\
	$(AM_CPPFLAGS)

idol_video_indexer_CFLAGS = \
	$(WARN_CFLAGS)		\
	$(DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

idol_video_indexer_LDFLAGS = \
	$(AM_LDFLAGS)

idol_video_indexer_LDADD = \
	backend/libbaconvideowidget.la	\
	$(DEPENDENCY_LIBS)		\
	$(EXTRA_BACKEND_LIBS)		\
	$(XTEST_LIBS)			\
	$(XVIDMODE_LIBS)		\
	$(X_LIBS)

# Idol Audio Preview for caja
idol_audio_preview_SOURCES =		\
	idol-audio-preview.c		\
	idol-resources.c		\
	idol-resources.h

idol_audio_preview_CPPFLAGS =	\
	-I$(top_srcdir)/			\
	-I$(srcdir)/backend			\
	-I$(top_builddir)/data			\
	$(common_defines)			\
	-DG_LOG_DOMAIN=\""IdolAudioPreview"\"	\
	$(AM_CPPFLAGS)

idol_audio_preview_CFLAGS = \
	$(WARN_CFLAGS)		\
	$(DEPENDENCY_CFLAGS)	\
	$(AM_CFLAGS)

idol_audio_preview_LDFLAGS = \
	$(AM_LDFLAGS)

idol_audio_preview_LDADD = \
	backend/libbaconvideowidget.la	\
	$(DEPENDENCY_LIBS)		\
	$(EXTRA_BACKEND_LIBS)		\
	$(XTEST_LIBS)			\
	$(XVIDMODE_LIBS)		\
	$(X_LIBS)

CLEANFILES =			\
	*.bak			\
	core*			\
	*.orig			\
	*~			\
	$(desktop_DATA)		\
	$(BUILT_SOURCES)

EXTRA_DIST = 				\
	idolobject-marshal.list	\
	idolvideolist-marshal.list	\
	idolplaylist-marshal.list
		
