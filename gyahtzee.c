/* -*- mode:C; indent-tabs-mode:nil; tab-width:8; c-basic-offset:8 -*- */

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

/* Define a sensible alternative to ngettext if we don't have it. Note that
 * this is only sensible in the context of gtali. */
#ifndef HAVE_NGETTEXT
#define ngettext(one,lots,n) gettext(lots)
#endif

#define DELAY_MS 600

int GyahtzeeAbort = 0;  /* Abort program without playing game */

static char *appID="gtali";
static char *appName=N_("GNOME Tali");
static guint lastHighScore = 0;
static guint last_timeout = 0;
static gboolean ready_to_advance_player;

#ifdef GNOMEPIXMAPDIR
#define PP GNOMEPIXMAPDIR "/gtali/" 
#else
#define PP ""
#endif

static char *dicefiles[NUMBER_OF_PIXMAPS] = { PP "gnome-dice-1.svg",
                                              PP "gnome-dice-2.svg",
                                              PP "gnome-dice-3.svg",
                                              PP "gnome-dice-4.svg",
                                              PP "gnome-dice-5.svg",
                                              PP "gnome-dice-6.svg",
                                              PP "gnome-dice-none.svg" };

static GtkWidget *dicePixmaps[NUMBER_OF_DICE][NUMBER_OF_PIXMAPS];

GtkWidget *window;
GtkWidget *ScoreList;
static GtkWidget *appbar;
static GtkToolItem *diceBox[NUMBER_OF_DICE];
static GtkWidget *rollLabel;
static GtkWidget *mbutton;

static gint gnome_modify_dice (GtkWidget *widget,
                               gpointer data);
static gint gnome_roll_dice (GtkWidget *widget, GdkEvent *event,
                             gpointer data);
void update_score_state (void);
static void roll_set_sensitive (gboolean state);
static void UpdateRollLabel (void);


static void
CheerWinner (void)
{
        int winner;
        int i;

	ShowoffPlayer (ScoreList,CurrentPlayer,0);

        winner = FindWinner ();

        if (winner < 0) {
                for (i = 0; i < NumberOfPlayers; i++) {
                        if (total_score (i) == -winner) {
                                ShowoffPlayer (ScoreList, i, 1);
                        }
                }
                
                say (_("The game is a draw!"));
                return;
        }

	ShowoffPlayer (ScoreList,winner,1);

        if (winner<NumberOfHumans) {
                lastHighScore = gnome_score_log ((guint)WinningScore, 
                                                 NULL, TRUE);
		update_score_state ();
                if (lastHighScore)
                        ShowHighScores ();
        }

        if (players[winner].name)
                say (ngettext("%s wins the game with %d point",
                              "%s wins the game with %d points", WinningScore),
                     players[winner].name,
                     WinningScore);
	else
                say (_("Game over!"));

}

static gboolean
do_computer_turns (void)
{
        if (!players[CurrentPlayer].comp) {
                last_timeout = 0;
                return FALSE;
        }

        if (ready_to_advance_player){
                NextPlayer ();
                return TRUE;
        }

        if (players[CurrentPlayer].finished){
                NextPlayer();
                return TRUE;
        }

        ComputerRolling(CurrentPlayer);
        if ( NoDiceSelected() || (NumberOfRolls>=NUM_ROLLS) ) {
                ComputerScoring(CurrentPlayer);
                ready_to_advance_player = TRUE;
        } else {
                RollSelectedDice();
                UpdateRollLabel();
        }

        if (!DoDelay)
                do_computer_turns();

        return TRUE;
}

