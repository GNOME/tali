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

gsettings_SCHEMAS = org.gnome.tali.gschema.xml
@GSETTINGS_RULES@

man_MANS = tali.6

desktopdir = $(datadir)/applications
desktop_in_files = gtali.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datadir)/appdata
appdata_DATA = tali.appdata.xml

EXTRA_DIST = \
	$(appdata_DATA) \
	$(desktop_in_files) \
	$(gsettings_SCHEMAS) \
	$(man_MANS) \
	$(pixmap_DATA)

CLEANFILES = $(desktop_DATA)

check-local: $(appdata_DATA) $(desktop_DATA)
	$(APPDATA_VALIDATE) $(appdata_DATA)
	$(DESKTOP_FILE_VALIDATE) $(desktop_DATA)

-include $(top_srcdir)/git.mk
