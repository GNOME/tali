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

#define GY_NEW 0  /* Just so I can play with stuff w/o breaking the dist */

#if GY_NEW
guint  gtk_signal_new			  (const gchar	       *name,
					   GtkSignalRunType	signal_flags,
					   GtkType		object_type,
					   guint		function_offset,
					   GtkSignalMarshaller	marshaller,
					   GtkType		return_val,
					   guint		nparams,
					   ...);
#endif

void
update_score_cell(GtkCList * clist, gint row, gint col, int val)
{
        char buf[5] = "    ";
        
        if (val >= 0)
                sprintf(buf,"%4d",val);
        gtk_clist_set_text(clist,row,col,buf);
}

#if 0
/* This version doesn't do a thing */
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

        if (so)
                gtk_clist_column_title_active (clist, column);
        else
                gtk_clist_column_title_passive (clist, column);

        gtk_widget_draw(GTK_WIDGET(button),NULL);
}
#elif 0
/* This one really does nothing */
void
ShowoffPlayer(GtkCList * clist, int player, int so)
{
}
#else
/* This version is full of cheats.  Goes into gtk code, access 
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

        gtk_clist_column_title_active(clist, column);

        if (so) {
                button->button_down = TRUE;
#if 1
                /* Press it */
                gtk_widget_set_state (GTK_WIDGET (button), GTK_STATE_ACTIVE);
#else
                /* Lighten it */
                gtk_widget_set_state (GTK_WIDGET (button), GTK_STATE_PRELIGHT);
#endif
        } else {
                button->button_down = FALSE;
                gtk_widget_set_state (GTK_WIDGET (button), GTK_STATE_NORMAL);
        }
  
        gtk_widget_draw(GTK_WIDGET(button),NULL);
        gtk_clist_column_title_passive(clist, column);

}
#endif

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
#if GY_NEW
                g_print("Selection should have been blocked!!!\n");
                gtk_signal_emit_stop_by_name(GTK_OBJECT(clist), "select_row");
                return TRUE;
#endif
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
#if GY_NEW
                } else {
                        /* Don't want to unselect row? */
                        return FALSE;
#endif
                }
	}
        
        gtk_clist_unselect_row (clist,row,col);

        return FALSE;
}

#if GY_NEW
static gint
select_row2(GtkCList *clist, gint row, gint col, GdkEventButton *event)
{
#if GY_NEW
        int field = row;

	if (!event) 
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
                                return FALSE;
                        }
                }
	}

        g_print("After: Selection should have been blocked!!!\n");
        gtk_signal_emit_stop_by_name(GTK_OBJECT(clist), "select_row");
        return TRUE;
#else
	return FALSE;
#endif
}
#endif /* GY_NEW */


/* This won't work.  Can't tell if we're being called to deselect 
   selected row  prior to selection of new row or just to deselect 
   current row and leave nothing selected */
/* There's no real reason to have this function since I've taken care of the
 * problem, but I'll leave it here anyways because ... I dunno, just because.
 * -- pschwan@cmu.edu */
#if GY_NEW
static gint
unselect_row(GtkCList *clist, gint row, gint col, GdkEventButton *event)
{
#if GY_NEW
	if (!event) 
                return FALSE;

        /* Multiple humans need to sometimes select score rows that are
         * highlighted */
        if (row==LastScoredRow) {
                LastScoredRow = -1;
                select_row(clist, row, col, event);
        }

        return FALSE;
#else
        return FALSE;
#endif
}
#endif /* GY_NEW */


GtkWidget * create_clist(void)
{
	GtkStyle *style;
	GdkGCValues vals;
        
	GtkWidget *clist;
	char *titles[MAX_NUMBER_OF_PLAYERS+2];
        
	int i;
        
        titles[MAX_NUMBER_OF_PLAYERS+1] = titles[0] = "";
        for (i=0; i<MAX_NUMBER_OF_PLAYERS; i++)
                titles[i+1] = players[i].name;
	clist = gtk_clist_new_with_titles(8,titles);
	gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_SINGLE);
        
	gtk_clist_set_column_justification(GTK_CLIST(clist), 0,
					   GTK_JUSTIFY_LEFT);
	for (i=1; i<8; i++)
                gtk_clist_set_column_justification(GTK_CLIST(clist), i,
                                                   GTK_JUSTIFY_RIGHT);
	style = gtk_widget_get_style(clist);
	g_return_val_if_fail(style != NULL, NULL);
	if (!style->fg_gc[0]) {
                gtk_clist_set_column_width(GTK_CLIST(clist), 0, 140);
                for (i=1; i<7; i++)
                        gtk_clist_set_column_width(GTK_CLIST(clist), i, 95);
	} else {
                /* !!!!!!!!! BROKEN !!!!!!!! */
                gdk_gc_get_values(style->fg_gc[0], &vals);
                gtk_clist_set_column_width(GTK_CLIST(clist), 0, 
                        gdk_string_width(vals.font,"XCLarge Straight [40]XC"));
                for (i=1; i<7; i++)
                        gtk_clist_set_column_width(GTK_CLIST(clist), i, 
                               gdk_string_width(vals.font, "SomeLongName"));
	}
/*	FIXME 
 * clists no longer get a scrolled window to play in the app has to provide
 * it. 
	gtk_clist_set_policy(GTK_CLIST(clist),
                             GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 */
	gtk_signal_connect(GTK_OBJECT(clist), "select_row",
                           GTK_SIGNAL_FUNC(select_row), NULL);
	gtk_signal_connect(GTK_OBJECT(clist), "unselect_row",
			   GTK_SIGNAL_FUNC(select_row), NULL);
#if GY_NEW
/*	gtk_signal_connect_after(GTK_OBJECT(clist), "select_row",
                           GTK_SIGNAL_FUNC(select_row2), NULL); */
#endif
	return clist;
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
        char *tmp[] = { 0, buf1, buf1, buf1, buf1, buf1, buf1, buf1 };

        int i;
        
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
