#ifndef _CYAHTZEE_H
#define _CYAHTZEE_H

#ifndef SCOREDIR                        /* Should be set by configure */
#define SCOREDIR "/usr/local/lib"
#endif

#define SCOREFNAME "yahtzee.sco"	/* must allow .L extension */

#define NUM_TOP_PLAYERS 10

/* #define HAS_RENAME	*/		/* Set by configure */


#ifdef NO_COLOR_CURSES
#define BGColorOn()
#define DiceColorOn()      standout()
#define DiceColorOff()     standend()
#define PlayerColorOn()	   standout()
#define PlayerColorOff()   standend()
#define TotalsColorOn()	   standout()
#define TotalsColorOff()   standend()
#define HScoreColorOn()
#define HScoreColorOff()
#else
/*=== Be careful with these multi command defines, always use {} w/if ===*/
#define BGColorOn()        attron(COLOR_PAIR(2))
#define DiceColorOn()      attron(COLOR_PAIR(1))
#define DiceColorOff()     attroff(COLOR_PAIR(1));BGColorOn()
#define PlayerColorOn()    attron(A_UNDERLINE|COLOR_PAIR(1))
#define PlayerColorOff()   attroff(A_UNDERLINE|COLOR_PAIR(1));BGColorOn()
#define TotalsColorOn()    attron(A_UNDERLINE|COLOR_PAIR(3))
#define TotalsColorOff()   attroff(A_UNDERLINE|COLOR_PAIR(3));BGColorOn()
#define HScoreColorOn()    attron(COLOR_PAIR(4))
#define HScoreColorOff()   attroff(COLOR_PAIR(4));BGColorOn()
#endif

/* Yuk, but I don't know what curses will have what defined */
#ifndef ACS_HLINE
#define ACS_HLINE	   '-'
#endif
#ifndef ACS_VLINE
#define ACS_VLINE	   '|'
#endif
#ifndef ACS_LRCORNER
#define ACS_LRCORNER	   '+'
#endif
#ifndef ACS_URCORNER
#define ACS_URCORNER	   '+'
#endif
#ifndef ACS_LLCORNER
#define ACS_LLCORNER	   '+'
#endif
#ifndef ACS_ULCORNER
#define ACS_ULCORNER	   '+'
#endif
#ifndef ACS_LTEE
#define ACS_LTEE	   '+'
#endif
#ifndef ACS_RTEE
#define ACS_RTEE	   '+'
#endif
#ifndef ACS_TTEE
#define ACS_TTEE           ACS_HLINE
#endif



#endif /* _CYAHTZEE_H */

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
*/   
