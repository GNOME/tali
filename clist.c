/*
 * Gyatzee: Gnomified Yahtzee game.
 * (C) 1998 the Free Software Foundation
 *
 * File:   clist.c
 *
 * Author: Scott Heavner
 *
 *   Scoring is done using a clist and handled in this file.
 *
 *   Variables are exported in gyahtzee.h
 *
 *   This file is largely based upon GTT code by Eckehard Berns.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <config.h>
#include <gnome.h>

#include <stdio.h>

#include "yahtzee.h"
#include "gyahtzee.h"

void
update_score_cell(GtkCList * clist, gint row, gint col, int val)
{
        char buf[5] = "    ";
        
        if (val >= 0)
                sprintf(buf,"%4d",val);
        gtk_clist_set_text(clist,row,col,buf);
}


/* This is full of cheats.  Goes into gtk code, access 
 * structures that would probably be private under C++.  Works now 
 * because clist*title_pass/active() doesn't do a redraw, may have to be
 * modified later. */
void
ShowoffPlayer(GtkCList * clist, int player, int so)
{
        gint column;
        GtkButton *button;
        
        g_return_if_fail (clist != NULL);
        
        column = player + 1;
        
        if (column < 0 || column >= clist->columns)
                return;
        
        button = GTK_BUTTON(clist->column[column].button);

        if (so) {
                button->button_down = TRUE;
                gtk_widget_set_state (GTK_WIDGET (button), GTK_STATE_ACTIVE);
        } else {
                button->button_down = FALSE;
                gtk_widget_set_state (GTK_WIDGET (button), GTK_STATE_NORMAL);
        }
  
        gtk_widget_draw(GTK_WIDGET(button),NULL);

}

static gint LastScoredRow = -1;

/* This function is called for both select and unselect events.  If the
 * unselect event is for an event that the pointer isn't on, it'll be dropped
 * on the floor.  Otherwise, it will be thought of as a select event and
 * processed properly.  This takes care of the problem of having to de-select
 * and then re-select rows in multi-player games. -- pschwan@cmu.edu */
static gint
select_row(GtkCList * clist, gint row, gint col, GdkEventButton * event)
{
        int field = row, actual_row, actual_col;

	if (!event) 
                return FALSE;

	gtk_clist_get_selection_info(GTK_CLIST(clist), event->x, event->y,
				     &actual_row, &actual_col);

	if (row != actual_row)
	  /* this is an unselect event for a row that the mouse pointer isn't
	   * on.  take evasive maneuvers, commander. */
	  return FALSE;

        LastScoredRow = -1;

	switch (row) {

	case (R_UTOTAL):	/* Can't select total/blank rows */
	case (R_BONUS):
	case (R_BLANK1):
	case (R_GTOTAL):
	case (R_LTOTAL):
                break;

	default:
                /* Adjust for Upper Total / Bonus entries */
                if (field >= NUM_UPPER)
                        field -= 3;
        
                if ( (field < NUM_FIELDS) && 
		     (!players[CurrentPlayer].finished)  ) {
                        if (play_score(CurrentPlayer,field)==SLOT_USED) {
                                say(_("Already used! " 
				      "Where do you want to put that?"));
                        } else {
                                LastScoredRow = row;
                                NextPlayer();
                                return TRUE;
                        }
                }
	}
        
        gtk_clist_unselect_row (clist,row,col);

        return FALSE;
}


GtkWidget * create_clist(void)
{
	GtkStyle *style;
	GdkGCValues vals;
        
	GtkCList *clist;
	char *titles[MAX_NUMBER_OF_PLAYERS+2];
        
	int i;
        
        titles[MAX_NUMBER_OF_PLAYERS+1] = titles[0] = "";
        for (i=0; i<MAX_NUMBER_OF_PLAYERS; i++)
                titles[i+1] = players[i].name;
	clist = GTK_CLIST(gtk_clist_new_with_titles(8,titles));
	gtk_clist_set_selection_mode(clist, GTK_SELECTION_SINGLE);
        
	gtk_clist_set_column_justification(clist, 0,
					   GTK_JUSTIFY_LEFT);
	for (i=1; i<8; i++) {
                gtk_clist_set_column_justification(clist, i,
                                                   GTK_JUSTIFY_RIGHT);
        }
	style = gtk_widget_get_style(GTK_WIDGET(clist));
	g_return_val_if_fail(style != NULL, NULL);
	if (!style->fg_gc[0]) {
                gtk_clist_set_column_width(clist, 0, 140);
                for (i=1; i<7; i++)
                        gtk_clist_set_column_width(clist, i, 95);
	} else {
                /* !!!!!!!!! BROKEN !!!!!!!! */
                gdk_gc_get_values(style->fg_gc[0], &vals);
                gtk_clist_set_column_width(clist, 0, 
                        gdk_string_width(vals.font,"XCLarge Straight [40]XC"));
                for (i=1; i<7; i++)
                        gtk_clist_set_column_width(clist, i, 
                               gdk_string_width(vals.font, "SomeLongName"));
	}
/*	FIXME 
 * clists no longer get a scrolled window to play in the app has to provide
 * it. 
	gtk_clist_set_policy(clist,
                             GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 */
	gtk_signal_connect(GTK_OBJECT(clist), "select_row",
                           GTK_SIGNAL_FUNC(select_row), NULL);
	gtk_signal_connect(GTK_OBJECT(clist), "unselect_row",
			   GTK_SIGNAL_FUNC(select_row), NULL);
	return GTK_WIDGET(clist);
}

static inline void
gyahtzee_append_clist(GtkWidget *clist, char *tmp[], char *firstcol)
{
        tmp[0] = firstcol;
        gtk_clist_append(GTK_CLIST(clist), tmp);
}

void setup_clist(GtkWidget *clist)
{
        char *buf1 = "";  /* Enough for 6 players */
        char *tmp[] = { 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
        int i;
        
       tmp[1] = tmp[2] = tmp[3] = tmp[4] = tmp[5] = tmp[6] = tmp[7] = buf1;

        gtk_clist_freeze(GTK_CLIST(clist));
        gtk_clist_clear(GTK_CLIST(clist));
        
        gtk_clist_column_titles_active(GTK_CLIST(clist));
        for (i=0; i<NumberOfPlayers; i++)
                gtk_clist_set_column_title (GTK_CLIST(clist),i+1,
                                            players[i].name);
        for (;i<MAX_NUMBER_OF_PLAYERS;i++)
                gtk_clist_set_column_title (GTK_CLIST(clist),i+1,"");
        gtk_clist_column_titles_passive(GTK_CLIST(clist));
        
        for (i=0; i < NUM_UPPER; i++)
                gyahtzee_append_clist(clist,tmp,_(FieldLabels[i]));
        gyahtzee_append_clist(clist,tmp,_(FieldLabels[F_UPPERT]));
        gyahtzee_append_clist(clist,tmp,_(FieldLabels[F_BONUS]));
        gyahtzee_append_clist(clist,tmp,buf1);
        for (i=0; i < NUM_LOWER; i++)
                gyahtzee_append_clist(clist,tmp,_(FieldLabels[i+NUM_UPPER]));
        gyahtzee_append_clist(clist,tmp,_(FieldLabels[F_LOWERT]));
        gyahtzee_append_clist(clist,tmp,_(FieldLabels[F_GRANDT]));

        gtk_clist_thaw(GTK_CLIST(clist));

}	

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
*/   
