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

#include <games-stock.h>

#include "yahtzee.h"
#include "gyahtzee.h"

#define DELAY_MS 600

static char *appID="gtali";
static char *appName=N_("Tali");
static guint lastHighScore = 0;
static guint last_timeout = 0;
static gboolean ready_to_advance_player;

#ifdef GNOMEPIXMAPDIR
#define PP GNOMEPIXMAPDIR "/gtali/" 
#else
#define PP ""
#endif

#define NUMBER_OF_PIXMAPS    7
#define DIE_SELECTED_PIXMAP  (NUMBER_OF_PIXMAPS-1) 	 
 
static char *dicefiles[NUMBER_OF_PIXMAPS] = { PP "gnome-dice-1.svg",
                                              PP "gnome-dice-2.svg",
                                              PP "gnome-dice-3.svg",
                                              PP "gnome-dice-4.svg",
                                              PP "gnome-dice-5.svg",
                                              PP "gnome-dice-6.svg",
                                              PP "gnome-dice-none.svg"};

static GtkWidget *dicePixmaps[NUMBER_OF_DICE][NUMBER_OF_PIXMAPS];

GtkWidget *window;
GtkWidget *ScoreList;
static GtkWidget *statusbar;
static GtkToolItem *diceBox[NUMBER_OF_DICE];
static GtkWidget *rollLabel;
static GtkWidget *mbutton;
static GtkAction *scores_action;

const static GOptionEntry yahtzee_options[] = {
  {"delay", 'd', 0, G_OPTION_ARG_NONE, &DoDelay, 
   N_("Delay computer moves"), NULL},
  {"thoughts", 't', 0, G_OPTION_ARG_NONE, &DisplayComputerThoughts, 
   N_("Display computer thoughts"), NULL},
  {"computers", 'n', 0, G_OPTION_ARG_INT, &NumberOfComputers, 
   N_("Number of computer opponents"), N_("NUMBER")},
  {"humans", 'p', 0, G_OPTION_ARG_INT, &NumberOfHumans, 
   N_("Number of human opponents"), N_("NUMBER")},
  { NULL }
};

static gint gnome_modify_dice (GtkWidget *widget,
                               gpointer data);
static gint gnome_roll_dice (GtkWidget *widget, GdkEvent *event,
                             gpointer data);
void update_score_state (void);
static void UpdateRollLabel (void);

static void
update_roll_button_sensitivity (void)
{
        gboolean state = FALSE;
        gint i;

        for (i=0; i < NUMBER_OF_DICE; i++)
                state |= gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (diceBox[i]));

        state &= NumberOfRolls < 3;
        state &= !players[CurrentPlayer].comp;
        
        gtk_widget_set_sensitive (GTK_WIDGET (mbutton), state);
}

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
quit_game (GObject *object, gpointer data)
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
        
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(diceBox[offset]), !DiceValues[offset].sel);

        return FALSE;
}

static void 
GyahtzeeNewGame (void)
{
        int i;

        say (_("Select dice to roll or choose a score slot."));

        NewGame();
        setup_score_list (ScoreList);
        UpdateRollLabel();

	for (i=0; i<NumberOfPlayers; i++)
                ShowoffPlayer (ScoreList,i,0);
	ShowoffPlayer (ScoreList,0,1);
}


static gint 
new_game_callback (GtkAction *action, gpointer data)
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
	
        update_roll_button_sensitivity();
}

static void 
UpdateDiePixmap (int n)
{
        static int last_val[NUMBER_OF_DICE] = { 0 };

        gtk_widget_hide (dicePixmaps[n][last_val[n]]);

        last_val[n] = DiceValues[n].sel ? DIE_SELECTED_PIXMAP : 
                                          DiceValues[n].val - 1;
        gtk_widget_show (dicePixmaps[n][last_val[n]]);
}

void
UpdateAllDicePixmaps (void)
{
        int i;
        
        for (i = 0; i < NUMBER_OF_DICE; i++) {
                UpdateDiePixmap (i);
        }
}

void
DeselectAllDice(void)
{
	int i;

	for (i = 0; i < NUMBER_OF_DICE; i++)
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (diceBox[i]), FALSE);
}

