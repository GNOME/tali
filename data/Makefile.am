pixmapdir = $(pkgdatadir)/gtali/pixmaps
pixmap_DATA = \
	gnome-dice-1.svg \
	gnome-dice-2.svg \
	gnome-dice-3.svg \
	gnome-dice-4.svg \
	gnome-dice-5.svg \
	gnome-dice-6.svg \
	gnome-dice-none.svg \
	kismet1.svg \
	kismet2.svg \
	kismet3.svg \
	kismet4.svg \
	kismet5.svg \
	kismet6.svg \
	kismet-none.svg

schema_in_files = gtali.schemas.in
if HAVE_GNOME
schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
endif

@INTLTOOL_SCHEMAS_RULE@

man_MANS = gtali.6

EXTRA_DIST = README AUTHORS $(schema_in_files) $(man_MANS) $(pixmap_DATA)

gamesdir = $(datadir)/applications
games_in_files = gtali.desktop.in.in
games_DATA = $(games_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

install-data-local:
if GCONF_SCHEMAS_INSTALL
	-GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/gtali/$(schema_DATA)
endif

DISTCLEANFILES = $(games_DATA) $(schema_DATA)

-include $(top_srcdir)/git.mk
