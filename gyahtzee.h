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
#define R_BLANK1 (R_UTOTAL+2)
#define R_GTOTAL (SCOREROWS-1) 
#define R_LTOTAL (R_GTOTAL-1)

#define NUMBER_OF_PIXMAPS    7
#define DIE_SELECTED_PIXMAP  (NUMBER_OF_PIXMAPS-1)

/* clist.c */
extern GtkWidget *create_clist(void);
extern void setup_clist(GtkWidget *clist);
extern void update_score_cell(GtkCList * clist, gint row, gint col, int val);
extern void ShowoffPlayerColumn(GtkCList * clist, int player, int so);
extern void ShowoffPlayer(GtkCList * clist, int player, int so);

/* setup.c */
extern gint setup_game(GtkWidget *widget, gpointer data);
extern const struct poptOption yahtzee_options[];
extern void GRenamePlayer(gint playerno);

/* gyahtzee.c */
extern int GyahtzeeAbort;
extern GtkWidget *ScoreList; 

#endif /* _Gyahtzee_H_ */

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
*/   
