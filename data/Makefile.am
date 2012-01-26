SUBDIRS = icons

bugreportdir = $(libexecdir)/idol
bugreport_SCRIPTS = idol-bugreport.py

man_MANS = idol.1 idol-video-thumbnailer.1

stuffdir = $(pkgdatadir)
stuff_DATA =				\
	filmholes.png			\
	filmholes-big-left.png		\
	filmholes-big-right.png		\
	idol.ui			\
	fullscreen.ui			\
	playlist.ui			\
	plugins.ui			\
	mozilla-viewer.ui		\
	properties.ui			\
	uri.ui				\
	video-list.ui

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

# idol.desktop.in.in.in = original
# idol.desktop.in.in = by configure, includes the X-Mate-Bugzilla
# idol.desktop.in = includes the mime-types
# idol.desktop = intltool-ised

desktop_in_files = idol.desktop.in
desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

idol.desktop.in: idol.desktop.in.in mime-type-list.txt desktop.sh
	$(AM_V_GEN) cat idol.desktop.in.in | sed 's,@FULL_LIBEXECDIR@,$(FULL_LIBEXECDIR),' > $@ &&\
	$(SHELL) $(srcdir)/desktop.sh $(srcdir)/mime-type-list.txt	\
	>> $@

nodist_noinst_HEADERS = idol-mime-types.h caja-audio-mime-types.h
idol-mime-types.h: mime-type-include.sh mime-type-list.txt mime-functions.sh
	$(AM_V_GEN) $(srcdir)/mime-type-include.sh $(srcdir)/mime-type-list.txt	\
	> $@
caja-audio-mime-types.h: mime-type-include.sh mime-type-list.txt mime-functions.sh
	$(AM_V_GEN) $(srcdir)/mime-type-include.sh --caja $(srcdir)/mime-type-list.txt	\
	> $@

update-mateconf.sh: idol-video-thumbnail.schemas.in

idol-video-thumbnail.schemas.in: schemas.sh mime-type-list.txt mime-functions.sh
	$(AM_V_GEN) $(srcdir)/schemas.sh $(srcdir)/mime-type-list.txt $(bindir)	\
	> $@								\
	3> update-mateconf.sh

idol-handlers.schemas.in: handlers.sh
	$(AM_V_GEN) $(srcdir)/handlers.sh > $@

generated_schema_in_files = idol-video-thumbnail.schemas.in idol-handlers.schemas.in
schema_in_files = idol.schemas.in $(generated_schema_in_files)
schemadir = $(MATECONF_SCHEMA_FILE_DIR)
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

idol-video-thumbnailer.1: idol-video-thumbnailer.pod
	$(AM_V_GEN) pod2man -c "" -s 1 -q none -n idol-video-thumbnailer -r MATE $< > $@

install-data-local: $(schema_DATA)
if MATECONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for p in $^ ; do \
		MATECONF_CONFIG_SOURCE=$(MATECONF_SCHEMA_CONFIG_SOURCE) $(MATECONFTOOL) --makefile-install-rule $$p 2>&1 > /dev/null; \
	done \
	fi
endif

EXTRA_DIST = \
	$(man_MANS)			\
	$(stuff_DATA)			\
	$(bugreport_SCRIPTS)		\
	desktop.sh			\
	handlers.sh			\
	mime-type-include.sh		\
	mime-type-list.txt		\
	schemas.sh			\
	mime-functions.sh		\
	idol.schemas.in		\
	idol-video-thumbnailer.1	\
	idol-video-thumbnailer.pod

CLEANFILES = \
	*.bak *.uip core* *.orig *~ 	\
	$(desktop_in_files)		\
	$(desktop_DATA)			\
	$(generated_schema_in_files)	\
	$(schema_DATA)			\
	idol-handlers.schemas.in 	\
	idol-mime-types.h		\
	idol-video-thumbnail.schemas.in\
	caja-audio-mime-types.h	\
	update-mateconf.sh

DISTCLEANFILES =
