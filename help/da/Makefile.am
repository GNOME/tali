## Process this file with automake to produce Makefile.in.

EXTRA_DIST = index.html \
	     topic.dat

gtali_helpdir = $(datadir)/gnome/help/gtali/da

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(gtali_helpdir)
	$(INSTALL_DATA)  $(srcdir)/index.html $(DESTDIR)$(gtali_helpdir)
	$(INSTALL_DATA)  $(srcdir)/topic.dat $(DESTDIR)$(gtali_helpdir)

