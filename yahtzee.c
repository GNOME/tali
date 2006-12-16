/*
 * Gyatzee: Gnomified Yahtzee game.
 * (C) 1998 the Free Software Foundation
 *
 * File:   yahtzee.c
 *
 * Author: Scott Heavner
 *
 *   Window manager independent yahtzee routines.
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <config.h>
#include "yahtzee.h"

char *ProgramHeader = "Yahtzee Version 2.00 (c)1998 SDH, (c)1992 by zorst";


/*=== Exported variables ===*/
DiceInfo DiceValues[5] = { {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0} };
Player players[MAX_NUMBER_OF_PLAYERS] = {
  {NULL, {0}, {0}, 0, 0},
  {NULL, {0}, {0}, 0, 0},
  {NULL, {0}, {0}, 0, 0},
  {NULL, {0}, {0}, 0, 0},
  {NULL, {0}, {0}, 0, 0},
  {NULL, {0}, {0}, 0, 0}
};
int NumberOfPlayers = 0;
int NumberOfComputers = 0;
int NumberOfHumans = 0;
int DoDelay = 0;
int NumberOfRolls;
int WinningScore;
int DisplayComputerThoughts = 0;
int CurrentPlayer;
GameType game_type = GAME_YAHTZEE;
int NUM_FIELDS = NUM_FIELDS_YAHTZEE;
int NUM_LOWER = NUM_LOWER_YAHTZEE;

char *DefaultPlayerNames[MAX_NUMBER_OF_PLAYERS] = { N_("Human"),
  "Wilber",
  "Bill",
  "Monica",
  "Kenneth",
  "Janet"
};
char *FieldLabelsYahtzee[NUM_FIELDS_YAHTZEE + EXTRA_FIELDS] = {
  N_("1s [total of 1s]"),
  N_("2s [total of 2s]"),
  N_("3s [total of 3s]"),
  N_("4s [total of 4s]"),
  N_("5s [total of 5s]"),
  N_("6s [total of 6s]"),
  /* End of upper panel */
  N_("3 of a Kind [total]"),
  N_("4 of a Kind [total]"),
  N_("Full House [25]"),
  N_("Small Straight [30]"),
  N_("Large Straight [40]"),
  N_("5 of a Kind [50]"),
  N_("Chance [total]"),
  /* End of lower panel */
  N_("Lower Total"),
  N_("Grand Total"),
  /* Need to squish between upper and lower pannel */
  N_("Upper total"),
  N_("Bonus if >62"),
};

char *FieldLabelsKismet[NUM_FIELDS_KISMET+EXTRA_FIELDS] =
{
  N_("1s [total of 1s]"),
  N_("2s [total of 2s]"),
  N_("3s [total of 3s]"),
  N_("4s [total of 4s]"),
  N_("5s [total of 5s]"),
  N_("6s [total of 6s]"),
  /* End of upper panel */
  N_("2 pair Same Color [total]"),
  N_("3 of a Kind [total]"),
  N_("Full House [15 + total]"),
  N_("Full House Same Color [20 + total]"),
  N_("Flush (all same color) [35]"),
  N_("Large Straight [40]"),
  N_("4 of a Kind [25 + total]"),
  N_("5 of a Kind [50 + total]"),
  N_("Chance [total]"),
  /* End of lower panel */
  N_("Lower Total"),
  N_("Grand Total"),
  /* Need to squish between upper and lower pannel */
  N_("Upper total"),
  N_("Bonus if >62"),
};

char **FieldLabels = FieldLabelsKismet;

int
NoDiceSelected (void)
{
  int i, j = 1;
  for (i = 0; i < 5; i++)
    if (DiceValues[i].sel)
      j = 0;
  return j;
}

void
SelectAllDice (void)
{
  int i;
  for (i = 0; i < 5; i++)
    DiceValues[i].sel = 1;
}

void
YahtzeeInit (void)
{
  int i;

  srand (time (NULL));

  for (i = 0; i < MAX_NUMBER_OF_PLAYERS; ++i) {
    players[i].name = _(DefaultPlayerNames[i]);
    players[i].comp = 1;
  }

  /* Make player number one human */
  players[0].comp = 0;

}

/* Must be called after window system is initted */
void
NewGame (void)
{
  int i, j;

  if (game_type == GAME_YAHTZEE) {
    FieldLabels = FieldLabelsYahtzee;
    NUM_FIELDS = NUM_FIELDS_YAHTZEE;
    NUM_LOWER = NUM_LOWER_YAHTZEE;
  }
  else if (game_type == GAME_KISMET) {
    FieldLabels = FieldLabelsKismet;
    NUM_FIELDS = NUM_FIELDS_KISMET;
    NUM_LOWER = NUM_LOWER_KISMET;
  }

  CurrentPlayer = 0;
  NumberOfRolls = 0;

  NumberOfPlayers = NumberOfComputers + NumberOfHumans;

  for (i = 0; i < MAX_NUMBER_OF_PLAYERS; ++i) {
    players[i].finished = 0;
    players[i].comp = 1;

    for (j = 0; j < NUM_FIELDS; ++j) {
      players[i].score[j] = 0;
      players[i].used[j] = 0;
    }
  }

  /* Possibly 0 humans? */
  for (i = 0; i < NumberOfHumans; i++)
    players[i].comp = 0;

  SelectAllDice ();
  RollSelectedDice ();
}

