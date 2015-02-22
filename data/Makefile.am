SUBDIRS = icons

dist_noinst_DATA = \
	tali-menus.ui

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
desktop_in_files = tali.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

appstream_in_files = tali.appdata.xml.in
appstream_XML = $(appstream_in_files:.xml.in=.xml)
@APPSTREAM_XML_RULES@
@INTLTOOL_XML_RULE@

EXTRA_DIST = \
	$(appstream_in_files) \
	$(desktop_in_files) \
	$(gsettings_SCHEMAS) \
	$(man_MANS) \
	$(pixmap_DATA)

CLEANFILES = \
	$(appstream_XML) \
	$(desktop_DATA)

check-local: $(desktop_DATA)
	$(DESKTOP_FILE_VALIDATE) $(desktop_DATA)

DISTCHECK_CONFIGURE_FLAGS = --enable-appdata-check

-include $(top_srcdir)/git.mk
