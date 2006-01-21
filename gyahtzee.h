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
#define R_UTOTAL (NUM_UPPER+1)
#define R_BONUS  (R_UTOTAL-1)
#define R_BLANK1 (R_UTOTAL+1)
#define R_GTOTAL (SCOREROWS-1) 
#define R_LTOTAL (R_GTOTAL-1)

/* clist.c */
extern GtkWidget *create_score_list(void);
extern void setup_score_list(GtkWidget *scorelist);
extern void update_score_cell(GtkWidget *scorelist, int row, int col, int val);
extern void ShowoffPlayerColumn(GtkWidget *scorelist, int player, int so);
extern void ShowoffPlayer(GtkWidget *scorelist, int player, int so);
extern void score_list_set_column_title(GtkWidget *scorelist, int column,
                                        const char *str);

/* setup.c */
extern gint setup_game(GtkAction *action, gpointer data);
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
