
Gamesdir = $(datadir)/apps/Games
scoredir=$(localstatedir)/games

SUBDIRS = pix

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
	-$(mkinstalldirs) $(scoredir)
	-touch $(scoredir)/gtali.scores
	-chown games.games $(scoredir)/gtali.scores
	-chmod 664 $(scoredir)/gtali.scores	
	-chgrp games $(bindir)/gtali && chmod 2111 $(bindir)/gtali
