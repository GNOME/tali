
Gamesdir = $(datadir)/applications
scoredir=$(localstatedir)/games

SUBDIRS = pix help

INCLUDES =						\
	$(GNOME_GAMES_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DDATADIR=\""$(datadir)"\"		   \
	-DGNOMEPIXMAPDIR=\""$(datadir)/pixmaps"\"	\
	-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED \
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
	$(GNOME_GAMES_LIBS) \
	$(INTLLIBS)

ctali_LDADD = \
	$(CURSES_LIBS) 				\
	$(INTLLIBS)

schemadir   = $(sysconfdir)/gconf/schemas
schema_DATA = gtali.schemas

EXTRA_DIST = gtali.desktop.in README AUTHORS $(schema_DATA)

Games_in_files = gtali.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

install-data-local:
	-GCONF_CONFIG_SOURCE=$(GCONF_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-touch $(DESTDIR)$(scoredir)/gtali.scores
	-chown games.games $(DESTDIR)$(scoredir)/gtali.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gtali.scores	
	-chgrp games $(DESTDIR)$(bindir)/gtali && chmod 2111 $(DESTDIR)$(bindir)/gtali
