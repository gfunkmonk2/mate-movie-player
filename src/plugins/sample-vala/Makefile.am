BUILT_SOURCES = idol-sample-vala-plugin.vala.stamp

plugindir = $(PLUGINDIR)/sample-vala
plugin_DATA = $(plugin_in_files:.idol-plugin.in=.idol-plugin)

plugin_in_files = sample-vala.idol-plugin.in
%.idol-plugin: %.idol-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

if ENABLE_VALA
modules_flags = -export_dynamic -avoid-version -module

plugin_LTLIBRARIES = libsample-vala.la
# override to _not_ install the test plugins
install-pluginLTLIBRARIES:

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

libsample_vala_la_VALASOURCES = \
	idol-sample-vala-plugin.vala

libsample_vala_la_SOURCES = \
	idol-sample-vala-plugin.vala.stamp \
	$(libsample_vala_la_VALASOURCES:.vala=.c)

libsample_vala_la_LDFLAGS = $(modules_flags)
libsample_vala_la_CPPFLAGS = $(common_defines)

libsample_vala_la_CFLAGS = \
	$(DEPENDENCY_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(DBUS_CFLAGS)		\
	$(AM_CFLAGS)		\
	-I$(top_srcdir)/	\
	-I$(top_srcdir)/lib	\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/plugins

idol-sample-vala-plugin.vala.stamp: $(libsample_vala_la_VALASOURCES)
	$(VALAC) $(VALAFLAGS) -C --vapidir=$(top_srcdir)/bindings/vala --pkg=idol $^
	touch $@

endif

EXTRA_DIST = $(plugin_in_files) $(libsample_vala_la_VALASOURCES) idol-sample-vala-plugin.vala.stamp

MAINTAINERCLEANFILES = \
	$(libsample_vala_la_VALASOURCES:.vala=.c)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)
