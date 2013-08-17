SUBDIRS = data help po src

install-data-local:
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-if test -e $(DESTDIR)$(scoredir)/gtali.scores ; then mv $(DESTDIR)$(scoredir)/gtali.scores $(DESTDIR)$(scoredir)/gtali.Regular.scores ; fi
	-touch $(DESTDIR)$(scoredir)/gtali.Regular.scores
	-chown $(scores_user):$(scores_group) $(DESTDIR)$(scoredir)/gtali.Regular.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gtali.Regular.scores	
	-touch $(DESTDIR)$(scoredir)/gtali.Colors.scores
	-chown $(scores_user):$(scores_group) $(DESTDIR)$(scoredir)/gtali.Colors.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gtali.Colors.scores

uninstall-local:
	rm -f $(DESTDIR)$(scoredir)/gtali.Regular.scores
	rm -f $(DESTDIR)$(scoredir)/gtali.Colors.scores

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN)

-include $(top_srcdir)/git.mk
