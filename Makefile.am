scoredir=$(localstatedir)/games
Gamesdir = $(datadir)/apps/Games

SUBDIRS = pix

INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DGNOMEPIXMAPDIR=\""${Pixmapdir}"\"		\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = gyahtzee #cyahtzee

gyahtzee_SOURCES =       \
        gyahtzee.c       \
        gyahtzee.h       \
        clist.c          \
        yahtzee.h        \
        yahtzee.c        \
        computer.c       \
        setup.c

#cyahtzee_SOURCES =       \
#        cyahtzee.c       \
#        cyahtzee.h       \
#        yahtzee.c        \
#        yahtzee.h        \
#	scores.c         \
#	scores.h         \
#        computer.c 

gyahtzee_LDADD =				\
	$(GNOME_LIBDIR)				\
	$(GNOMEUI_LIBS)				\
	$(INTLLIBS)

#cyahtzee_LDADD = \
#	-L/usr/local -lncurses \
#	$(INTLLIBS)

EXTRA_DIST = gyahtzee.desktop README AUTHORS

Games_DATA = gyahtzee.desktop

install-data-local:
	-$(mkinstalldirs) $(scoredir)
	-touch $(scoredir)/gyahtzee.scores
	-chown games.games $(scoredir)/gyahtzee.scores
	-chmod 664 $(scoredir)/gyahtzee.scores	
	-chgrp games $(bindir)/gyahtzee && chmod 2111 $(bindir)/gyahtzee
