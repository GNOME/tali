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
#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <libgnome-games-support.h>

#include "yahtzee.h"
#include "gyahtzee.h"

#define DELAY_MS 600

static char *appName = N_("Tali");
static GtkApplication *application;
static guint last_timeout = 0;
static gboolean ready_to_advance_player;

#define NUMBER_OF_PIXMAPS    7
#define GAME_TYPES           2
#define DIE_SELECTED_PIXMAP  (NUMBER_OF_PIXMAPS-1)
#define SCORES_CATEGORY      (game_type == GAME_KISMET ? "Colors" : NULL)

static char *dicefiles[NUMBER_OF_PIXMAPS] = { "gnome-dice-1.svg",
  "gnome-dice-2.svg",
  "gnome-dice-3.svg",
  "gnome-dice-4.svg",
  "gnome-dice-5.svg",
  "gnome-dice-6.svg",
  "gnome-dice-none.svg"
};

static char *kdicefiles[NUMBER_OF_PIXMAPS] = { "kismet1.svg",
  "kismet2.svg",
  "kismet3.svg",
  "kismet4.svg",
  "kismet5.svg",
  "kismet6.svg",
  "kismet-none.svg"
};

static GtkWidget *dicePixmaps[NUMBER_OF_DICE][NUMBER_OF_PIXMAPS][GAME_TYPES];

GSettings *settings;
GtkWidget *window;
GtkWidget *ScoreList;
static GtkToolItem *diceBox[NUMBER_OF_DICE];
static GtkWidget *rollLabel;
static GtkWidget *mbutton;
static GtkWidget *hbar;
static GAction *scores_action;
static GAction *undo_action;
static gchar *game_type_string = NULL;
static gint   test_computer_play = 0;
gint NUM_TRIALS = 0;

static const GOptionEntry yahtzee_options[] = {
  {"delay", 'd', 0, G_OPTION_ARG_NONE, &DoDelay,
   N_("Delay computer moves"), NULL},
  {"thoughts", 't', 0, G_OPTION_ARG_NONE, &DisplayComputerThoughts,
   N_("Display computer thoughts"), NULL},
  {"computers", 'n', 0, G_OPTION_ARG_INT, &NumberOfComputers,
   N_("Number of computer opponents"), N_("NUMBER")},
  {"humans", 'p', 0, G_OPTION_ARG_INT, &NumberOfHumans,
   N_("Number of human opponents"), N_("NUMBER")},
  {"game", 'g', 0, G_OPTION_ARG_STRING, &game_type_string,
   N_("Game choice: Regular or Colors"), N_("STRING")},
  {"computer-test", 'c', 0, G_OPTION_ARG_INT, &test_computer_play,
   N_("Number of computer-only games to play"), N_("NUMBER")},
  {"monte-carlo-trials", 'm', 0, G_OPTION_ARG_INT, &NUM_TRIALS,
   N_("Number of trials for each roll for the computer"), N_("NUMBER")},
  {NULL}
};

typedef struct
{
  gchar *key;
  gchar *name;
} key_value;

static const key_value category_array[] = {
  /* Order must match GameType enum order */
  {"Regular", NC_("game type", "Regular")},
  {"Colors",  NC_("game type", "Colors")}
};

const gchar *
category_name_from_key (const gchar *key)
{
  int i;
  for (i = 0; i < G_N_ELEMENTS (category_array); ++i) {
    if (g_strcmp0 (category_array[i].key, key) == 0)
      return g_dpgettext2(NULL, "game type", category_array[i].name);
  }
  return NULL;
}

static GamesScoresCategory *
create_category_from_key (const gchar *key,
                          gpointer     user_data)
{
  const gchar *name = category_name_from_key (key);
  if (name == NULL)
    return NULL;
  return games_scores_category_new (key, name);
}

GamesScoresContext *highscores;

static void modify_dice (GtkToolButton * widget, gpointer data);
static void UpdateRollLabel (void);

