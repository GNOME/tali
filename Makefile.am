
Gamesdir = $(datadir)/gnome/apps/Games
scoredir=$(localstatedir)/games

SUBDIRS = pix help

INCLUDES =					\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DGNOMEPIXMAPDIR=\""$(datadir)/pixmaps"\"	\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)				\
	$(CURSES_INCLUDEDIR)

if HAS_CURSES
CTALI=ctali
endif


bin_PROGRAMS = gtali	$(CTALI)


gtali_SOURCES =       \
        gyahtzee.c       \
        gyahtzee.h       \
        clist.c          \
        yahtzee.h        \
        yahtzee.c        \
        computer.c       \
        setup.c

ctali_SOURCES =       \
        cyahtzee.c       \
        cyahtzee.h       \
        yahtzee.c        \
        yahtzee.h        \
	scores.c         \
	scores.h         \
        computer.c 

gtali_LDADD =				\
	$(GNOME_LIBDIR)				\
	$(GNOMEUI_LIBS)				\
	$(INTLLIBS)

ctali_LDADD = \
	$(CURSES_LIBS) 				\
	$(INTLLIBS)

EXTRA_DIST = gtali.desktop README AUTHORS

Games_DATA = gtali.desktop

install-data-local:
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-touch $(DESTDIR)$(scoredir)/gtali.scores
	-chown games.games $(DESTDIR)$(scoredir)/gtali.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gtali.scores	
	-chgrp games $(DESTDIR)$(bindir)/gtali && chmod 2111 $(DESTDIR)$(bindir)/gtali