void 
NextPlayer (void)
{
        if (GameIsOver ()) {
                if (last_timeout){
                        g_source_remove (last_timeout);
                        last_timeout = 0;
                }
                CheerWinner ();
                return;
        }

        NumberOfRolls = 0;
        ready_to_advance_player = FALSE;
	ShowoffPlayer (ScoreList,CurrentPlayer,0);

        /* Find the next player with rolls left */
        do {
                CurrentPlayer = (CurrentPlayer + 1) % NumberOfPlayers;
        } while (players[CurrentPlayer].finished);
                
	ShowoffPlayer (ScoreList,CurrentPlayer,1);

        if (players[CurrentPlayer].name) {
                if (players[CurrentPlayer].comp) {
                        say (_("Computer playing for %s"),
                             players[CurrentPlayer].name);
                } else {
                        say (_("%s! -- You're up."),
                             players[CurrentPlayer].name);
                }
        }

        SelectAllDice ();
        RollSelectedDice ();
	
        if (players[CurrentPlayer].comp) {
                if (DoDelay){
                        if (!last_timeout)
                                last_timeout = g_timeout_add (DELAY_MS,
                                                              (GSourceFunc)do_computer_turns,
                                                              NULL);
                } else {
                       do_computer_turns();
                }
        }
        UpdateRollLabel ();
}

void
ShowPlayer (int num, int field)
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

                        update_score_cell (ScoreList, line, num+1, score);
                }
	}
        
	upper_tot = upper_total (num);
	lower_tot = lower_total (num);
        
	if (upper_tot >= 63) {
                bonus = 35;
		upper_tot += bonus;
	}
        
        update_score_cell (ScoreList, R_BONUS, num+1, bonus);
        update_score_cell (ScoreList, R_UTOTAL, num+1, upper_tot);
        update_score_cell (ScoreList, R_LTOTAL, num+1, lower_tot);
        update_score_cell (ScoreList, R_GTOTAL, num+1, upper_tot+lower_tot);
}

static gint 
quit_game (GtkWidget *widget, gpointer data)
{
        gtk_main_quit ();
	return TRUE;
}


/* This handles the keys 1..5 for the dice. */
static gint
key_press (GtkWidget *widget, GdkEventKey * event, gpointer data)
{
        gint offset;

        offset = event->keyval - GDK_1;

        if ((offset < 0) || (offset >= NUMBER_OF_DICE)) {
                return FALSE;
        }

        /* I hate faking signal calls. */
        gnome_modify_dice (NULL, &DiceValues[offset]);

        return FALSE;
}

static void 
GyahtzeeNewGame (void)
{
        int i;

        say (_("Select dice to re-roll, press Roll!,"
               " or select score slot."));

        NewGame();
        setup_score_list (ScoreList);
        UpdateRollLabel();

	for (i=0; i<NumberOfPlayers; i++)
                ShowoffPlayer (ScoreList,i,0);
	ShowoffPlayer (ScoreList,0,1);
}


static gint 
new_game_callback (GtkWidget *widget, gpointer data)
{
        GyahtzeeNewGame ();
	return FALSE;
}

static void
UpdateRollLabel (void)
{
        static GString *str = NULL;

        if (!str)
                str = g_string_sized_new(22);
        
        g_string_printf(str, "<b>%s %d/3</b>", _("Roll"), NumberOfRolls);
        gtk_label_set_label(GTK_LABEL(rollLabel), str->str);
	
        roll_set_sensitive ((NumberOfRolls < 3) && !players[CurrentPlayer].comp);
}

static void
UpdateDiePixmap (DiceInfo *d)
{
        static int last_val[NUMBER_OF_DICE] = { 0 } ;
        int new_val = d->val - 1;
        int dicebox = d - DiceValues;
        
        if (d->sel)
                new_val = DIE_SELECTED_PIXMAP;
        
        if (last_val[dicebox]!=new_val) {
                
                gtk_widget_hide (dicePixmaps[dicebox][last_val[dicebox]]);
                gtk_widget_show (dicePixmaps[dicebox][new_val]);
                
                last_val[dicebox] = new_val;
        }
        
}


void
UpdateDie (int no)
{
        /* printf("Update die #%d\n",no); */

        UpdateDiePixmap (&DiceValues[no]);
}