static void
update_roll_button_sensitivity (void)
{
  gboolean state = FALSE;
  gint i;

  for (i = 0; i < NUMBER_OF_DICE; i++)
    state |=
      gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (diceBox[i]));

  if (!state) {
    gtk_button_set_label (GTK_BUTTON (mbutton), _("Roll all!"));
    state = TRUE;
  } else {
    gtk_button_set_label (GTK_BUTTON (mbutton), _("Roll!"));
    state = TRUE;
  }

  state &= NumberOfRolls < 3;
  state &= !players[CurrentPlayer].comp;

  if (GameIsOver ()) {
    state = FALSE;
  }

  gtk_widget_set_sensitive (GTK_WIDGET (mbutton), state);
}

static void
add_score_cb (GObject      *source_object,
              GAsyncResult *res,
              gpointer      user_data)
{
  GamesScoresContext *context = GAMES_SCORES_CONTEXT (source_object);
  GError *error = NULL;

  games_scores_context_add_score_finish (context, res, &error);
  if (error != NULL) {
    g_warning ("Failed to add score: %s", error->message);
    g_error_free (error);
  }
}

static void
CheerWinner (void)
{
  int winner;
  int i;

  ShowoffPlayer (ScoreList, CurrentPlayer, 0);

  winner = FindWinner ();

  /* draw. The score is returned as a negative value */
  if (winner < 0) {
    for (i = 0; i < NumberOfPlayers; i++) {
      if (total_score (i) == -winner) {
	ShowoffPlayer (ScoreList, i, 1);
      }
    }

    say (_("The game is a draw!"));
    return;
  }

  ShowoffPlayer (ScoreList, winner, 1);

  if (winner < NumberOfHumans) {
    GamesScoresCategory *category;
    category = create_category_from_key (category_array[game_type].key, NULL);
    games_scores_context_add_score (highscores, (guint32) WinningScore,
                                    category, NULL,
                                    add_score_cb, NULL);
  }

  if (players[winner].name)
    say (ngettext ("%s wins the game with %d point",
		   "%s wins the game with %d points", WinningScore),
	 players[winner].name, WinningScore);
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

  if (ready_to_advance_player) {
    NextPlayer ();
    return TRUE;
  }

  if (players[CurrentPlayer].finished) {
    NextPlayer ();
    return TRUE;
  }

  ComputerRolling (CurrentPlayer);
  if (NoDiceSelected () || (NumberOfRolls >= NUM_ROLLS)) {
    ComputerScoring (CurrentPlayer);
    ready_to_advance_player = TRUE;
  } else {
    RollSelectedDice ();
    UpdateRollLabel ();
  }

  if (!DoDelay)
    do_computer_turns ();

  return TRUE;
}

/* Show the current score and prompt for current player state */

void
DisplayCurrentPlayer (void) {
  ShowoffPlayer (ScoreList, CurrentPlayer, 1);

  if (players[CurrentPlayer].name) {
    if (players[CurrentPlayer].comp) {
      say (_("Computer playing for %s"), players[CurrentPlayer].name);
    } else {
      say (_("%s! – You’re up."), players[CurrentPlayer].name);
    }
  }
}

/* Display current player and refresh dice/display */
void
DisplayCurrentPlayerRefreshDice (void) {
  DisplayCurrentPlayer ();
  UpdateAllDicePixmaps ();
  DeselectAllDice ();
  UpdateRollLabel ();
}

