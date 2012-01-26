SUBDIRS = $(PLUGINS)
DIST_SUBDIRS = $(ALL_PLUGINS) idol

noinst_LTLIBRARIES = libidolmodule.la

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

modules_flags = -export_dynamic -avoid-version -module

libidolmodule_la_SOURCES =		\
	idol-module.c			\
	idol-module.h			\
	idol-plugin.c			\
	idol-plugin.h			\
	idol-plugins-engine.c		\
	idol-plugins-engine.h		\
	idol-plugin-manager.c		\
	idol-plugin-manager.h

libidolmodule_la_CPPFLAGS = \
	$(common_defines)	\
	$(AM_CPPFLAGS)

libidolmodule_la_CFLAGS = \
	$(DEPENDENCY_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(DBUS_CFLAGS)		\
	$(AM_CFLAGS)		\
	-I$(top_srcdir) 				\
	-I$(top_srcdir)/lib 				\
	-I$(top_builddir)/lib 				\
	-I$(top_srcdir)/src				\
	-I$(top_srcdir)/src/backend			\
	-I$(top_srcdir)/src/plugins

libidolmodule_la_LDFLAGS = \
	$(AM_LDFLAGS)			

if ENABLE_PYTHON

SUBDIRS += idol

libidolmodule_la_LIBADD = \
	$(DEPENDENCY_LIBS)	\
	$(PYTHON_LIBS)		\
	$(PYTHON_EXTRA_LIBS)	\
	$(PYGTK_LIBS)

libidolmodule_la_SOURCES += \
	idol-python-module.c		\
	idol-python-module.h		\
	idol-python-plugin.c		\
	idol-python-plugin.h

libidolmodule_la_CFLAGS += \
	$(NO_STRICT_ALIASING_CFLAGS)	\
	$(PYGTK_CFLAGS)			\
	$(PYTHON_CFLAGS)
endif
