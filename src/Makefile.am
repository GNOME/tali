bin_PROGRAMS = gtali

gtali_SOURCES =       \
	gyahtzee.c       \
	gyahtzee.h       \
	clist.c          \
	yahtzee.h        \
	yahtzee.c        \
	computer.c       \
	setup.c          \
	games-frame.c    \
	games-frame.h

gtali_CPPFLAGS = \
	-I$(top_srcdir) \
	$(AM_CPPFLAGS)

gtali_CFLAGS = \
	-DDATA_DIRECTORY=\"$(datadir)/gtali\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	$(GTK_CFLAGS) \
	$(RSVG_CFLAGS) \
	$(AM_CFLAGS)
		
gtali_LDADD = \
	$(top_builddir)/libgames-support/libgames-support.la \
	$(GTK_LIBS)	\
	$(RSVG_LIBS) \
	$(INTLLIBS)

install-exec-hook:
	-if test "$(setgid)" = "true"; then \
	  chgrp $(scores_group) $(DESTDIR)$(bindir)/gtali && chmod 2555 $(DESTDIR)$(bindir)/gtali ;\
	fi

-include $(top_srcdir)/git.mk