void
NextPlayer (void)
{
  if (GameIsOver ()) {
    if (last_timeout) {
      g_source_remove (last_timeout);
      last_timeout = 0;
    }
    if (DoDelay && NumberOfComputers > 0)
        NumberOfRolls = NUM_ROLLS;
    else
        NumberOfRolls = LastHumanNumberOfRolls;
    /* update_roll_button_sensitivity () needs to be called in
       this context however UpdateRollLabel () also calls that method */
    UpdateRollLabel ();
    CheerWinner ();
    return;
  }

  NumberOfRolls = 0;
  ready_to_advance_player = FALSE;
  ShowoffPlayer (ScoreList, CurrentPlayer, 0);

  /* Find the next player with rolls left */
  do {
    CurrentPlayer = (CurrentPlayer + 1) % NumberOfPlayers;
  } while (players[CurrentPlayer].finished);

  DisplayCurrentPlayer ();
  SelectAllDice ();
  RollSelectedDice ();
  FreeRedoList ();

  /* Remember the roll count if this turn is for a
     human player for display at the end of the game */
  if (!players[CurrentPlayer].comp)
    LastHumanNumberOfRolls = NumberOfRolls;

  if (players[CurrentPlayer].comp) {
    if (DoDelay) {
      if (!last_timeout)
	last_timeout = g_timeout_add (DELAY_MS,
				      (GSourceFunc) do_computer_turns, NULL);
    } else {
      do_computer_turns ();
    }
  }
  /* Only update the roll label if we are running in
    delay mode or if this turn is for a human player */
  if (DoDelay || (!players[CurrentPlayer].comp))
    UpdateRollLabel ();
}

/* Go back to the previous player */

void
PreviousPlayer (void)
{
  if (UndoPossible ()) {
    NumberOfRolls = 1;
    ready_to_advance_player = FALSE;
    ShowoffPlayer (ScoreList, CurrentPlayer, 0);

    /* Find the next player with rolls left */
    do {
      CurrentPlayer = (UndoLastMove () + NumberOfPlayers) % NumberOfPlayers;
    } while (players[CurrentPlayer].comp && UndoPossible ());

    DisplayCurrentPlayerRefreshDice ();
  }
}

void
RedoPlayer (void)
{
  if (RedoPossible ()) {
    NumberOfRolls = 1;
    ready_to_advance_player = FALSE;
    ShowoffPlayer (ScoreList, CurrentPlayer, 0);
    /* The first element of the list is the undone turn, so
     * we need to remove it from the list before redoing other
     * turns.                                                    */
    FreeRedoListHead ();

    /* Redo all computer players */
    do {
      CurrentPlayer = RedoLastMove ();
    } while (players[CurrentPlayer].comp && RedoPossible ());

    RestoreLastRoll ();
    DisplayCurrentPlayerRefreshDice ();
  }
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

      if (test_computer_play == 0) update_score_cell (ScoreList, line, num + 1, score);
    }
  }

  upper_tot = upper_total (num);
  lower_tot = lower_total (num);

  if (upper_tot >= 63) {
    bonus = 35;
    if (game_type == GAME_KISMET) {
      if (upper_tot >= 78)
        bonus = 75;
      else if (upper_tot >= 71)
        bonus = 55;
    }
    upper_tot += bonus;
  }

  if (test_computer_play == 0) {
      update_score_cell (ScoreList, R_BONUS, num + 1, bonus);
      update_score_cell (ScoreList, R_UTOTAL, num + 1, upper_tot);
      update_score_cell (ScoreList, R_LTOTAL, num + 1, lower_tot);
      update_score_cell (ScoreList, R_GTOTAL, num + 1, upper_tot + lower_tot);
  }
}

static void
quit_cb (GSimpleAction * action, GVariant * parameter, gpointer data)
{
  gtk_widget_destroy (window);
}

static void
preferences_cb (GSimpleAction * action, GVariant * parameter, gpointer data)
{
  setup_game ();
}

/* This handles the keys 1..5 for the dice. */
static gint
key_press (GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  gint offset;

  offset = event->keyval - GDK_KEY_1;

  if ((offset < 0) || (offset >= NUMBER_OF_DICE)) {
    return FALSE;
  }

  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (diceBox[offset]),
				     !DiceValues[offset].sel);

  return FALSE;
}

