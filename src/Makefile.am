bin_PROGRAMS = gtali

gtali_SOURCES =       \
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
	games-stock.c    \
	games-stock.h

if ENABLE_SETGID
gtali_SOURCES +=		\
	games-setgid-io.c		\
	games-setgid-io.h
endif # ENABLE_SETGID

gtali_CPPFLAGS = \
	-I$(top_srcdir) \
	$(AM_CPPFLAGS)

gtali_CFLAGS = \
	-DDATA_DIRECTORY=\"$(datadir)/gtali\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	-DICON_THEME_DIRECTORY="\"$(datadir)/icons\"" \
	-DSCORESDIR="\"$(scoredir)\""				\
	$(GTK_CFLAGS) \
	$(RSVG_CFLAGS) \
	$(AM_CFLAGS)
		
gtali_LDADD = \
	$(GTK_LIBS)	\
	$(RSVG_LIBS) \
	$(INTLLIBS)

install-exec-hook:
	-if test "$(setgid)" = "true"; then \
	  chgrp $(scores_group) $(DESTDIR)$(bindir)/gtali && chmod 2555 $(DESTDIR)$(bindir)/gtali ;\
	fi

-include $(top_srcdir)/git.mk
