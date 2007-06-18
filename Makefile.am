Gamesdir = $(datadir)/applications

SUBDIRS = pix help

INCLUDES =						\
	$(GNOME_GAMES_CFLAGS)				\
	-I$(top_srcdir)/libgames-support           \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DDATADIR=\""$(datadir)"\"		   \
	-DGNOMEPIXMAPDIR=\""$(datadir)/pixmaps"\"

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
	../libgames-support/libgames-support.la \
	$(GNOME_GAMES_LIBS) \
	$(INTLLIBS)

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_in_files = gtali.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = README AUTHORS $(schema_in_files)

Games_in_files = gtali.desktop.in.in
Games_DATA = $(Games_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

install-data-local:
if GCONF_SCHEMAS_INSTALL
	-GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/gtali/$(schema_DATA)
endif
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-if test -e $(DESTDIR)$(scoredir)/gtali.scores ; then mv $(DESTDIR)$(scoredir)/gtali.scores $(DESTDIR)$(scoredir)/gtali.Regular.scores ; fi
	-touch $(DESTDIR)$(scoredir)/gtali.Regular.scores
	-chown $(scores_user):$(scores_group) $(DESTDIR)$(scoredir)/gtali.Regular.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gtali.Regular.scores	
	-touch $(DESTDIR)$(scoredir)/gtali.Colors.scores
	-chown $(scores_user):$(scores_group) $(DESTDIR)$(scoredir)/gtali.Colors.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gtali.Colors.scores	
	-if test "x$(setgid)" = "xtrue"; then chgrp $(scores_group) $(DESTDIR)$(bindir)/gtali && chmod 2555 $(DESTDIR)$(bindir)/gtali ; fi

DISTCLEANFILES = $(Games_DATA) $(schema_DATA)