/* Callback on dice press */
gint 
gnome_modify_dice (GtkWidget *widget, gpointer data) 
{
        DiceInfo *tmp = (DiceInfo *) data;

        /* Stop play when player is marked finished */
	if (players[CurrentPlayer].finished)
                return TRUE;

	if (players[CurrentPlayer].comp)
		return TRUE;

        if (NumberOfRolls >= NUM_ROLLS) {
                say(_("You're only allowed three rolls! Select a score box."));
                return TRUE;
        }

        /* Stop play when player is marked finished */
        if (players[CurrentPlayer].finished)
                return TRUE;
        
        if (NumberOfRolls >= NUM_ROLLS) {
                say (_("You're only allowed three rolls! Select a score box."));
                return TRUE;
        }
        
        tmp->sel = 1 - tmp->sel;
        
        UpdateDiePixmap (tmp);

	return TRUE;
}


/* Callback on Roll! button press */
gint 
gnome_roll_dice (GtkWidget *widget, GdkEvent *event, gpointer data) {
        if (! players[CurrentPlayer].comp) {
                RollSelectedDice ();
                UpdateRollLabel ();
        }
	return FALSE;
}

void
say (char *fmt, ...)
{
	va_list ap;
	char buf[200];

	va_start (ap, fmt);
	g_vsnprintf (buf, 200, fmt, ap);
	va_end (ap);

	gnome_appbar_pop (GNOME_APPBAR (appbar));
	gnome_appbar_push (GNOME_APPBAR (appbar), buf);
}


static gint
about (GtkWidget *widget, gpointer data)
{
        const gchar *authors[] = {
                N_("GNOME version (1998):"),
		"Scott Heavner",
		"",
		N_("Console version (1992):"),
		"Orest Zborowski",
		NULL
	};
        
	gtk_show_about_dialog (GTK_WINDOW (window),
			       "name", appName,
			       "version", VERSION,
			       "copyright", "Copyright \xc2\xa9 1998-2005 "
					    "Free Software Foundation, Inc.",
			       "comments", _("A variation on poker with "
					     "dice and less money."),
			       "authors", authors,
			       "translator_credits", _("translator-credits"),
			       NULL);
	
	return FALSE;
}

void
ShowHighScores (void)
{
	GtkWidget *dialog;

	dialog = gnome_scores_display (appName, appID, NULL, lastHighScore);
	if (dialog != NULL) {
		gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                              GTK_WINDOW (window));
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
		gtk_dialog_set_has_separator (GTK_DIALOG(dialog), FALSE);
		gtk_container_set_border_width (GTK_CONTAINER(dialog), 5);
		gtk_box_set_spacing (GTK_BOX (GTK_DIALOG(dialog)->vbox), 2);
		gtk_window_set_resizable (GTK_WINDOW(dialog), FALSE);
	}
}

static gint 
score_callback(GtkWidget *widget, gpointer data)
{
        ShowHighScores ();
	return FALSE;
}

