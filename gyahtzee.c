/*
 * Gyatzee: Gnomified Dice game.
 * (C) 1998 the Free Software Foundation
 *
 * File:   gyahtzee.c
 *
 * Author: Scott Heavner
 *
 *   Gnome specific yahtzee routines.
 *
 *   Curses routines are located in cyahtzee.c (cyahtzee.c is 
 *   very broken at the moment 980617).
 *
 *   Other gnome specific code is in setup.c and clist.c
 *
 *   Window manager independent routines are in yahtzee.c and computer.c
 *
 *   Variables are exported in yahtzee.h
 *
 *   This program is based on based on orest zborowski's curses based
 *   yahtze (c)1992.
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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <config.h>
#include <gnome.h>
#include <gconf/gconf-client.h>
#include <libgnomeui/gnome-window-icon.h>

#include "yahtzee.h"
#include "gyahtzee.h"

#include "pix/gnome-dice-1.xpm"
#include "pix/gnome-dice-2.xpm"
#include "pix/gnome-dice-3.xpm"
#include "pix/gnome-dice-4.xpm"
#include "pix/gnome-dice-5.xpm"
#include "pix/gnome-dice-6.xpm"
#include "pix/gnome-dice-none.xpm"

int GyahtzeeAbort = 0;  /* Abort program without playing game */

static char *appID="gtali";
static char *appName=N_("Gnome Tali");
static guint lastHighScore = 0;
GtkWidget *ScoreList; 

#ifdef GNOMEPIXMAPDIR
#define PP GNOMEPIXMAPDIR "/" 
#else
#define PP ""
#endif

static char *dicefiles[NUMBER_OF_PIXMAPS] = { PP "gnome-dice-1.xpm",
                                              PP "gnome-dice-2.xpm",
                                              PP "gnome-dice-3.xpm",
                                              PP "gnome-dice-4.xpm",
                                              PP "gnome-dice-5.xpm",
                                              PP "gnome-dice-6.xpm",
                                              PP "gnome-dice-none.xpm" };

static GtkWidget *dicePixmaps[NUMBER_OF_DICE][NUMBER_OF_PIXMAPS];

static GtkWidget *window, *appbar, *diceBox[NUMBER_OF_DICE];
static GtkWidget *diceTable;

static gint gnome_modify_dice( GtkWidget *widget, GdkEventButton *event,
                               gpointer data );
static gint gnome_roll_dice( GtkWidget *widget, GdkEvent *event,
                             gpointer data );


void
YahtzeeIdle()
{
        while (gtk_events_pending())
                gtk_main_iteration();
}


static void
CheerWinner(void)
{
        int winner;

        winner = FindWinner();
        if ( (winner<NumberOfHumans) && !IsCheater ) {
                lastHighScore = gnome_score_log((guint)WinningScore, 
                                                NULL, TRUE);
                if (lastHighScore)
                        ShowHighScores();
        }

	ShowoffPlayer(ScoreList,CurrentPlayer,0);
	ShowoffPlayer(ScoreList,winner,1);

        if (players[winner].name)
                say(_("%s wins the game with %d points"),
                    players[winner].name,
		    WinningScore);
	else
                say(_("Game over!"));
}

void 
NextPlayer(void)
{
        if (GameIsOver()) {
                CheerWinner();
                return;
        } 
        NumberOfRolls = 0;

	ShowoffPlayer(ScoreList,CurrentPlayer,0);

        if ( ++CurrentPlayer >= NumberOfPlayers )
                CurrentPlayer = 0;

	ShowoffPlayer(ScoreList,CurrentPlayer,1);

        if (players[CurrentPlayer].name) {
                if (players[CurrentPlayer].comp) {
                        say( _("Computer playing for %s"),
                            players[CurrentPlayer].name);
                } else {
                        say(_("%s! -- You're up."),
                            players[CurrentPlayer].name);
                }
        }

#if 0
	/* This is the old way.  I disapprove. */
	if (HumansAreDone())      /* Because of multiple yahtzees, humans */
	       NextPlayer();     /* may finish before computers */
#else
	/* If player 2 had more yahtzees than player 1, player 1 would be asked
	 * to roll when he had no moves left, and player 2 would never get a
	 * chance to roll, thus the game deadlocked. -- pschwan@cmu.edu */
	if (players[CurrentPlayer].finished)
	  NextPlayer();
#endif

        SelectAllDice();
        RollSelectedDice();

        if (players[CurrentPlayer].comp) {
                ComputerTurn(CurrentPlayer);
                NextPlayer();
                return;
        }
}