static void
GyahtzeeNewGame (void)
{
  int i;

  say (_("Select dice to roll or choose a score slot."));

  game_type = get_new_game_type ();
  NewGame ();
  setup_score_list (ScoreList);
  UpdateRollLabel ();

  for (i = 0; i < NumberOfPlayers; i++)
    ShowoffPlayer (ScoreList, i, 0);
  ShowoffPlayer (ScoreList, 0, 1);
}


static void
new_game_cb (GSimpleAction * action, GVariant * parameter, gpointer data)
{
  GyahtzeeNewGame ();
}

static void
UpdateRollLabel (void)
{
  static GString *str = NULL;

  if (!str)
    str = g_string_sized_new (22);

  g_string_printf (str, "<b>%s %d/3</b>", _("Roll"), NumberOfRolls);
  gtk_label_set_label (GTK_LABEL (rollLabel), str->str);

  update_score_tooltips ();
  update_roll_button_sensitivity ();
  update_undo_sensitivity ();
}

static void
UpdateDiePixmap (int n, int prev_game_type)
{
  static int last_val[NUMBER_OF_DICE] = { 0 };

  gtk_widget_hide (dicePixmaps[n][last_val[n]][prev_game_type]);

  last_val[n] = DiceValues[n].sel ? DIE_SELECTED_PIXMAP :
    DiceValues[n].val - 1;
  gtk_widget_show (dicePixmaps[n][last_val[n]][game_type]);
}

void
UpdateAllDicePixmaps (void)
{
  int i;
  static int prev_game_type = 0;

  for (i = 0; test_computer_play == 0 && i < NUMBER_OF_DICE; i++) {
    UpdateDiePixmap (i, prev_game_type);
  }
  prev_game_type = game_type;
}

void
DeselectAllDice (void)
{
  int i;

  if (test_computer_play > 0) return;
  for (i = 0; i < NUMBER_OF_DICE; i++)
    gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (diceBox[i]),
				       FALSE);
}

/* Callback on dice press */
static void
modify_dice (GtkToolButton * widget, gpointer data)
{
  DiceInfo *tmp = (DiceInfo *) data;
  GtkToggleToolButton *button = GTK_TOGGLE_TOOL_BUTTON (widget);

  /* Don't modify dice if player is marked finished or computer is playing */
  if (players[CurrentPlayer].finished || players[CurrentPlayer].comp) {
    if (gtk_toggle_tool_button_get_active (button))
      gtk_toggle_tool_button_set_active (button, FALSE);
    return;
  }

  if (NumberOfRolls >= NUM_ROLLS) {
    say (_("You are only allowed three rolls. Choose a score slot."));
    gtk_toggle_tool_button_set_active (button, FALSE);
    return;
  }

  tmp->sel = gtk_toggle_tool_button_get_active (button);

  UpdateAllDicePixmaps ();

  update_roll_button_sensitivity ();
  return;
}

static void
roll (void)
{
  if (!players[CurrentPlayer].comp) {
    RollSelectedDice ();
    if (NumberOfRolls > 1)
        FreeUndoRedoLists ();
    UpdateRollLabel ();
    LastHumanNumberOfRolls = NumberOfRolls;
  }
}

static void
roll_button_pressed_cb (GtkButton * button, gpointer data)
{
  roll ();
}

static void
roll_cb (GSimpleAction * action, GVariant * parameter, gpointer data)
{
  roll ();
}

void
say (char *fmt, ...)
{
  va_list ap;
  char buf[200];

  if (test_computer_play > 0) return;
  va_start (ap, fmt);
  g_vsnprintf (buf, 200, fmt, ap);
  va_end (ap);

  gtk_header_bar_set_subtitle (GTK_HEADER_BAR (hbar), buf);

}


