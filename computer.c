/*
 * Gyatzee: Gnomified Yahtzee game.
 * (C) 1998 the Free Software Foundation
 *
 * File:   computer.c
 *
 * Author: Scott Heavner
 *
 *   Window manager independent yahtzee routines for computer opponents.
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
#include <config.h>
#include "yahtzee.h"


/*
**	questions are:
**		0:	none
**		1:	"what dice to roll again?"
**		2:	"where do you want to put that?"
*/

typedef struct {
  int score;        /* Score if we don't roll again */
} DiceRank;

static DiceRank bc_table[MAX_FIELDS];
extern int NUM_TRIALS;

static void
BuildTable (int player)
{
  int i;
  int d;
  int overflow;

  for (i = 0; i < NUM_FIELDS; ++i) {
    bc_table[i].score = 0;
    if (players[player].used[i]) {
      bc_table[i].score = -99;
    }
  }

/*
**	HANDLING UPPER SLOTS
*/

/*
**	first, we calculate the overflow of the upper ranks. that is, we
**	count all the points we have left over from our 3-of-all rule
**	if we get 3 of all in the upper ranks, we get a bonus, so if
**	we get 4 of something, that means a lower roll may be acceptable,
**	as long as we remain in the running for a bonus.  overflow can
**	be negative as well, in which case throwaway rolls are not
**	encouraged, and a high roll will get a nice boost
*/
  overflow = 0;

  for (i = 0; i < NUM_UPPER; ++i) {
    if (players[player].used[i])
      continue;

    overflow += (count (i + 1) - 3) * (i + 1);
  }

  for (i = 0; i < NUM_UPPER; ++i) {
    if (players[player].used[i])
      continue;

/*
**	ok. now we set a base value on the roll based on its count and
**	how much it is worth to us.
*/
    bc_table[i].score = (count(i + 1) - 2) * (i + 1) * 4 - (i + 1);
  }

/*
**	HANDLING LOWER SLOTS
*/

/*
**	now we look hard at what we got
*/

  /* Small straight */
  if (H_SS > 0 && !players[player].used[H_SS] && find_straight (4, 0, 0)) {
    d = find_straight (4, 0, 0);

    for (i = 0; i < 5; ++i)
      if (count (DiceValues[i].val) > 1 ||
	  DiceValues[i].val < d || DiceValues[i].val > d + 3) {
	break;
      }

    bc_table[H_SS].score = 30;
  }

  /* Large straight */
  if (!players[player].used[H_LS] && find_straight (5, 0, 0)) {
    bc_table[H_LS].score = 40;
  }

  /* chance - sum of all dice */
  if (!players[player].used[H_CH] && NumberOfRolls > 2) {
    bc_table[H_CH].score = add_dice ();

    if (bc_table[H_CH].score < 20)
      bc_table[H_CH].score /= 2;
  }

  /* Full house */
  if (!players[player].used[H_FH]) {
    d = find_n_of_a_kind (3, 0);

    if (d != 0) {
      if (find_n_of_a_kind (2, d)) {
	bc_table[H_FH].score = 25;
    if (game_type == GAME_KISMET) bc_table[H_FH].score += (add_dice () - 10);
      }
    }
  }

  /* Two pair same color */
  if (H_2P > 0 && !players[player].used[H_2P]) {
    d = find_n_of_a_kind (2, 0);

    if (d != 0) {
      if (find_n_of_a_kind (2, d) + d == 7 ||
          find_n_of_a_kind (4, d)) {
	bc_table[H_2P].score = add_dice ();
      }
    }
  }

  /* Full house same color */
  if (H_FS > 0 && !players[player].used[H_FS]) {
    d = find_n_of_a_kind (3, 0);

    if (d != 0) {
      if (find_n_of_a_kind (2, d) + d == 7 ||
          find_n_of_a_kind (5, 0)) {
	bc_table[H_FS].score = 20 + add_dice ();
      }
    }
  }

  /* Flush - all same color */
  if (H_FL > 0 && !players[player].used[H_FL]) {
    d = find_n_of_a_kind (3, 0);

    if (d != 0) {
      if (find_n_of_a_kind (2, d) + d == 7) {
	bc_table[H_FL].score = 35;
      }
    }

    d = find_n_of_a_kind (4, 0);
    if (d != 0) {
      if (find_n_of_a_kind (1, d) + d == 7) {
	bc_table[H_FL].score = 35;
      }
    }
    d = find_n_of_a_kind (5, 0);
    if (d != 0) {
	bc_table[H_FL].score = 35;
      }
  }

  /* 3 of a kind */
  if (!players[player].used[H_3]) {
    d = find_n_of_a_kind (3, 0);

    if (d != 0) {
      bc_table[H_3].score = add_dice ();
    }
  }

  /* 4 of a kind */
  if (!players[player].used[H_4]) {
    d = find_n_of_a_kind (4, 0);

    if (d != 0) {
/*
**	there will be a tie between 3 of a kind and 4 of a kind. we add 1
**	to break the tie in favor of 4 of a kind
*/
      bc_table[H_4].score = add_dice () + 1;
      if (game_type == GAME_KISMET) bc_table[H_4].score += 24;
    }
  }

  /* 5 of a kind */
  if (find_n_of_a_kind (5, 0)) {

    if (players[player].used[H_YA] && players[player].score[H_YA] == 0)
      bc_table[H_YA].score = -99;	/* scratch */

    else
      bc_table[H_YA].score = 150;	/* so he will use it! */
  }

  if (DisplayComputerThoughts) {
    for (i = 0; i < NUM_FIELDS; ++i) {
      fprintf (stderr, "%s : SCORE = %d\n",
	       _(FieldLabels[i]), bc_table[i].score);
    }
  }
}

