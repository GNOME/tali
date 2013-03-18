SUBDIRS = icons

pixmapdir = $(datadir)/tali
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

gsettings_in_file = org.gnome.tali.gschema.xml.in
gsettings_SCHEMAS = $(gsettings_in_file:.xml.in=.xml)
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

man_MANS = tali.6

EXTRA_DIST = $(gsettings_in_file) $(man_MANS) $(pixmap_DATA)

gamesdir = $(datadir)/applications
games_in_files = gtali.desktop.in.in
games_DATA = $(games_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

CLEANFILES = $(games_DATA) $(gsettings_SCHEMAS)
DISTCLEANFILES = $(games_DATA) $(gsettings_SCHEMAS)

-include $(top_srcdir)/git.mk