static void
about_cb (GSimpleAction * action, GVariant * parameter, gpointer data)
{
  const gchar *authors[] = {
    N_("GNOME version (1998):"),
    "Scott Heavner",
    "",
    N_("Console version (1992):"),
    "Orest Zborowski",
    "",
    N_("Colors game and multi-level AI (2006):"),
    "Geoff Buchan",
    NULL
  };

  const gchar *documenters[] = {
    "Scott D Heavner",
    "Callum McKenzie",
    NULL
  };

  gtk_show_about_dialog (GTK_WINDOW (window),
			 "name", appName,
			 "version", VERSION,
			 "copyright", "Copyright © 1998–2008 "
			 "Free Software Foundation, Inc.",
			 "license-type", GTK_LICENSE_GPL_2_0,
			 "comments", _("A variation on poker with "
				       "dice and less money."),
			 "authors", authors,
			 "documenters", documenters,
			 "translator-credits", _("translator-credits"),
			 "logo-icon-name", "org.gnome.Tali",
			 "website",
			 "https://wiki.gnome.org/Apps/Tali",
			 NULL);
}

void
ShowHighScores (void)
{
  games_scores_context_run_dialog (highscores);
}

static void
score_cb (GSimpleAction * action, GVariant * parameter, gpointer data)
{
  ShowHighScores ();
}

static void
undo_cb (GSimpleAction * action, GVariant * parameter, gpointer data)
{
  PreviousPlayer ();
}

static void
LoadDicePixmaps (void)
{
  int i, j;
  char *path, *path_kismet;
  GError *error = NULL;

  for (i = 0; i < NUMBER_OF_PIXMAPS; i++) {
    /* This is not efficient, but it lets us load animated types,
     * there is no way for us to copy a general GtkImage (the old 
     * code had a way for static images). */
    path = g_build_filename (DATA_DIRECTORY, dicefiles[i], NULL);
    path_kismet = g_build_filename (DATA_DIRECTORY, kdicefiles[i], NULL);

    if (g_file_test (path, G_FILE_TEST_EXISTS) && 
          g_file_test (path_kismet, G_FILE_TEST_EXISTS)) {

      for (j = 0; j < NUMBER_OF_DICE; j++) {
        GdkPixbuf *pixbuf;

        pixbuf = gdk_pixbuf_new_from_file_at_size (path, 60, 60, &error);
        dicePixmaps[j][i][GAME_YAHTZEE] = gtk_image_new_from_pixbuf (pixbuf);
        if (error) {
          g_warning ("Loading dice image %s: %s", path, error->message);
          g_clear_error (&error);
        }
        g_object_unref (pixbuf);

        pixbuf = gdk_pixbuf_new_from_file_at_size (path_kismet, 60, 60, &error);
        dicePixmaps[j][i][GAME_KISMET] = gtk_image_new_from_pixbuf (pixbuf);
        if (error) {
          g_warning ("Loading dice image %s: %s", path_kismet, error->message);
          g_clear_error (&error);
        }
        g_object_unref (pixbuf);
      }

    } /* FIXME: What happens if the file isn't found. */
    g_free (path);
    g_free (path_kismet);
  }
}

void
update_undo_sensitivity (void)
{
  g_simple_action_set_enabled (((GSimpleAction *) undo_action), UndoVisible ());
}

static void
help_cb (GSimpleAction * action, GVariant * parameter, gpointer data)
{
  GError *error = NULL;

  gtk_show_uri_on_window (GTK_WINDOW (window), "help:tali", gtk_get_current_event_time (), &error);
  if (error)
    g_warning ("Failed to show help: %s", error->message);
  g_clear_error (&error);
}

static const GActionEntry app_entries[] = {
  {"new-game", new_game_cb, NULL, NULL, NULL},
  {"undo", undo_cb, NULL, NULL, NULL},
  {"scores", score_cb, NULL, NULL, NULL},
  {"quit", quit_cb, NULL, NULL, NULL},
  {"preferences", preferences_cb, NULL, NULL, NULL},
  {"help", help_cb, NULL, NULL, NULL},
  {"about", about_cb, NULL, NULL, NULL},
  {"roll", roll_cb, NULL, NULL, NULL}
};


