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
	games-stock.c    \
	games-stock.h

if ENABLE_SETGID
tali_SOURCES +=		\
	games-setgid-io.c		\
	games-setgid-io.h
endif # ENABLE_SETGID

tali_CPPFLAGS = \
	-I$(top_srcdir) \
	$(AM_CPPFLAGS)

tali_CFLAGS = \
	-DDATA_DIRECTORY=\"$(datadir)/tali\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	-DICON_THEME_DIRECTORY="\"$(datadir)/icons\"" \
	-DSCORESDIR="\"$(scoredir)\""				\
	$(TALI_CFLAGS)
		
tali_LDADD = \
	$(TALI_LIBS)

install-exec-hook:
	-if test "$(setgid)" = "true"; then \
	  chgrp $(scores_group) $(DESTDIR)$(bindir)/tali && chmod 2555 $(DESTDIR)$(bindir)/tali ;\
	fi

-include $(top_srcdir)/git.mk
