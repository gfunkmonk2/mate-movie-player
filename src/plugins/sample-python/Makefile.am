plugindir = $(PLUGINDIR)/sample-python
#plugin_PYTHON = sample-python.py

plugin_in_files = sample-python.idol-plugin.in

%.idol-plugin: %.idol-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

#plugin_DATA = $(plugin_in_files:.idol-plugin.in=.idol-plugin)

EXTRA_DIST = $(plugin_in_files) sample-python.py

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)