static void
GyahtzeeCreateMainWindow (GApplication *app, gpointer user_data)
{
  GtkWidget *hbox, *vbox;
  GtkWidget *toolbar;
  GtkWidget *tmp;
  GtkWidget *dicebox;
  GtkWidget *undo_button;
  GtkWidget *menu_button;
  GtkWidget *icon;
  GtkBuilder *builder;
  GMenuModel *appmenu;
  int i, j;

  window = gtk_application_window_new (application);
  gtk_window_set_application (GTK_WINDOW (window), application);
  gtk_window_set_title (GTK_WINDOW (window), _(appName));
  gtk_window_set_hide_titlebar_when_maximized (GTK_WINDOW (window), FALSE);
  gtk_window_set_icon_name (GTK_WINDOW (window), "org.gnome.Tali");

  //games_conf_add_window (GTK_WINDOW (window), NULL);

  g_signal_connect (GTK_WIDGET (window), "key_press_event",
		    G_CALLBACK (key_press), NULL);

  g_action_map_add_action_entries (G_ACTION_MAP (application), app_entries, G_N_ELEMENTS (app_entries), application);
  const gchar *vaccels_help[] = {"F1", NULL};
  const gchar *vaccels_new[] = {"<Primary>n", NULL};
  const gchar *vaccels_roll[] = {"<Primary>r", NULL};
  const gchar *vaccels_undo[] = {"<Primary>z", NULL};
  const gchar *vaccels_quit[] = {"<Primary>q", NULL};

  gtk_application_set_accels_for_action (application, "app.help", vaccels_help);
  gtk_application_set_accels_for_action (application, "app.new-game", vaccels_new);
  gtk_application_set_accels_for_action (application, "app.roll", vaccels_roll);
  gtk_application_set_accels_for_action (application, "app.undo", vaccels_undo);
  gtk_application_set_accels_for_action (application, "app.quit", vaccels_quit);

  scores_action = g_action_map_lookup_action (G_ACTION_MAP (application), "scores");
  undo_action   = g_action_map_lookup_action (G_ACTION_MAP (application), "undo");
  update_undo_sensitivity ();

        /*--- Headerbar ---*/
  hbar = gtk_header_bar_new ();
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (hbar), TRUE);
  gtk_header_bar_set_title (GTK_HEADER_BAR (hbar), _(appName));
  gtk_widget_show (hbar);
  gtk_window_set_titlebar (GTK_WINDOW (window), hbar);

  if (gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL)
    undo_button = gtk_button_new_from_icon_name ("edit-undo-rtl-symbolic", GTK_ICON_SIZE_BUTTON);
  else
    undo_button = gtk_button_new_from_icon_name ("edit-undo-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_valign (undo_button, GTK_ALIGN_CENTER);
  gtk_actionable_set_action_name (GTK_ACTIONABLE (undo_button), "app.undo");
  gtk_widget_set_tooltip_text (undo_button, _("Undo your most recent move"));
  gtk_widget_show (undo_button);
  gtk_header_bar_pack_start (GTK_HEADER_BAR (hbar), undo_button);

  builder = gtk_builder_new_from_resource ("/org/gnome/Tali/ui/menus.ui");
  appmenu = (GMenuModel *) gtk_builder_get_object (builder, "app-menu");

  menu_button = gtk_menu_button_new();
  icon = gtk_image_new_from_icon_name ("open-menu-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_button_set_image (GTK_BUTTON (menu_button), icon);
  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (menu_button), appmenu);
  gtk_widget_show (menu_button);
  gtk_header_bar_pack_end (GTK_HEADER_BAR (hbar), menu_button);

	/*---- Content ----*/

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  gtk_container_add (GTK_CONTAINER (window), vbox);

  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  /* Retreive dice pixmaps from memory or files */
  LoadDicePixmaps ();

  /* Put all the dice in a vertical column */
  dicebox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox), dicebox, FALSE, TRUE, 0);
  gtk_widget_show (dicebox);

  rollLabel = gtk_label_new (NULL);
  gtk_label_set_use_markup (GTK_LABEL (rollLabel), TRUE);
  gtk_widget_show (rollLabel);
  gtk_box_pack_start (GTK_BOX (dicebox), rollLabel, FALSE, TRUE, 5);

  mbutton = gtk_button_new_with_label (_("Roll!"));
  gtk_box_pack_end (GTK_BOX (dicebox), mbutton, FALSE, FALSE, 5);
  g_signal_connect (GTK_BUTTON (mbutton), "clicked",
		    G_CALLBACK (roll_button_pressed_cb), NULL);
  gtk_widget_show (GTK_WIDGET (mbutton));

  toolbar = gtk_toolbar_new ();
  gtk_orientable_set_orientation (GTK_ORIENTABLE (toolbar),
			       GTK_ORIENTATION_VERTICAL);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), FALSE);
  gtk_box_pack_end (GTK_BOX (dicebox), toolbar, TRUE, TRUE, 0);

  for (i = 0; i < NUMBER_OF_DICE; i++) {
    tmp = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    for (j = 0; j < NUMBER_OF_PIXMAPS; j++) {
      gtk_box_pack_start (GTK_BOX (tmp), dicePixmaps[i][j][GAME_YAHTZEE], FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (tmp), dicePixmaps[i][j][GAME_KISMET], FALSE, FALSE, 0);
    }

    diceBox[i] = gtk_toggle_tool_button_new ();
    gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (diceBox[i]), tmp);
    g_signal_connect (GTK_TOOL_BUTTON (diceBox[i]), "clicked",
		      G_CALLBACK (modify_dice), &DiceValues[i]);

    gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
			GTK_TOOL_ITEM (diceBox[i]), -1);

    gtk_widget_show (GTK_WIDGET (diceBox[i]));
    gtk_widget_show (tmp);
  /*gtk_widget_show (dicePixmaps[i][0][game_type]);*/
  }
  gtk_widget_show (toolbar);

  /* Scores displayed in score list */
  ScoreList = create_score_list ();
  gtk_box_pack_end (GTK_BOX (hbox), ScoreList, TRUE, TRUE, 0);
  setup_score_list (ScoreList);
  gtk_widget_show (ScoreList);

  gtk_widget_show (hbox);
  gtk_widget_show (vbox);
}