int
RollDie (void)
{
  return ((rand () % 6) + 1);

}

void
RollSelectedDice (void)
{
  int i, cnt = 0;

  if (NumberOfRolls >= NUM_ROLLS) {
    return;
  }

  for (i = 0; i < 5; i++) {
    if (DiceValues[i].sel) {
      DiceValues[i].val = RollDie ();
      DiceValues[i].sel = 0;
      cnt++;
    }
  }

  /* If no dice is selcted roll them all */
  if(cnt == 0){
    for (i = 0; i < 5; i++) {
      DiceValues[i].val = RollDie ();
    }
  }

  UpdateAllDicePixmaps ();
  DeselectAllDice ();

  NumberOfRolls++;

  if (NumberOfRolls >= NUM_ROLLS) {
    say (_("Choose a score slot."));
  }

}

int
GameIsOver (void)
{
  int i;

  for (i = 0; i < NumberOfPlayers; i++)
    if (players[i].finished == 0)
      return 0;
  return 1;
}

int
upper_total (int num)
{
  int val;
  int i;

  val = 0;

  for (i = 0; i < NUM_UPPER; ++i)
    val += players[num].score[i];

  return (val);
}

int
lower_total (int num)
{
  int val;
  int i;

  val = 0;

  for (i = 0; i < NUM_LOWER; ++i)
    val += players[num].score[i + NUM_UPPER];

  return (val);
}

int
total_score (int num)
{
  int upper_tot;
  int lower_tot;

  upper_tot = 0;
  lower_tot = 0;

  lower_tot = lower_total (num);
  upper_tot = upper_total (num);

  if (game_type == GAME_KISMET && upper_tot >= 78)
    upper_tot += 75;
  else if (game_type == GAME_KISMET && upper_tot >= 71)
    upper_tot += 55;
  else if (upper_tot >= 63)
    upper_tot += 35;

  return (upper_tot + lower_tot);
}

int
count (int val)
{
  int i;
  int num;

  num = 0;

  for (i = 0; i < 5; ++i)
    if (DiceValues[i].val == val)
      ++num;

  return (num);
}

int
find_n_of_a_kind (int n, int but_not)
{
  int i;

  for (i = 0; i < 5; ++i) {
    if (DiceValues[i].val == but_not)
      continue;

    if (count (DiceValues[i].val) >= n)
      return (DiceValues[i].val);
  }

  return (0);
}

int
find_straight (int run, int notstart, int notrun)
{
  int i;
  int j;

  for (i = 1; i < 7; ++i) {
    if (i >= notstart && i < notstart + notrun)
      continue;

    for (j = 0; j < run; ++j)
      if (!count (i + j))
	break;

    if (j == run)
      return (i);
  }

  return (0);
}

int
find_yahtzee (void)
{
  int i;

  for (i = 1; i < 7; ++i)
    if (count (i) == 5)
      return (i);

  return (0);
}

int
add_dice (void)
{
  int i;
  int val;

  val = 0;

  for (i = 0; i < 5; ++i)
    val += DiceValues[i].val;

  return (val);
}

static void play_score_kismet(int player, int field)
{
  int i;

  players[player].used[field] = 1;

  switch (field) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
    players[player].score[field] = count (field + 1) * (field + 1);
    break;

  case 6: /* 2 pair same color */
    if ((i = find_n_of_a_kind(2, 0))) {
            int j = find_n_of_a_kind(2, i);
        if (i + j == 7 || find_n_of_a_kind(4, 0))
            players[player].score[field] = add_dice ();
    }
    break;

  case 7: /* 3 of a kind */
    if (find_n_of_a_kind (3, 0))
      players[player].score[field] = add_dice ();
    break;

  case 8: /* Full house */
    if ((i = find_n_of_a_kind (3, 0))) {
      /* We treat a yahtzee as a full house. */
      if (find_n_of_a_kind (2, i) || find_n_of_a_kind (5, 0))
        players[player].score[field] = 15 + add_dice ();
    }
    break;

  case 9: /* Full house same color */
    if ((i = find_n_of_a_kind(3, 0))) {
      /* We treat a yahtzee as a full house. */
      if (find_n_of_a_kind(2, i) + i == 7 || find_n_of_a_kind(5,0))
         players[player].score[field] = 20 + add_dice ();
    }
    break;

  case 10: /* Flush - all same color */
    if (find_n_of_a_kind(5, 0))
      /* We treat a yahtzee as a flush. */
      players[player].score[field] = 35;
    else if ((i = find_n_of_a_kind(4, 0)) ) {
      if (find_n_of_a_kind(1, i) + i == 7)
        players[player].score[field] = 35;
    }
    else if ((i = find_n_of_a_kind(3, 0)) ) {
      if (find_n_of_a_kind(2, i) + i == 7)
        players[player].score[field] = 35;
    }

  case 11: /* Straight */
    if (find_straight (5, 0, 0))
      players[player].score[field] = 40;
    break;

  case 12: /* 4 of a kind */
    if (find_n_of_a_kind (4, 0))
      players[player].score[field] = 25 + add_dice ();
    break;

  case 13: /* Yahtzee or kismet */
    if (find_yahtzee ())
      players[player].score[field] += 50 + add_dice ();
    break;

  case 14: /* Chance or yarborough */
    players[player].score[field] = add_dice ();
    break;
  }
}

