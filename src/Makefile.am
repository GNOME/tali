bin_PROGRAMS = gtali

gtali_SOURCES =       \
	gyahtzee.c       \
	gyahtzee.h       \
	clist.c          \
	yahtzee.h        \
	yahtzee.c        \
	computer.c       \
	setup.c          \
	$(NULL)

gtali_CPPFLAGS = \
	-I$(top_srcdir) \
	$(AM_CPPFLAGS)

gtali_CFLAGS = \
	$(GTK_CFLAGS)	\
	$(AM_CFLAGS)
		
gtali_LDADD = \
	$(top_builddir)/libgames-support/libgames-support.la \
	$(GTK_LIBS)	\
	$(INTLLIBS)	\
	$(NULL)

if HAVE_GNOME
gtali_CFLAGS += $(GNOME_CFLAGS)
gtali_LDADD += $(GNOME_LIBS)
endif

if HAVE_RSVG
gtali_CFLAGS += $(RSVG_CFLAGS)
gtali_LDADD += $(RSVG_LIBS)
endif

if WITH_GTHREAD
gtali_CFLAGS += $(GHTREAD_CFLAGS)
gtali_LDADD += $(GTHREAD_LIBS)
endif

install-exec-hook:
	-if test "$(setgid)" = "true"; then \
	  chgrp $(scores_group) $(DESTDIR)$(bindir)/gtali && chmod 2555 $(DESTDIR)$(bindir)/gtali ;\
	fi

-include $(top_srcdir)/git.mk
