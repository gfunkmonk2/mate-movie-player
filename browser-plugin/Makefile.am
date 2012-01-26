plugindir = $(BROWSER_PLUGIN_DIR)
plugin_LTLIBRARIES =

EXTRA_DIST =		\
	plugin.symbols	\
	marshal.list	\
	TODO		\
	README.browser-plugin

BUILT_SOURCES =

CLEANFILES = $(BUILT_SOURCES)

# Marshaller

BUILT_SOURCES += \
	marshal.h \
	marshal.c \
	$(NULL)

marshal.h: marshal.list
	$(GLIB_GENMARSHAL) --prefix=idolpluginviewer_marshal --internal --header $< > $@
marshal.c: marshal.list
	$(GLIB_GENMARSHAL) --prefix=idolpluginviewer_marshal --internal --body --header $< > $@

noinst_LTLIBRARIES = libidolbrowserplugin_helper.la

libidolbrowserplugin_helper_la_SOURCES = \
	marshal.h \
	marshal.c \
	$(NULL)

libidolbrowserplugin_helper_la_CPPFLAGS =	\
	$(DISABLE_DEPRECATED)			\
	$(AM_CPPFLAGS)

libidolbrowserplugin_helper_la_CFLAGS =	\
	$(DEPENDENCY_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(AM_CFLAGS)

libidolbrowserplugin_helper_la_LIBADD = 	\
	$(DEPENDENCY_CFLAGS)			\
	$(WARN_CFLAGS)				\
	$(AM_CFLAGS)

# The Glow button test program

noinst_PROGRAMS = test-glow-button

test_glow_button_SOURCES = \
	idol-glow-button.c \
	idol-glow-button.h \
	test-glow-button.c

test_glow_button_LDADD = \
	libidolbrowserplugin_helper.la		\
	$(DEPENDENCY_LIBS) -lm

test_glow_button_CFLAGS =	\
	$(DEPENDENCY_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(AM_CFLAGS)

test_glow_button_CPPFLAGS =	\
	$(DISABLE_DEPRECATED)	\
	$(AM_CPPFLAGS)

# Idol Plugin Viewer

libexec_PROGRAMS = idol-plugin-viewer

idol_plugin_viewer_SOURCES = \
	idol-glow-button.c		\
	idol-glow-button.h		\
	idol-plugin-viewer.c		\
	idol-plugin-viewer-interface.h	\
	idol-plugin-viewer-constants.h	\
	idol-plugin-viewer-options.h	\
	$(NULL)

idol_plugin_viewer_CPPFLAGS = \
	-D_REENTRANT					\
	-DDBUS_API_SUBJECT_TO_CHANGE			\
	-I$(top_srcdir)					\
	-I$(top_srcdir)/lib				\
	-I$(top_srcdir)/src				\
	-I$(top_srcdir)/src/backend			\
	-I$(top_builddir)/src				\
	-I$(top_builddir)/src/backend			\
	-DMATELOCALEDIR="\"$(datadir)/locale\""	\
	-DMATECONF_PREFIX="\"/apps/idol\""		\
	-DDATADIR="\"$(datadir)\""			\
	-DLIBEXECDIR="\"$(libexecdir)\""		\
	-DSHAREDIR="\"$(pkgdatadir)\""			\
	-DLOGO_NAME="\"idol_logo.png\""		\
	-DG_LOG_DOMAIN=\""IdolEmbedded\""		\
	$(DISABLE_DEPRECATED)				\
	$(AM_CPPFLAGS)

idol_plugin_viewer_CFLAGS = \
	$(DEPENDENCY_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(DBUS_CFLAGS)		\
	$(SN_CFLAGS)		\
	$(AM_CFLAGS)

idol_plugin_viewer_LDADD =						\
	$(top_builddir)/src/backend/libbaconvideowidget.la		\
	$(top_builddir)/src/libidol_player.la				\
	$(top_builddir)/lib/libidolscrsaver.la				\
	libidolbrowserplugin_helper.la					\
	$(DEPENDENCY_LIBS)	\
	$(SN_LIBS)		\
	$(XVIDMODE_LIBS) 	\
	$(DBUS_LIBS)		\
	$(XTEST_LIBS)		\
	$(X_LIBS)

BUILT_SOURCES += \
	idol-plugin-viewer-interface.h

idol-plugin-viewer-interface.h: org_mate_idol_PluginViewer.xml
	$(DBUS_BIND) --prefix=idol_embedded --mode=glib-server $< > $@

EXTRA_DIST += \
	org_mate_idol_PluginViewer.xml

# Idol GMP Plugin

if ENABLE_GMP_PLUGIN

plugin_LTLIBRARIES += libidol-gmp-plugin.la

libidol_gmp_plugin_la_SOURCES =	\
	npapi.h				\
	npruntime.h			\
	npupp.h				\
	idolNPNGlue.cpp		\
	idolNPClass.cpp		\
	idolNPClass.h			\
	idolNPObject.cpp		\
	idolNPObject.h			\
	idolNPObjectWrapper.h		\
	idolGMPControls.cpp		\
	idolGMPControls.h		\
	idolGMPError.cpp		\
	idolGMPError.h			\
	idolGMPNetwork.cpp		\
	idolGMPNetwork.h		\
	idolGMPPlayer.cpp		\
	idolGMPPlayer.h		\
	idolGMPPlaylist.cpp		\
	idolGMPPlaylist.h		\
	idolGMPSettings.cpp		\
	idolGMPSettings.h		\
	idolPluginGlue.cpp		\
	idolPlugin.cpp			\
	idolPlugin.h			\
	idol-plugin-viewer-constants.h

libidol_gmp_plugin_la_CPPFLAGS = \
	-I$(top_srcdir)/src				\
	-I$(top_srcdir)/src/backend			\
	-I$(top_builddir)/src				\
	-I$(top_builddir)/src/backend			\
	-DXP_UNIX					\
	-DMOZ_X11					\
	-DIDOL_GMP_PLUGIN				\
	-DIDOL_PL_PARSER_MINI				\
	-D_REENTRANT					\
	-DDBUS_API_SUBJECT_TO_CHANGE			\
	-DSYSCONFDIR="\"$(sysconfdir)\""		\
	-DLIBEXECDIR="\"$(libexecdir)\""		\
	$(DISABLE_DEPRECATED)				\
	$(AM_CPPFLAGS)

libidol_gmp_plugin_la_CXXFLAGS = \
	$(BROWSER_PLUGIN_CFLAGS)	\
	$(DBUS_CFLAGS)			\
	$(WARN_CXXFLAGS)		\
	$(PLUGIN_EXTRA_WARN_CXXFLAGS)	\
	$(AM_CXXFLAGS)

libidol_gmp_plugin_la_LIBADD = \
	libidolbrowserplugin_helper.la	\
	$(DBUS_LIBS)			\
	$(BROWSER_PLUGIN_LIBS)

libidol_gmp_plugin_la_LDFLAGS = \
	-avoid-version	\
	-export-symbols $(srcdir)/plugin.symbols \
	-module		\
	$(AM_LDFLAGS)

endif

# Idol NarrowSpace Plugin

if ENABLE_NARROWSPACE_PLUGIN

plugin_LTLIBRARIES += libidol-narrowspace-plugin.la

libidol_narrowspace_plugin_la_SOURCES = \
	npapi.h				\
	npruntime.h			\
	npupp.h				\
	idolNPNGlue.cpp		\
	idolNPClass.cpp		\
	idolNPClass.h			\
	idolNPObject.cpp		\
	idolNPObject.h			\
	idolNPObjectWrapper.h		\
	idolNPVariantWrapper.h		\
	idolPluginGlue.cpp		\
	idolPlugin.cpp			\
	idolPlugin.h			\
	idolNarrowSpacePlugin.cpp	\
	idolNarrowSpacePlugin.h	\
	idol-plugin-viewer-constants.h

libidol_narrowspace_plugin_la_CPPFLAGS = \
	-I$(top_srcdir)/src				\
	-I$(top_srcdir)/src/backend			\
	-I$(top_builddir)/src				\
	-I$(top_builddir)/src/backend			\
	-DXP_UNIX					\
	-DMOZ_X11					\
	-DIDOL_NARROWSPACE_PLUGIN			\
	-DIDOL_PL_PARSER_MINI				\
	-D_REENTRANT					\
	-DDBUS_API_SUBJECT_TO_CHANGE			\
	-DSYSCONFDIR="\"$(sysconfdir)\""		\
	-DLIBEXECDIR="\"$(libexecdir)\""		\
	$(DISABLE_DEPRECATED)				\
	$(AM_CPPFLAGS)

libidol_narrowspace_plugin_la_CXXFLAGS = \
	$(BROWSER_PLUGIN_CFLAGS)	\
	$(DBUS_CFLAGS)			\
	$(WARN_CXXFLAGS)		\
	$(PLUGIN_EXTRA_WARN_CXXFLAGS)	\
	$(AM_CXXFLAGS)

libidol_narrowspace_plugin_la_LIBADD = \
	libidolbrowserplugin_helper.la	\
	$(DBUS_LIBS)			\
	$(BROWSER_PLUGIN_LIBS)

libidol_narrowspace_plugin_la_LDFLAGS = \
	-avoid-version	\
	-export-symbols $(srcdir)/plugin.symbols \
	-module		\
	$(AM_LDFLAGS)

endif

# Idol MULLY Plugin

if ENABLE_MULLY_PLUGIN

plugin_LTLIBRARIES += libidol-mully-plugin.la

libidol_mully_plugin_la_SOURCES =	\
	npapi.h				\
	npruntime.h			\
	npupp.h				\
	idolNPNGlue.cpp		\
	idolNPClass.cpp		\
	idolNPClass.h			\
	idolNPObject.cpp		\
	idolNPObject.h			\
	idolNPObjectWrapper.h		\
	idolNPVariantWrapper.h		\
	idolPluginGlue.cpp		\
	idolPlugin.cpp			\
	idolPlugin.h			\
	idolMullYPlugin.cpp		\
	idolMullYPlugin.h		\
	idol-plugin-viewer-constants.h

libidol_mully_plugin_la_CPPFLAGS = \
	-I$(top_srcdir)/src				\
	-I$(top_srcdir)/src/backend			\
	-I$(top_builddir)/src				\
	-I$(top_builddir)/src/backend			\
	-DXP_UNIX					\
	-DMOZ_X11					\
	-DIDOL_MULLY_PLUGIN				\
	-DIDOL_PL_PARSER_MINI				\
	-D_REENTRANT					\
	-DDBUS_API_SUBJECT_TO_CHANGE			\
	-DSYSCONFDIR="\"$(sysconfdir)\""		\
	-DLIBEXECDIR="\"$(libexecdir)\""		\
	$(DISABLE_DEPRECATED)				\
	$(AM_CPPFLAGS)

libidol_mully_plugin_la_CXXFLAGS = \
	$(BROWSER_PLUGIN_CFLAGS)	\
	$(DBUS_CFLAGS)			\
	$(WARN_CXXFLAGS)		\
	$(PLUGIN_EXTRA_WARN_CXXFLAGS)	\
	$(AM_CXXFLAGS)

libidol_mully_plugin_la_LIBADD = \
	libidolbrowserplugin_helper.la	\
	$(DBUS_LIBS)			\
	$(BROWSER_PLUGIN_LIBS)

libidol_mully_plugin_la_LDFLAGS = \
	-avoid-version	\
	-export-symbols $(srcdir)/plugin.symbols \
	-module		\
	$(AM_LDFLAGS)

endif

# Idol Cone Plugin

if ENABLE_CONE_PLUGIN

plugin_LTLIBRARIES += libidol-cone-plugin.la

libidol_cone_plugin_la_SOURCES =	\
	npapi.h				\
	npruntime.h			\
	npupp.h				\
	idolNPNGlue.cpp		\
	idolNPClass.cpp		\
	idolNPClass.h			\
	idolNPObject.cpp		\
	idolNPObject.h			\
	idolNPObjectWrapper.h		\
	idolNPVariantWrapper.h		\
	idolConeAudio.cpp		\
	idolConeAudio.h		\
	idolCone.cpp			\
	idolCone.h			\
	idolConeInput.cpp		\
	idolConeInput.h		\
	idolConePlaylist.cpp		\
	idolConePlaylist.h		\
	idolConePlaylistItems.cpp	\
	idolConePlaylistItems.h	\
	idolConeVideo.cpp		\
	idolConeVideo.h		\
	idolPluginGlue.cpp		\
	idolPlugin.cpp			\
	idolPlugin.h			\
	idol-plugin-viewer-constants.h

libidol_cone_plugin_la_CPPFLAGS = \
	-I$(top_srcdir)/src				\
	-I$(top_srcdir)/src/backend			\
	-I$(top_builddir)/src				\
	-I$(top_builddir)/src/backend			\
	-DXP_UNIX					\
	-DMOZ_X11					\
	-DIDOL_CONE_PLUGIN				\
	-DIDOL_PL_PARSER_MINI				\
	-D_REENTRANT					\
	-DDBUS_API_SUBJECT_TO_CHANGE			\
	-DSYSCONFDIR="\"$(sysconfdir)\""		\
	-DLIBEXECDIR="\"$(libexecdir)\""		\
	$(DISABLE_DEPRECATED)				\
	$(AM_CPPFLAGS)

libidol_cone_plugin_la_CXXFLAGS = \
	$(BROWSER_PLUGIN_CFLAGS)	\
	$(DBUS_CFLAGS)			\
	$(WARN_CXXFLAGS)		\
	$(PLUGIN_EXTRA_WARN_CXXFLAGS)	\
	$(AM_CXXFLAGS)

libidol_cone_plugin_la_LIBADD = \
	libidolbrowserplugin_helper.la	\
	$(DBUS_LIBS)			\
	$(BROWSER_PLUGIN_LIBS)

libidol_cone_plugin_la_LDFLAGS = \
	-avoid-version	\
	-export-symbols $(srcdir)/plugin.symbols \
	-module		\
	$(AM_LDFLAGS)

endif
