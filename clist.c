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

#if 0
void
HiglightPlayer(GtkCList * clist, int player)
{
        gtk_clist_column_title_active (clist, player+1);
}

void
UnHiglightPlayer(GtkCList * clist, int player)
{
        gtk_clist_column_title_passive (clist, player-1);
}
#elif 1
void
HiglightPlayer(GtkCList * clist, int player)
{
}

void
UnHiglightPlayer(GtkCList * clist, int player)
{
}
#else
void
HiglightPlayer(GtkCList * clist, int player)
{
        gint column;
        
        g_return_if_fail (clist != NULL);
        
        column = player + 1;
        
        if (column < 0 || column >= clist->columns)
                return;
        
        gtk_button_pressed(GTK_BUTTON(clist->column[column].button));
        
        gtk_widget_draw (clist->column[column].button,NULL);
}
void
UnHiglightPlayer(GtkCList * clist, int player)
{
        gint column;
        
        g_return_if_fail (clist != NULL);
        
        column = player + 1;
        
        if (column < 0 || column >= clist->columns)
                return;
        
        gtk_button_released(GTK_BUTTON(clist->column[column].button));

}
#endif

static gint
select_row(GtkCList *clist, gint row, gint col, GdkEventButton *event)
{
        int deselect = 0;
        int field = row;

	if (!event) 
                return FALSE;

	/* Can't select total boxes */
	switch (row) {
	case (NUM_UPPER):
	case (NUM_UPPER+1):
	case (NUM_UPPER+2):
	case (SCOREROWS-1):
	case (SCOREROWS-2):
                deselect = 1;
                /* return FALSE; - gtk doesn't seem to care true or false */
                break;
	default:
                /* Adjust for Upper Total / Bonus entries */
                if (field >= NUM_UPPER)
                        field -= 3;
        
                if ( (field < NUM_FIELDS) && (!GameIsOver())  ) {
                        if (play_score(CurrentPlayer,field)==SLOT_USED) {
                                say(_("Already used! " 
				      "Where do you want to put that?"));
                                deselect = 1;
                        } else {
                                NextPlayer();
                                return;
                        }
                }
	}
        
        if (deselect)
                gtk_clist_unselect_row (clist,row,col);
}

static void
unselect_row(GtkCList *clist, gint row, gint col, GdkEventButton *event)
{
	if (!event) return;

	/* g_print("unselect_row, row=%d col=%d button=%d\n", row, col, 
                event->button); */
}

static void
click_column(GtkCList *clist, gint col)
{

        /* g_print("click_column, col=%d\n ", col); */
}


	

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
        
	gtk_clist_column_titles_passive(GTK_CLIST(clist));
        
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
	gtk_clist_set_policy(GTK_CLIST(clist),
                             GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_signal_connect(GTK_OBJECT(clist), "select_row",
                           GTK_SIGNAL_FUNC(select_row), NULL);
	gtk_signal_connect(GTK_OBJECT(clist), "click_column",
                           GTK_SIGNAL_FUNC(click_column), NULL);
	gtk_signal_connect(GTK_OBJECT(clist), "unselect_row",
                           GTK_SIGNAL_FUNC(unselect_row), NULL);
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
                gyahtzee_append_clist(clist,tmp,FieldLabels[i]);
        gyahtzee_append_clist(clist,tmp,FieldLabels[F_UPPERT]);
        gyahtzee_append_clist(clist,tmp,FieldLabels[F_BONUS]);
        gyahtzee_append_clist(clist,tmp,buf1);
        for (i=0; i < NUM_LOWER; i++)
                gyahtzee_append_clist(clist,tmp,FieldLabels[i+NUM_UPPER]);
        gyahtzee_append_clist(clist,tmp,FieldLabels[F_LOWERT]);
        gyahtzee_append_clist(clist,tmp,FieldLabels[F_GRANDT]);

        gtk_clist_thaw(GTK_CLIST(clist));

}	
