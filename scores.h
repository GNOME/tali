#ifndef _scores_H_
#define _scores_H_
/*
 * Gyatzee: Gnomified Yahtzee game.
 * (C) 1998 the Free Software Foundation
 *
 * File:   scores.h
 *
 * Author: Scott Heavner
 *
 * This program is based on based on orest zborowski's curses based
 * yahtze (c)1992.
 */

void update_scorefile(void);
int read_score(FILE *fp, char *name, char *date, int *score);

#endif /* _scores_H_ */

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
*/   
