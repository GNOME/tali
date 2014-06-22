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
	games-scores-backend.h

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

-include $(top_srcdir)/git.mk
