gtali_helpdir = $(datadir)/gnome/help/gtali/C

figs = \
	preferences.png \
	mainwindow.png 

gtali_help_DATA = 	\
	index.html		\
	topic.dat

EXTRA_DIST = gtali.sgml $(gtali_help_DATA) $(figs)

all: index.html

# No-op if db2html failed
index.html: gtali/index.html
	-cd $(srcdir) && cp gtali/index.html index.html

gtali/index.html: gtali.sgml
	-cd $(srcdir) && db2html gtali.sgml

dist-hook:
	-mkdir $(distdir)/gtali
	-mkdir $(distdir)/gtali/stylesheet-images
	cp $(srcdir)/gtali/*.html $(distdir)/gtali
	cp -R $(srcdir)/gtali/stylesheet-images/*.png $(distdir)/gtali/stylesheet-images
	cp $(srcdir)/gtali/*.css $(distdir)/gtali

# No-op if db2html failed
install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(gtali_helpdir)
	files=`echo $(srcdir)/gtali/*.html $(srcdir)/gtali/*.css $(srcdir)/*.png`; \
	if test "$$files" != '$(srcdir)/gtali/*.html $(srcdir)/gtali/*.css $(srcdir)/*.png'; then \
	  for file in $$files; do \
	    basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	    $(INSTALL_DATA) $$file $(DESTDIR)$(gtali_helpdir)/$$basefile; \
	  done; \
	fi
	-$(mkinstalldirs) $(DESTDIR)$(gtali_helpdir)/stylesheet-images
	files=`echo $(srcdir)/gtali/stylesheet-images/*.png`; \
	if test "$$files" != '$(srcdir)/gtali/stylesheet-images/*.png'; then \
	  for file in $$files; do \
	    basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	    $(INSTALL_DATA) $$file $(DESTDIR)$(gtali_helpdir)/stylesheet-images/$$basefile; \
	  done; \
	fi

gtali.ps: gtali.sgml
	db2ps $<

gtali.rtf: gtali.sgml
	db2rtf $<
