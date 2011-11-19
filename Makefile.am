SUBDIRS = src data

if BUILD_HELP
SUBDIRS += help
endif

install-data-local:
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-if test -e $(DESTDIR)$(scoredir)/gtali.scores ; then mv $(DESTDIR)$(scoredir)/gtali.scores $(DESTDIR)$(scoredir)/gtali.Regular.scores ; fi
	-touch $(DESTDIR)$(scoredir)/gtali.Regular.scores
	-chown $(scores_user):$(scores_group) $(DESTDIR)$(scoredir)/gtali.Regular.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gtali.Regular.scores	
	-touch $(DESTDIR)$(scoredir)/gtali.Colors.scores
	-chown $(scores_user):$(scores_group) $(DESTDIR)$(scoredir)/gtali.Colors.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gtali.Colors.scores

-include $(top_srcdir)/git.mk
