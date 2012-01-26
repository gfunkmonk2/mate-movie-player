plugindir = $(PLUGINDIR)/jamendo
uidir = $(plugindir)
plugin_PYTHON = jamendo.py
plugin_in_files = jamendo.idol-plugin.in

%.idol-plugin: %.idol-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.idol-plugin.in=.idol-plugin)
ui_DATA = jamendo.ui

EXTRA_DIST = $(plugin_in_files) $(ui_DATA) jamendo.py

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)