static void
GyahtzeeActivateGame (GApplication *app, gpointer user_data)
{
  GamesScoresImporter *importer;

  importer = GAMES_SCORES_IMPORTER (games_scores_directory_importer_new ());
  highscores = games_scores_context_new_with_importer ("tali",
                                                       _("Game Type:"),
                                                       GTK_WINDOW (window),
                                                       create_category_from_key, NULL,
                                                       GAMES_SCORES_STYLE_POINTS_GREATER_IS_BETTER,
                                                       importer);
  g_object_unref (importer);

  if (!gtk_widget_is_visible (window)) {
    gtk_widget_show (window);
    GyahtzeeNewGame ();
  }
}

int
main (int argc, char *argv[])
{
  char **player_names;
  gsize n_player_names;
  guint i;
  GOptionContext *context;
  gboolean retval;
  GError *error = NULL;

  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  application = gtk_application_new ("org.gnome.Tali", 0);
  g_signal_connect (application, "startup", G_CALLBACK (GyahtzeeCreateMainWindow), NULL);
  g_signal_connect (application, "activate", G_CALLBACK (GyahtzeeActivateGame), NULL);

  /* Reset all yahtzee variables before parsing args */
  YahtzeeInit ();

  context = g_option_context_new (NULL);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_add_main_entries (context, yahtzee_options,
				     GETTEXT_PACKAGE);
  retval = g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);
  if (!retval) {
    g_print ("%s", error->message);
    g_error_free (error);
    exit (1);
  }

  settings = g_settings_new ("org.gnome.Tali");

  g_set_application_name (_(appName));

  /* If we're in computer test mode, just run some tests, no GUI */
  if (test_computer_play > 0) {
      gint ii, jj, kk;
      gdouble sum_scores = 0.0;
      game_type = GAME_YAHTZEE;
      if (game_type_string)
          game_type = game_type_from_string (game_type_string);
      g_message ("In test computer play section - Using %d trials for simulation", NUM_TRIALS);
      for (ii = 0; ii < test_computer_play; ii++) {
          int num_rolls = 0;
          NumberOfHumans = 0;
          NumberOfComputers = 1;
          NewGame ();

          while (!GameIsOver () && num_rolls < 100) {
              ComputerRolling (CurrentPlayer);
              if (NoDiceSelected () || (NumberOfRolls >= NUM_ROLLS)) {
                ComputerScoring (CurrentPlayer);
                NumberOfRolls = 0;
                SelectAllDice ();
                RollSelectedDice ();
              } else {
                RollSelectedDice ();
              }
              num_rolls++;
          }
          for (kk = NumberOfHumans; kk < NumberOfPlayers; kk++) {
              printf ("Computer score: %d\n", total_score (kk));
              sum_scores += total_score (kk);
              if (num_rolls > 98) {
                  for (jj = 0; jj < NUM_FIELDS; jj++)
                      g_message ("Category %d is score %d", jj, players[kk].score[jj]);
              }
          }
      }
      printf ("Computer average: %.2f for %d trials\n", sum_scores / test_computer_play, NUM_TRIALS);
      exit (0);
  }

  gtk_window_set_default_icon_name ("org.gnome.Tali");

  if (NumberOfComputers == 0)	/* Not set on the command-line. */
    NumberOfComputers = g_settings_get_int (settings, "number-of-computer-opponents");

  if (NumberOfHumans == 0)	/* Not set on the command-line. */
    NumberOfHumans = g_settings_get_int (settings, "number-of-human-opponents");

  if (NumberOfHumans < 1)
    NumberOfHumans = 1;
  if (NumberOfComputers < 0)
    NumberOfComputers = 0;

  if (NumberOfHumans > MAX_NUMBER_OF_PLAYERS)
    NumberOfHumans = MAX_NUMBER_OF_PLAYERS;
  if ((NumberOfHumans + NumberOfComputers) > MAX_NUMBER_OF_PLAYERS)
    NumberOfComputers = MAX_NUMBER_OF_PLAYERS - NumberOfHumans;

  if (game_type_string)
    game_type = game_type_from_string (game_type_string);
  else {
    char *type;

    type = g_settings_get_string (settings, "game-type");
    game_type = game_type_from_string (type);
  }

  set_new_game_type (game_type);

  if (NUM_TRIALS <= 0)
      NUM_TRIALS = g_settings_get_int (settings, "monte-carlo-trials");

  if (DoDelay == 0)		/* Not set on the command-line */
    DoDelay = g_settings_get_boolean (settings, "delay-between-rolls");
  if (DisplayComputerThoughts == 0)	/* Not set on the command-line */
    DisplayComputerThoughts = g_settings_get_boolean (settings, "display-computer-thoughts");
  
  /* Read in new player names */
  player_names = g_settings_get_strv (settings, "player-names");
  n_player_names = g_strv_length (player_names);
  if (player_names) {
    n_player_names = MIN (n_player_names, MAX_NUMBER_OF_PLAYERS);

    for (i = 0; i < n_player_names; ++i) {
      if (i == 0 && strcasecmp (player_names[i], _("Human")) == 0) {
	const char *realname;

	realname = g_get_real_name ();
        if (realname && realname[0] && strcmp (realname, "Unknown") != 0) {
          players[i].name = g_locale_to_utf8 (realname, -1, NULL, NULL, NULL);
        }
        if (!players[i].name) {
	  players[i].name = g_strdup (player_names[i]);
	}
      } else {
        players[i].name = g_strdup (player_names[i]);
      }
    }

    g_strfreev (player_names);
  }

  g_application_run (G_APPLICATION (application), argc, argv);

  exit (0);
}