/* Define menus later so we don't have to proto callbacks? */
GnomeUIInfo gamemenu[] = {
        GNOMEUIINFO_MENU_NEW_GAME_ITEM (new_game_callback, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_SCORES_ITEM (score_callback, NULL),
	GNOMEUIINFO_SEPARATOR,
        GNOMEUIINFO_MENU_QUIT_ITEM (quit_game, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo settingsmenu[] = {
	GNOMEUIINFO_MENU_PREFERENCES_ITEM (setup_game, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo helpmenu[] = {
        GNOMEUIINFO_HELP ("gtali"),
	GNOMEUIINFO_MENU_ABOUT_ITEM (about, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo mainmenu[] = {
	GNOMEUIINFO_MENU_GAME_TREE (gamemenu),
	GNOMEUIINFO_MENU_SETTINGS_TREE (settingsmenu),
	GNOMEUIINFO_MENU_HELP_TREE (helpmenu),
	GNOMEUIINFO_END
};

static void roll_set_sensitive (gboolean state)
{
        gtk_widget_set_sensitive (GTK_WIDGET (mbutton), state);
}

static void
LoadDicePixmaps(void)
{
	int i, j;

	for (i=0; i < NUMBER_OF_PIXMAPS; i++) {
                /* This is not efficient, but it lets us load animated types,
                 * there is no way for us to copy a general GtkImage (the old 
                 * code had a way for static images). */
		if (g_file_test (dicefiles[i], G_FILE_TEST_EXISTS)) 
			for (j=0; j < NUMBER_OF_DICE; j++)
                                dicePixmaps[j][i] = gtk_image_new_from_file (dicefiles[i]);
                /* FIXME: What happens if the file isn't found. */
	}
}

void update_score_state (void)
{
        gchar **names = NULL;
        gfloat *scores = NULL;
        time_t *scoretimes = NULL;
	gint top;

	top = gnome_score_get_notable (appID, NULL,
                                       &names, &scores, &scoretimes);
	if (top > 0) {
		gtk_widget_set_sensitive (gamemenu[2].widget, TRUE);
		g_strfreev (names);
		g_free (scores);
		g_free (scoretimes);
	} else {
		gtk_widget_set_sensitive (gamemenu[2].widget, FALSE);
	}
}

static void
GyahtzeeCreateMainWindow(void)
{
        GtkWidget *all_boxes;
        GtkWidget *toolbar;
	GtkWidget *tmp;
        GtkWidget *vbox;
	int i, j;

        /*        gtk_window_set_resizable (GTK_WINDOW (window), FALSE);  */
        g_signal_connect (G_OBJECT (window), "delete_event",
                          G_CALLBACK (quit_game), NULL);
        g_signal_connect (G_OBJECT (window), "key_press_event",
                          G_CALLBACK (key_press), NULL);

	/*---- Menus ----*/
	gnome_app_create_menus (GNOME_APP (window), mainmenu);


	/*---- Status Bar ----*/
	appbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_USER);
	gnome_app_set_statusbar (GNOME_APP (window), appbar);
	gnome_appbar_push (GNOME_APPBAR (appbar),
			   _("Select dice to re-roll, press Roll!,"
                             " or select score slot."));

	gnome_app_install_menu_hints (GNOME_APP (window), mainmenu);

	/*---- Content ----*/
        all_boxes = gtk_hbox_new (FALSE, 0);
	gnome_app_set_contents (GNOME_APP (window), all_boxes);

        /* Retreive dice pixmaps from memory or files */
        LoadDicePixmaps ();
        
	/* Put all the dice in a vertical column */
        vbox = gtk_vbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX(all_boxes), vbox, FALSE, TRUE, 0);
        gtk_widget_show (vbox);

        rollLabel = gtk_label_new(NULL);
        gtk_label_set_use_markup(GTK_LABEL(rollLabel), TRUE);
        gtk_widget_show(rollLabel);
        gtk_box_pack_start (GTK_BOX (vbox), rollLabel, FALSE, TRUE, 5);

        mbutton = gtk_button_new_with_label (_("Roll!"));
        gtk_box_pack_end (GTK_BOX (vbox), mbutton, FALSE, FALSE, 5); 
        g_signal_connect (G_OBJECT (mbutton), "clicked",
                          G_CALLBACK (gnome_roll_dice), NULL);
        gtk_widget_show (GTK_WIDGET (mbutton));
        
        toolbar = gtk_toolbar_new ();
        gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), 
                                     GTK_ORIENTATION_VERTICAL);
        gtk_toolbar_set_style (GTK_TOOLBAR (toolbar),
                               GTK_TOOLBAR_ICONS);
        gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), FALSE);
        gtk_box_pack_end (GTK_BOX (vbox), toolbar, TRUE, TRUE, 0); 

 	for (i=0; i < NUMBER_OF_DICE; i++) {
                tmp = gtk_vbox_new(FALSE, 0);
                
                for (j=0; j < NUMBER_OF_PIXMAPS; j++)
                        gtk_box_pack_start (GTK_BOX (tmp), 
                                            dicePixmaps[i][j],
                                            FALSE, FALSE, 0);

                diceBox[i] = gtk_tool_button_new (tmp, NULL);
                g_signal_connect (G_OBJECT (diceBox[i]), "clicked",
                                  G_CALLBACK (gnome_modify_dice),
                                  &DiceValues[i]);

                gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
                                    diceBox[i], -1);

                gtk_widget_show (GTK_WIDGET (diceBox[i])); 
                gtk_widget_show (tmp); 
                gtk_widget_show (dicePixmaps[i][0]);
	}
        gtk_widget_show (toolbar);

	/* Scores displayed in score list */
        ScoreList = create_score_list ();
        gtk_box_pack_end (GTK_BOX (all_boxes), ScoreList, TRUE, TRUE, 0);
        setup_score_list (ScoreList);
        gtk_widget_show (ScoreList);
        
	gtk_widget_show (all_boxes);

        gtk_widget_show (window);

	update_score_state ();

}