/* Callback on dice press */
gint 
gnome_modify_dice (GtkWidget *widget, gpointer data) 
{
        DiceInfo *tmp = (DiceInfo *) data;
	GtkToggleToolButton *button = GTK_TOGGLE_TOOL_BUTTON(widget);
 
        /* Don't modify dice if player is marked finished or computer is playing */
	if (players[CurrentPlayer].finished || players[CurrentPlayer].comp) {
		if (gtk_toggle_tool_button_get_active (button))
			gtk_toggle_tool_button_set_active (button, FALSE);
                return TRUE;
	}

        if (NumberOfRolls >= NUM_ROLLS) {
                say(_("You are only allowed three rolls. Choose a score slot."));
		gtk_toggle_tool_button_set_active (button, FALSE);
                return TRUE;
        }

        tmp->sel = gtk_toggle_tool_button_get_active (button);

        UpdateAllDicePixmaps ();

	update_roll_button_sensitivity();
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
	guint context_id;

	va_start (ap, fmt);
	g_vsnprintf (buf, 200, fmt, ap);
	va_end (ap);

	context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "message");
	gtk_statusbar_pop (GTK_STATUSBAR (statusbar), context_id);
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), context_id, buf);
}


static gint
about_cb (GtkAction *action, gpointer data)
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
score_callback(GtkAction *action, gpointer data)
{
        ShowHighScores ();
	return FALSE;
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
		gtk_action_set_sensitive (scores_action, TRUE);
		g_strfreev (names);
		g_free (scores);
		g_free (scoretimes);
	} else {
		gtk_action_set_sensitive (scores_action, FALSE);
	}
}

static void
help_cb (GtkAction *action, gpointer data)
{
        gnome_help_display ("gtali.xml", NULL, NULL);
}


static const GtkActionEntry action_entry[] = {
	{ "GameMenu", NULL, N_("_Game") },
	{ "SettingsMenu", NULL, N_("_Settings") },
	{ "HelpMenu", NULL, N_("_Help") },
	{ "NewGame", GAMES_STOCK_NEW_GAME, NULL, NULL, NULL, G_CALLBACK (new_game_callback) },
	{ "Scores", GAMES_STOCK_SCORES, NULL, NULL, NULL, G_CALLBACK (score_callback) },
	{ "Quit", GTK_STOCK_QUIT, NULL, NULL, NULL, G_CALLBACK (quit_game) },
	{ "Preferences", GTK_STOCK_PREFERENCES, NULL, NULL, NULL, G_CALLBACK (setup_game) },
	{ "Contents", GAMES_STOCK_CONTENTS, NULL, NULL, NULL, G_CALLBACK (help_cb) },
	{ "About", GTK_STOCK_ABOUT, NULL, NULL, NULL, G_CALLBACK (about_cb) },
	/* Roll is just an accelerator */
	{ "Roll", GTK_STOCK_REFRESH, NULL, "r", NULL, G_CALLBACK (gnome_roll_dice) }
};


static const char ui_description[] =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='GameMenu'>"
"      <menuitem action='NewGame'/>"
"      <menuitem action='Scores'/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='SettingsMenu'>"
"      <menuitem action='Preferences'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Contents'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"  <accelerator action='Roll' />"
"</ui>";


static void
create_menus (GtkUIManager *ui_manager)
{
	GtkActionGroup *action_group;

	action_group = gtk_action_group_new ("group");

	gtk_action_group_set_translation_domain(action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, action_entry, G_N_ELEMENTS (action_entry), window);

	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, NULL);
	scores_action = gtk_action_group_get_action (action_group, "Scores");
}


