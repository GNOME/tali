#ifndef _CYAHTZEE_H
#define _CYAHTZEE_H

#define SCOREDIR "/usr/local/lib"
#define SCOREFNAME "yahtzee.sco"	/* must allow .L extension */

#define NUM_TOP_PLAYERS 10

/*
   your linux either:
   a) has no rename, or
   b) can't rename regular files if newname exists
*/
/* #define HAS_RENAME			/* comment out if you don't */


#ifdef NOCOLORCURSES
#define BGColorOn()
#define DiceColorOn()      standout()
#define DiceColorOff()     standend()
#define PlayerColorOn()	   standout()
#define PlayerColorOff()   standend()
#define TotalsColorOn()	   standout()
#define TotalsColorOff()   standend()
#define HScoreColorOn()
#define HScoreColorOff()
#define ACS_HLINE		'-'
#define ACS_VLINE		'|'
#define ACS_LRCORNER		'+'
#define ACS_URCORNER		'+'
#define ACS_LLCORNER		'+'
#define ACS_ULCORNER		'+'
#define ACS_LTEE		'+'
#define ACS_RTEE		'+'
#define ACS_TTEE		'-'
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

#endif /* _CYAHTZEE_H */


