Gamesdir = $(datadir)/applications
scoredir=$(localstatedir)/games

SUBDIRS = pix help

INCLUDES =						\
	$(GNOME_GAMES_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DDATADIR=\""$(datadir)"\"		   \
	-DGNOMEPIXMAPDIR=\""$(datadir)/pixmaps"\"	\
	-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED

bin_PROGRAMS = gtali


gtali_SOURCES =       \
        gyahtzee.c       \
        gyahtzee.h       \
        clist.c          \
        yahtzee.h        \
        yahtzee.c        \
        computer.c       \
        setup.c

gtali_LDADD =				\
	$(GNOME_GAMES_LIBS) \
	$(INTLLIBS)

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_in_files = gtali.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = gtali.desktop.in README AUTHORS $(schema_in_files)

Games_in_files = gtali.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

install-data-local:
	-GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-touch $(DESTDIR)$(scoredir)/gtali.scores
	-chown games.games $(DESTDIR)$(scoredir)/gtali.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gtali.scores	
	-chgrp games $(DESTDIR)$(bindir)/gtali && chmod 2555 $(DESTDIR)$(bindir)/gtali

CLEANFILES = $(Games_DATA) $(schema_DATA)