static void
GyahtzeeCreateMainWindow(void)
{
        GtkWidget *hbox, *vbox;
        GtkWidget *toolbar;
	GtkWidget *tmp;
        GtkWidget *dicebox;
	GtkWidget *menubar;
	GtkAccelGroup *accel_group;
	GtkUIManager *ui_manager;
	int i, j;

	games_stock_init();
        
	g_signal_connect (G_OBJECT (window), "delete_event",
                          G_CALLBACK (quit_game), NULL);
        g_signal_connect (G_OBJECT (window), "key_press_event",
                          G_CALLBACK (key_press), NULL);

	statusbar = gtk_statusbar_new ();
	ui_manager = gtk_ui_manager_new ();

	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), FALSE);
	games_stock_prepare_for_statusbar_tooltips (ui_manager, statusbar);

	/*---- Menus ----*/
	create_menus (ui_manager);
	accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	menubar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	
	/*---- Content ----*/

	hbox = gtk_hbox_new (FALSE, 0);
	vbox = gtk_vbox_new (FALSE, 0);

	gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), statusbar, FALSE, FALSE, 0);

	gnome_app_set_contents (GNOME_APP (window), vbox);

	gtk_widget_show (statusbar);
        /* Retreive dice pixmaps from memory or files */
        LoadDicePixmaps ();
        
	/* Put all the dice in a vertical column */
        dicebox = gtk_vbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX(hbox), dicebox, FALSE, TRUE, 0);
        gtk_widget_show (dicebox);

        rollLabel = gtk_label_new(NULL);
        gtk_label_set_use_markup(GTK_LABEL(rollLabel), TRUE);
        gtk_widget_show(rollLabel);
        gtk_box_pack_start (GTK_BOX (dicebox), rollLabel, FALSE, TRUE, 5);

        mbutton = gtk_button_new_with_label (_("Roll!"));
        gtk_box_pack_end (GTK_BOX (dicebox), mbutton, FALSE, FALSE, 5); 
        g_signal_connect (G_OBJECT (mbutton), "clicked",
                          G_CALLBACK (gnome_roll_dice), NULL);
        gtk_widget_show (GTK_WIDGET (mbutton));
        
        toolbar = gtk_toolbar_new ();
        gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), 
                                     GTK_ORIENTATION_VERTICAL);
        gtk_toolbar_set_style (GTK_TOOLBAR (toolbar),
                               GTK_TOOLBAR_ICONS);
        gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), FALSE);
        gtk_box_pack_end (GTK_BOX (dicebox), toolbar, TRUE, TRUE, 0); 

 	for (i=0; i < NUMBER_OF_DICE; i++) {
                tmp = gtk_vbox_new(FALSE, 0);
                
                for (j=0; j < NUMBER_OF_PIXMAPS; j++)
                        gtk_box_pack_start (GTK_BOX (tmp), 
                                            dicePixmaps[i][j],
                                            FALSE, FALSE, 0);

                diceBox[i] = gtk_toggle_tool_button_new ();
                gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(diceBox[i]), tmp);
                g_signal_connect (G_OBJECT (diceBox[i]), "clicked",
                                  G_CALLBACK (gnome_modify_dice),
                                  &DiceValues[i]);

                gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
                                    GTK_TOOL_ITEM (diceBox[i]), -1);

                gtk_widget_show (GTK_WIDGET (diceBox[i])); 
                gtk_widget_show (tmp); 
                gtk_widget_show (dicePixmaps[i][0]);
	}
        gtk_widget_show (toolbar);

	/* Scores displayed in score list */
        ScoreList = create_score_list ();
        gtk_box_pack_end (GTK_BOX (hbox), ScoreList, TRUE, TRUE, 0);
        setup_score_list (ScoreList);
        gtk_widget_show (ScoreList);
        
	gtk_widget_show (hbox);

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
        GOptionContext *context;

	gnome_score_init (appID);
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* Reset all yahtzee variables before parsing args */
        YahtzeeInit ();

        context = g_option_context_new ("");
        g_option_context_add_main_entries (context, yahtzee_options, 
                                           GETTEXT_PACKAGE);

	/* Create gnome client */
        gnome_program_init (appID, VERSION,
                            LIBGNOMEUI_MODULE,
                            argc, argv,
                            GNOME_PARAM_GOPTION_CONTEXT, context,
                            GNOME_PARAM_APP_DATADIR, DATADIR, NULL);

        gtk_window_set_default_icon_from_file (GNOME_ICONDIR"/gnome-gtali.png",
                                               NULL);

        client = gconf_client_get_default ();
        if (NumberOfComputers == 0) /* Not set on the command-line. */
                NumberOfComputers = gconf_client_get_int (client, 
                                                          "/apps/gtali/NumberOfComputerOpponents", 
                                                          NULL);

        if (NumberOfHumans == 0) /* Not set on the command-line. */
                NumberOfHumans = gconf_client_get_int(client, 
                                                      "/apps/gtali/NumberOfHumanOpponents", 
                                                      NULL);

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
        if (DoDelay == 0) /* Not set on the command-line */
                DoDelay = gconf_client_get_bool (client,
                                                 "/apps/gtali/DelayBetweenRolls",
                                                 NULL);
        if (DisplayComputerThoughts == 0) /* Not set on the command-line */
        DisplayComputerThoughts = gconf_client_get_bool (client,
                                                         "/apps/gtali/DisplayComputerThoughts",
                                                         NULL);
        /* Read in new player names */
        name_list = gconf_client_get_list (client, "/apps/gtali/PlayerNames",
                                           GCONF_VALUE_STRING, NULL);

        for (i = 0; i < MAX_NUMBER_OF_PLAYERS && name_list; i++) {
                if (name_list->data)
                        players[i].name = g_strdup (name_list->data);
                
                name_list = g_slist_next (name_list);
        }
        g_slist_free (name_list);

        window = gnome_app_new (appID, appName);
	  
        GyahtzeeCreateMainWindow ();

        /* Need to roll the dice once */
        GyahtzeeNewGame ();
                
        gtk_main ();
                
	gnome_accelerators_sync();
        
	return 0;
}

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
End:
*/ 
