#ifndef _Gyahtzee_H_
#define _Gyahtzee_H_
/*
 * Gyatzee: Gnomified Yahtzee game.
 * (C) 1998 the Free Software Foundation
 *
 * File:   gyahtzee.h
 *
 * Author: Scott Heavner
 *
 */
#define SCOREROWS (NUM_FIELDS+5)

/* Screen row numbers containing totals */
#define R_UTOTAL NUM_UPPER
#define R_BONUS  (R_UTOTAL+1)
#define R_GTOTAL (SCOREROWS-1) 
#define R_LTOTAL (R_GTOTAL-1)

#define NUMBER_OF_PIXMAPS    7
#define DIE_SELECTED_PIXMAP  (NUMBER_OF_PIXMAPS-1)

/* clist.c */
extern GtkWidget *create_clist(void);
extern void setup_clist(GtkWidget *clist);
extern void update_score_cell(GtkCList * clist, gint row, gint col, int val);
extern void HiglightPlayerColumn(GtkCList * clist, int player);
extern void UnHiglightPlayerColumn(GtkCList * clist, int player);

/* setup.c */
extern gint setup_game(GtkWidget *widget, gpointer data);
extern struct argp GyahtzeeParser;

/* gyahtzee.c */
extern int GyahtzeeAbort;

#endif /* _Gyahtzee_H_ */
