bin_PROGRAMS = tali

tali_SOURCES =       \
	gyahtzee.c       \
	gyahtzee.h       \
	clist.c          \
	yahtzee.h        \
	yahtzee.c        \
	computer.c       \
	setup.c          \
	games-frame.c    \
	games-frame.h    \
	games-score.h			\
	games-score.c			\
	games-scores.c			\
	games-scores.h			\
	games-scores-dialog.c	\
	games-scores-dialog.h	\
	games-scores-backend.c	\
	games-scores-backend.h	\
	$(BUILT_SOURCES)

BUILT_SOURCES = tali-resources.c

tali-resources.c: $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies tali.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-source tali.gresource.xml

tali_CPPFLAGS = \
	-I$(top_srcdir) \
	$(AM_CPPFLAGS)

tali_CFLAGS = \
	-DDATA_DIRECTORY=\"$(datadir)/tali\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	-DICON_THEME_DIRECTORY="\"$(datadir)/icons\"" \
	$(TALI_CFLAGS)
		
tali_LDADD = \
	$(TALI_LIBS)

MAINTAINERCLEANFILES = tali-resources.c

EXTRA_DIST = tali.gresource.xml

-include $(top_srcdir)/git.mk
