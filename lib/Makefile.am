noinst_LTLIBRARIES =					\
	libidolscrsaver.la

common_defines = \
	-D_REENTRANT					\
	-DDBUS_API_SUBJECT_TO_CHANGE			\
	-DMATELOCALEDIR=\""$(datadir)/locale"\"	\
	-DMATECONF_PREFIX=\""/apps/idol"\"		\
	-DDATADIR=\""$(datadir)"\"			\
	-DLIBEXECDIR=\""$(libexecdir)"\"		\
	-DBINDIR=\""$(bindir)"\"			\
	-DLOGO_PATH=DATADIR\"\"G_DIR_SEPARATOR_S\"idol\"G_DIR_SEPARATOR_S\"idol_logo.png\"	\
	$(DISABLE_DEPRECATED)

libidolscrsaver_la_SOURCES =				\
	idol-scrsaver.c				\
	idol-scrsaver.h

libidolscrsaver_la_CFLAGS =				\
	$(common_defines)				\
	$(DEPENDENCY_CFLAGS)				\
	$(WARN_CFLAGS)					\
	$(DBUS_CFLAGS)					\
	$(AM_CFLAGS)

libidolscrsaver_la_LDFLAGS =				\
	$(AM_LDFLAGS)

libidolscrsaver_la_LIBADD =				\
	$(DBUS_LIBS)					\
	$(XTEST_LIBS)
