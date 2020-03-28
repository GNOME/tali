#pragma once
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
extern GtkWidget *create_score_list (void);
extern void setup_score_list (GtkWidget * scorelist);
extern void update_score_cell (GtkWidget * scorelist, int row, int col,
			       int val);
extern void ShowoffPlayerColumn (GtkWidget * scorelist, int player, int so);
extern void ShowoffPlayer (GtkWidget * scorelist, int player, int so);
extern void score_list_set_column_title (GtkWidget * scorelist, int column,
					 const char *str);
extern void update_score_tooltips (void);
/* setup.c */
extern void setup_game (void);
extern void GRenamePlayer (gint playerno);
extern GameType game_type_from_string(const gchar *string);
extern GameType get_new_game_type(void);
extern void set_new_game_type(GameType type);

/* gyahtzee.c */
extern int GyahtzeeAbort;
extern GtkWidget *ScoreList;
extern void update_undo_sensitivity(void);