int
main (int argc, char *argv[])
{
        GConfClient *client;
        GError *err = NULL;
        GSList *name_list = NULL;
        gint i;

	gnome_score_init (appID);

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* Reset all yahtzee variables before parsing args */
        YahtzeeInit ();

	/* Create gnome client */
        gnome_program_init (appID, VERSION,
                            LIBGNOMEUI_MODULE,
                            argc, argv,
                            GNOME_PARAM_POPT_TABLE, yahtzee_options,
                            GNOME_PARAM_APP_DATADIR, DATADIR, NULL);
        gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-gtali.png");

        client = gconf_client_get_default ();
        NumberOfComputers = gconf_client_get_int (client, "/apps/gtali/NumberOfComputerOpponents", &err);
        if (err) {
                g_warning (G_STRLOC ": gconf error: %s\n", err->message);
                g_error_free (err);
                err = NULL;
        }

	NumberOfHumans = gconf_client_get_int(client, "/apps/gtali/NumberOfHumanOpponents", &err);
        if (err) {
                g_warning (G_STRLOC ": gconf error: %s\n", err->message);
                g_error_free (err);
                err = NULL;
        }

        if (NumberOfHumans < 0)
                NumberOfHumans = 0;
        if (NumberOfComputers < 0)
                NumberOfComputers = 0;
        
        if (NumberOfHumans > MAX_NUMBER_OF_PLAYERS)
                NumberOfHumans = MAX_NUMBER_OF_PLAYERS;
        if ((NumberOfHumans + NumberOfComputers) > MAX_NUMBER_OF_PLAYERS)
                NumberOfComputers = MAX_NUMBER_OF_PLAYERS - NumberOfHumans;

        /* We ignore errors for these, they're boolean so it will be valid 
         * data even if not the right data and there's nothing else we
         * can do (if it's a systematic gconf problem, the calls above
         * would have caught it. */
        DoDelay = gconf_client_get_bool (client,
                                         "/apps/gtali/DelayBetweenRolls",
                                         NULL);
        DisplayComputerThoughts = gconf_client_get_bool (client,
                                                         "/apps/gtali/DisplayComputerThoughts",
                                                         NULL);
        /* Read in new player names */
        name_list = gconf_client_get_list (client, "/apps/gtali/PlayerNames",
                                           GCONF_VALUE_STRING, &err);

        if (err) {
                g_warning (G_STRLOC ": gconf error: %s\n", err->message);
                g_error_free (err);
                err = NULL;
        }

        for (i = 0; i < MAX_NUMBER_OF_PLAYERS && name_list; i++) {
                if (name_list->data)
                        players[i].name = g_strdup (name_list->data);
                
                name_list = g_slist_next (name_list);
        }
        g_slist_free (name_list);

	/* For some options, we don't want to play a game */
	if (! GyahtzeeAbort) {

                window = gnome_app_new (appID, appName);
	  
                GyahtzeeCreateMainWindow ();

                /* Need to roll the dice once */
                GyahtzeeNewGame ();
        
                gtk_main ();

	}
        
	return 0;
}

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
End:
*/ 