void
ShowPlayer(int num, int field)
{
	int i;
	int line;
	int upper_tot;
	int lower_tot;
        int bonus = -1;
        int score;
        
	for (i = 0; i < NUM_FIELDS; ++i) {
                
		if (i == field || field == -1) {
                        
			line = i;
                        
			if (i >= NUM_UPPER)
				line += 3;
                        
			if (players[num].used[i])
				score = players[num].score[i];
                        else
                                score = -1;
                        update_score_cell(ScoreList, line, num+1, score);
                }
	}
        
	upper_tot = upper_total(num);
	lower_tot = lower_total(num);
        
	if (upper_tot >= 63) {
                bonus = 35;
		upper_tot += bonus;
	}
        
        update_score_cell(ScoreList, R_BONUS, num+1, bonus);
        update_score_cell(ScoreList, R_UTOTAL, num+1, upper_tot);
        update_score_cell(ScoreList, R_LTOTAL, num+1, lower_tot);
        update_score_cell(ScoreList, R_GTOTAL, num+1, upper_tot+lower_tot);
}

static gint 
quit_game(GtkWidget *widget, gpointer data)
{
        gtk_main_quit();
	return TRUE;
}


static void 
GyahtzeeNewGame(void)
{
        int i;

        NewGame();
        setup_score_list(ScoreList);

	for (i=0; i<NumberOfPlayers; i++)
                ShowoffPlayer(ScoreList,i,0);
	ShowoffPlayer(ScoreList,0,1);

        /* All players are computers, start game immediately */
        if (players[CurrentPlayer].comp) {
                CurrentPlayer = NumberOfComputers - 1;
                NextPlayer();
        }
}


static gint 
new_game_callback(GtkWidget *widget, gpointer data)
{
        GyahtzeeNewGame();
	return FALSE;
}

static void
UpdateDiePixmap(DiceInfo *d)
{
        static int last_val[NUMBER_OF_DICE] = { 0 } ;
        int new_val = d->val - 1;
        int dicebox = d - DiceValues;
        
        if (d->sel)
                new_val = DIE_SELECTED_PIXMAP;
        
        if (last_val[dicebox]!=new_val) {
                
                gtk_widget_hide(dicePixmaps[dicebox][last_val[dicebox]]);
                gtk_widget_show(dicePixmaps[dicebox][new_val]);
                
                last_val[dicebox] = new_val;
        }
        
}


void
UpdateDie(int no)
{
        /* printf("Update die #%d\n",no); */

        UpdateDiePixmap(&DiceValues[no]);
}

/* Callback on dice press */
gint 
gnome_modify_dice( GtkWidget *widget, GdkEventButton *event, gpointer data ) 
{
        DiceInfo *tmp = (DiceInfo *) data;

        /* printf("Pressed die: %d. NumberOfRolls=%d\n",
               tmp - DiceValues + 1, NumberOfRolls); */

        /* Stop play when player is marked finished */
	if (players[CurrentPlayer].finished)
                return TRUE;

        if (NumberOfRolls >= NUM_ROLLS) {
                say(_("You're only allowed three rolls! Select a score box."));
                return TRUE;
        }

#ifdef DEBUG
	if (event->state & GDK_SHIFT_MASK)
	  {
	    /* If the user is holding down shift, cycle through the dice--very
	     * useful for debugging. */
	    tmp->val = (tmp->val + 1)%7;
	    if (!tmp->val)
	      tmp->val++;

	    printf("val before update: %d\n", tmp->val);
	    UpdateDiePixmap(tmp);
	  }
	else
#endif
	  {
	    /* Stop play when player is marked finished */
	    if (players[CurrentPlayer].finished)
	      return TRUE;

	    if (NumberOfRolls >= NUM_ROLLS) {
	      say(_("You're only allowed three rolls! Select a score box."));
	      return TRUE;
	    }
	    
	    tmp->sel = 1 - tmp->sel;

	    UpdateDiePixmap(tmp);
	  }

	return TRUE;
}


/* Callback on Roll! button press */
gint 
gnome_roll_dice( GtkWidget *widget, GdkEvent *event, gpointer data ) {
        if (!players[CurrentPlayer].comp)
                RollSelectedDice();
	return FALSE;
}

void
say(char *fmt, ...)
{
	va_list ap;
	char buf[200];

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	gnome_appbar_pop(GNOME_APPBAR (appbar));
	gnome_appbar_push(GNOME_APPBAR (appbar), buf);
}


static gint
about(GtkWidget *widget, gpointer data)
{
        GtkWidget *about;
        const gchar *authors[] = {
		N_("Scott Heavner"),
		N_("Orest Zborowski - Curses Version (C) 1992"),
		NULL
	};
        gchar *documenters[] = {
                NULL
        };
        /* Translator credits */
        gchar *translator_credits = _("translator_credits");
	GdkPixbuf *pixbuf = NULL;
        
#ifdef ENABLE_NLS
	{
		int i=0;
		while (authors[i] != NULL) { authors[i]=_(authors[i]); i++; }
	}
#endif

	pixbuf = gdk_pixbuf_new_from_file (GNOMEPIXMAPDIR "/gnome-gtali.png",
			NULL);

        about = gnome_about_new (appName, VERSION,
				 _("(C) 1998 the Free Software Fundation"),
				 _("Gnome Tali"),
				 (const char **)authors,
				 (const char **)documenters,
				 strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
				 pixbuf);
        gtk_widget_show (about);
	return FALSE;
}