/*
**	The idea here is to use a Monte Carlo simulation.
**	For each possible set of dice to roll, try NUM_TRIALS random rolls,
**	and average the scores. The highest average score will be the set
**	that we decide to roll.
**	Currently, this ignores the number of rolls a player has left.
**  We could try to do a second set of trials when there are two rolls
**  left, but that would take a lot more CPU time.
*/

void
ComputerRolling (int player)
{
  int i;
  int best;
  int bestv;
  int num_options = 1;
  int die_comp[NUMBER_OF_DICE];
  double best_score;
  int ii, jj, kk;

  for (i = 0; i < NUMBER_OF_DICE; i++) {
      die_comp[i] = num_options;
      num_options *= 2;
  }

  best = 0;
  bestv = -99;
  {
  double avg_score[num_options];
  memset(avg_score, 0, sizeof(avg_score));
  DiceInfo sav_DiceValues[NUMBER_OF_DICE];
  memcpy(sav_DiceValues, DiceValues, sizeof(sav_DiceValues));

  for (ii = 0; ii < num_options; ii++) {
      for (jj = 0; jj < NUM_TRIALS; jj++) {
          DiceInfo loc_info[NUMBER_OF_DICE];
          memcpy(loc_info, sav_DiceValues, sizeof(loc_info));
          for (kk = 0; kk < NUMBER_OF_DICE; kk++) {
              if (die_comp[kk] & ii) {
                  loc_info[kk].val = RollDie();
              }
          }
          memcpy(DiceValues, loc_info, sizeof(sav_DiceValues));
          BuildTable(player);
          bestv = -99;
          best  = 0;
          for (i = NUM_FIELDS - 1; i >= 0; i--) {
              if (bc_table[i].score >= bestv) {
                  best = i;
                  bestv = bc_table[i].score;
              }
          }
          avg_score[ii] += bestv;
      }
      avg_score[ii] /= NUM_TRIALS;
      if (DisplayComputerThoughts) printf ("Set avg score for %d to %f\n", ii, avg_score[ii]);
  }

  best  = 0;
  best_score = -99;
  for (ii = 0; ii < num_options; ii++) {
      if (avg_score[ii] >= best_score) {
          best = ii;
          best_score = avg_score[ii];
      }
  }
  if (DisplayComputerThoughts) printf("Best choice is %d val %f for dice ", best, best_score);

  /* Restore DiceValues */
  memcpy(DiceValues, sav_DiceValues, sizeof(sav_DiceValues));
  for (ii = 0; ii < NUMBER_OF_DICE; ii++) {
      if (die_comp[ii] & best) {
          DiceValues[ii].sel = 1;
          if (DisplayComputerThoughts) printf("Reset to roll die %d value %d bit %d comp %d test %d\n", ii, DiceValues[ii].val, ii, best, ii & best);
      }
      else {
          DiceValues[ii].sel = 0;
          if (DisplayComputerThoughts) printf("Reset NOT to roll die %d value %d bit %d comp %d test %d\n", ii, DiceValues[ii].val, ii, best, ii & best);
      }
  }
  }
}

/*
**	what we do here is generate a table of all the choices then choose
**	the highest value one.  in case of a tie, we go for the lower choice
**	cause higher choices tend to be easier to better
*/

void
ComputerScoring (int player)
{
  int i;
  int best;
  int bestv;

  NumberOfRolls = 2;		/* in case skipped middle */

  BuildTable (player);

  best = 0;

  bestv = -99;

  for (i = NUM_FIELDS - 1; i >= 0; --i) {
    if (player % 2) {
      if (bc_table[i].score > bestv) {
	best = i;

	bestv = bc_table[i].score;
      }
    }

    else {
      if (bc_table[i].score >= bestv) {
	best = i;

	bestv = bc_table[i].score;
      }
    }

    if (DisplayComputerThoughts) {
      fprintf (stderr, "<<BEST>> %s : VALUE = %d\n",
	       _(FieldLabels[best]),
	       bc_table[best].score);
    }

  }

  if (DisplayComputerThoughts)
      fprintf(stderr, "I choose category %d as best %d score name %s\n",
              best, bc_table[best].score, _(FieldLabels[best]));

  play_score (player, best);

}

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
*/
