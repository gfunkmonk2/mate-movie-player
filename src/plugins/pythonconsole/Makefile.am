plugindir = $(PLUGINDIR)/pythonconsole
plugin_PYTHON = pythonconsole.py console.py

plugin_in_files = pythonconsole.idol-plugin.in

%.idol-plugin: %.idol-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.idol-plugin.in=.idol-plugin)

EXTRA_DIST = $(plugin_in_files) pythonconsole.py console.py

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)