static void play_score_yahtzee(int player, int field)
{
  int i;

  switch (field) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
    players[player].score[field] = count (field + 1) * (field + 1);
    break;

  case 6: /* 3 of a kind */
    if (find_n_of_a_kind (3, 0))
      players[player].score[field] = add_dice ();
    break;

  case 7: /* 4 of a kind */
    if (find_n_of_a_kind (4, 0))
      players[player].score[field] = add_dice ();
    break;

  case 8: /* Full house */
    if ((i = find_n_of_a_kind (3, 0))) {
      /* We treat a yahtzee as a full house. */
      if (find_n_of_a_kind (2, i) || find_n_of_a_kind (5, 0))
	players[player].score[field] = 25;
    }
    break;

  case 9: /* Small Straight */
    if (find_straight (4, 0, 0))
      players[player].score[field] = 30;
    break;

  case 10: /* Large Straight */
    if (find_straight (5, 0, 0))
      players[player].score[field] = 40;
    break;

  case 11: /* Yahtzee */
    if (find_yahtzee ())
      players[player].score[field] += 50;
    break;

  case 12: /* Chance */
    players[player].score[field] = add_dice ();
    break;
  }
}

/* Test if we can use suggested score slot */
int
play_score (int player, int field)
{
  int i;

  /* Special case for yahtzee, allow multiple calls if 1st wasn't 0 */

  /* This, however, was broken: it didn't actually check to see if the
   * user had a yahtzee if this wasn't their first time clicking on it.
   * Bad. -- pschwan@cmu.edu */
  if (field == 11 && game_type == GAME_YAHTZEE) {
    if ((players[player].used[11] && (players[player].score[11] == 0)))
      return SLOT_USED;

    if ((players[player].used[11] && !find_yahtzee ()))
      return SLOT_USED;
  } else if (players[player].used[field])
    return SLOT_USED;

  players[player].used[field] = 1;

  if (game_type == GAME_KISMET)
    play_score_kismet(player, field);
  else if (game_type == GAME_YAHTZEE)
    play_score_yahtzee(player, field);
  else {
    fprintf(stderr, "Unexpected game type %d. Aborting...", game_type);
    exit(1);
  }

  ShowPlayer (player, field);

  for (i = 0; i < NUM_FIELDS; ++i)
    if (!players[player].used[i])
      return SCORE_OK;

  players[player].finished = 1;

  return SCORE_OK;
}

int
FindWinner (void)
{
  int i;
  int winner = 0;
  int total;

  WinningScore = 0;

  for (i = 0; i < NumberOfPlayers; ++i) {
    total = total_score (i);
    if (total > WinningScore) {
      WinningScore = total_score (i);
      winner = i;
    }
  }

  /* Detect a drawn game. Returning the negative of the score 
   * is a bit of a hack, but it allows us to find out who the winners
   * were without having to pass around a list. */
  for (i = 0; i < NumberOfPlayers; ++i) {
    total = total_score (i);
    if ((total == WinningScore) && (i != winner))
      return -total;
  }

  return winner;
}



void
calc_random (void)
{
  char nrollstr[10];
  int nroll;
  int table[NUM_FIELDS];
  int i;
  int j;

  printf ("%s ", _("How many times do you wish to roll?"));

  fgets (nrollstr, 10, stdin);
  nroll = atoi (nrollstr);

  printf ("%s\n", _("Generating ..."));

  for (i = 0; i < NUM_FIELDS; ++i)
    table[i] = 0;

  for (i = 0; i < nroll; ++i) {

    for (j = 0; j < 5; ++j)
      DiceValues[j].val = RollDie ();

    for (j = 1; j <= 6; ++j)
      if (count (j) > 0)
	++table[j - 1];

    if (find_n_of_a_kind (3, 0))
      ++table[6];

    if (find_n_of_a_kind (4, 0))
      ++table[7];

    j = find_n_of_a_kind (3, 0);

    if (j != 0 && find_n_of_a_kind (2, j))
      ++table[8];

    if (find_straight (4, 0, 0))
      ++table[9];

    if (find_straight (5, 0, 0))
      ++table[10];

    if (find_yahtzee ())
      ++table[11];
  }

  printf ("%-35s: %10s %20s\n", _("Results"), _("Num Rolls"), _("Total"));

  for (i = 0; i < NUM_FIELDS; ++i) {

    printf ("%-35s", _(FieldLabels[i]));

    printf (" %10d %20ld\n", table[i], (long) (table[i] * 100) / nroll);
  }

}

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
*/