void
ShowHighScores(void)
{
        gnome_scores_display (appName, appID, NULL, lastHighScore);
}

static gint 
score_callback(GtkWidget *widget, gpointer data)
{
        ShowHighScores();
	return FALSE;
}

static gint 
undo_callback(GtkWidget *widget, gpointer data)
{
        int i;
	if (!GameIsOver()) {
                for (i=0; i<NumberOfPlayers; i++)
                        ExecSingleUndo(SCORE_OK);
                say(_("Cheater! Any high scores will not be recorded."));
        }
	return FALSE;
}


/* Define menus later so we don't have to proto callbacks? */
GnomeUIInfo gamemenu[] = {
        GNOMEUIINFO_MENU_NEW_GAME_ITEM(new_game_callback, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_UNDO_MOVE_ITEM(undo_callback, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_SCORES_ITEM(score_callback, NULL),
	GNOMEUIINFO_SEPARATOR,
        GNOMEUIINFO_MENU_EXIT_ITEM(quit_game, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo settingsmenu[] = {
	GNOMEUIINFO_MENU_PREFERENCES_ITEM(setup_game, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo helpmenu[] = {
        GNOMEUIINFO_HELP("gtali"),
	GNOMEUIINFO_MENU_ABOUT_ITEM(about, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo mainmenu[] = {
	GNOMEUIINFO_MENU_GAME_TREE(gamemenu),
	GNOMEUIINFO_MENU_SETTINGS_TREE(settingsmenu),
	GNOMEUIINFO_MENU_HELP_TREE(helpmenu),
	GNOMEUIINFO_END
};


static void
LoadDicePixmaps(void)
{
        GtkWidget *tmp;
	GdkPixbuf *pixbuf;
	const char **xpm_data[] = {
		gnome_dice_1_xpm,
		gnome_dice_2_xpm,
		gnome_dice_3_xpm,
		gnome_dice_4_xpm,
		gnome_dice_5_xpm,
		gnome_dice_6_xpm,
		gnome_dice_none_xpm
	};
	int i, j;

	for (i=0; i<NUMBER_OF_PIXMAPS; i++) {
		tmp = NULL;

		/* Check for files w/pixmaps, override compiled defaults */
		if (g_file_test(dicefiles[i], G_FILE_TEST_EXISTS))
			tmp = gtk_image_new_from_file(dicefiles[i]);

		if (tmp != NULL) {
			dicePixmaps[0][i] = tmp;
		} else { /* Load default dice */
			pixbuf = gdk_pixbuf_new_from_xpm_data(xpm_data[i]);
			dicePixmaps[0][i] = gtk_image_new_from_pixbuf (pixbuf);
			g_object_unref(pixbuf);
		}

		for (j=0; j<NUMBER_OF_DICE; j++) {
			pixbuf = gtk_image_get_pixbuf(
				GTK_IMAGE(dicePixmaps[0][i]));
			dicePixmaps[j][i] = gtk_image_new_from_pixbuf(pixbuf);
		}
	}
}


static void
GyahtzeeCreateMainWindow(void)
{
        GtkWidget *all_boxes;
	GtkWidget *mbutton;
	GtkWidget *tmp;
	int i, j;

        gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
        g_signal_connect(G_OBJECT(window), "delete_event",
                         G_CALLBACK(quit_game), NULL);

	/*---- Menus ----*/
	gnome_app_create_menus(GNOME_APP(window), mainmenu);


	/*---- Status Bar ----*/
	appbar = gnome_appbar_new(FALSE, TRUE, GNOME_PREFERENCES_USER);
	gnome_app_set_statusbar(GNOME_APP(window), appbar);
	gnome_appbar_push( GNOME_APPBAR (appbar),
			   _("Select dice to re-roll, press Roll!,"
                            " or select score slot."));

	gnome_app_install_menu_hints(GNOME_APP (window), mainmenu);

	/*---- Content ----*/
        all_boxes = gtk_hbox_new(FALSE, 0);
	gnome_app_set_contents(GNOME_APP(window), all_boxes);

        /* Retreive dice pixmaps from memory or files */
        LoadDicePixmaps();
        
	/* Put all the dice in a vertical column */
        diceTable = gtk_table_new(NUMBER_OF_DICE+1, 1, FALSE);
	gtk_box_pack_start(GTK_BOX(all_boxes), diceTable, FALSE, TRUE, 0);
 	for (i=0; i<NUMBER_OF_DICE; i++) {

                diceBox[i] = gtk_event_box_new();
                gtk_table_attach(GTK_TABLE(diceTable), diceBox[i], 0, 1,
                                 i, i+1, 0, 0, 5, 5);
                tmp = gtk_vbox_new(FALSE, 0);
                gtk_container_add (GTK_CONTAINER (diceBox[i]),
                                   tmp);
                
                for (j=0; j<NUMBER_OF_PIXMAPS; j++)
                        gtk_box_pack_start(GTK_BOX(tmp), 
                                           dicePixmaps[i][j], FALSE, FALSE, 0);
                
                gtk_widget_set_events( diceBox[i],
                                       gtk_widget_get_events(diceBox[i]) |
                                       GDK_BUTTON_PRESS_MASK );
                
                g_signal_connect(G_OBJECT(diceBox[i]), "button_press_event",
                                 G_CALLBACK(gnome_modify_dice), &DiceValues[i]);

                gtk_widget_show(dicePixmaps[i][0]);
                gtk_widget_show(tmp);
                gtk_widget_show(diceBox[i]);
	}
	mbutton = gtk_button_new_with_label(_(" Roll ! "));
	gtk_table_attach(GTK_TABLE(diceTable), mbutton, 0, 1, i, i+1,
			 0, 0, 5, 5);
        g_signal_connect(G_OBJECT(mbutton), "clicked",
                         G_CALLBACK(gnome_roll_dice), NULL);
	gtk_widget_show(mbutton);
	gtk_widget_show(diceTable);
        
	/* Scores displayed in score list */
        ScoreList = create_score_list();
        gtk_box_pack_end(GTK_BOX(all_boxes), ScoreList, TRUE, TRUE, 0);
        setup_score_list(ScoreList);
        gtk_widget_show(ScoreList);
        
	gtk_widget_show(all_boxes);

        gtk_widget_show(window);

}


int
main (int argc, char *argv[])
{
        GConfClient *client;
        GError *err = NULL;
        GSList *name_list = NULL;
        gchar *newname, *PrefLoc;
        gint i;

	gnome_score_init(appID);

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* Reset all yahtzee variables before parsing args */
        YahtzeeInit();

	/* Create gnome client */
        gnome_init_with_popt_table(appID, VERSION, argc, argv,
				   yahtzee_options, 0, NULL);
        gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-gtali.png");

        client = gconf_client_get_default ();
        NumberOfComputers = gconf_client_get_int(client, "/apps/gtali/NumberOfComputerOpponents", &err);
        if(err) {
                g_warning (G_STRLOC ": gconf error: %s\n", err->message);
                g_error_free(err);
                err = NULL;
        }

	NumberOfHumans = gconf_client_get_int(client, "/apps/gtali/NumberOfHumanOpponents", &err);
        if(err) {
                g_warning (G_STRLOC ": gconf error: %s\n", err->message);
                g_error_free(err);
                err = NULL;
        }

 	ExtraYahtzeeBonus = gconf_client_get_int(client, "/apps/gtali/ExtraYahtzeeBonus", &err);
        if(err) {
                g_warning (G_STRLOC ": gconf error: %s\n", err->message);
                g_error_free(err);
                err = NULL;
        }

 	ExtraYahtzeeBonusVal = gconf_client_get_int(client, "/apps/gtali/ExtraYahtzeeBonusVal", &err);
        if(err) {
                g_warning (G_STRLOC ": gconf error: %s\n", err->message);
                g_error_free(err);
                err = NULL;
        }

 	ExtraYahtzeeJoker = gconf_client_get_int(client, "/apps/gtali/ExtraYahtzeeJoker", &err);
        if(err) {
                g_warning (G_STRLOC ": gconf error: %s\n", err->message);
                g_error_free(err);
                err = NULL;
        }

        /* Read in new player names */
        name_list = gconf_client_get_list(client, "/apps/gtali/PlayerNames",
                                          GCONF_VALUE_STRING, &err);

        if(err) {
                g_warning (G_STRLOC ": gconf error: %s\n", err->message);
                g_error_free(err);
                err = NULL;
        }

        for(i = 0; i < MAX_NUMBER_OF_PLAYERS && name_list; i++) {
                if(name_list->data)
                        players[i].name = g_strdup(name_list->data);
                
                name_list = g_slist_next(name_list);
        }
        g_slist_free(name_list);

	/* For some options, we don't want to play a game */
	if (!GyahtzeeAbort) {

	  window = gnome_app_new(appID, appName);
	  
	  GyahtzeeCreateMainWindow();

	  /* Need to roll the dice once */
	  GyahtzeeNewGame();
        
	  gtk_main();

	}
        
	return 0;
}

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
*/   
