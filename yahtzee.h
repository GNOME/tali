#ifndef _yahtzee_H_
#define _yahtzee_H_
/*
 * Gyatzee: Gnomified Yahtzee game.
 * (C) 1998 the Free Software Foundation
 *
 * File:   yahtzee.h
 *
 * Author: Scott Heavner
 *
 * This program is based on based on orest zborowski's curses based
 * yahtze (c)1992.
 */


#ifdef DONT_USE_LIBINTL
#define  _(s) (s)
#define N_(s) (s)
#else
#include <gnome.h>  /* What else can I use here ? */
#endif

#define COMPUTER_DELAY 1		/* sec */

#define NUM_ROLLS 3
#define NUMBER_OF_DICE       5

#define MAX_NUMBER_OF_PLAYERS 6
#define MAX_NAME_LENGTH 8
#define NUM_UPPER 6
#define NUM_LOWER 7
#define NUM_FIELDS (NUM_UPPER + NUM_LOWER)

#define EXTRA_FIELDS 4

/* Locations of fields containing totals */
#define F_LOWERT (NUM_FIELDS)
#define F_GRANDT (F_LOWERT+1)
#define F_UPPERT (F_GRANDT+1)
#define F_BONUS  (F_UPPERT+1)

#define H_3 6
#define H_4 7
#define H_FH 8
#define H_SS 9
#define H_LS 10
#define H_YA 11
#define H_CH 12

typedef struct
{
	char *name;
	short used[NUM_FIELDS];
	int score[NUM_FIELDS];
	int finished;
	int comp;
} Player;

typedef struct {
  int val;
  int sel;
} DiceInfo;

/* This is a little bigger than it needs to be */
typedef struct _UndoInfo {
  int player;
  int field;
  int fstate;
  int oldscore;
  struct _UndoInfo *prev;
} UndoInfo;

/* yahtzee.c */
extern DiceInfo DiceValues[];
extern Player players[];
extern int NumberOfPlayers;
extern int NumberOfHumans;
extern int NumberOfComputers;
extern int DoDelay;
extern int NumberOfRolls;
extern int WinningScore;
extern int DisplayComputerThoughts;
extern int OnlyShowScores;
extern int ExtraYahtzeeBonus;
extern int ExtraYahtzeeBonusVal;
extern int ExtraYahtzeeJoker;
extern int CurrentPlayer;
extern int IsCheater;
extern char *ProgramHeader;
extern char *FieldLabels[NUM_FIELDS+EXTRA_FIELDS];
extern char *DefaultPlayerNames[MAX_NUMBER_OF_PLAYERS];

extern void YahtzeeInit(void);
extern void NewGame(void);
extern void YahtzeeDelay(void);
extern int upper_total(int num);
extern int lower_total(int num);
extern int total_score(int num);
extern int count(int val);
extern int find_n_of_a_kind(int n, int but_not);
extern int find_straight(int run, int notstart, int notrun);
extern int find_yahtzee(void);
extern int add_dice(void);
extern int play_score(int player, int field);
extern int RegisterUndo(int player, int field, int fstate, int oldscore);
extern int ExecSingleUndo(int screenupdate);
extern void handle_play(int player);
extern void play(void);
extern void calc_random(void);
extern void say(char *fmt, ...);
extern void SelectAllDice(void);
extern int NoDiceSelected(void);
extern int RollDie(void);
extern void RollSelectedDice(void);
extern int GameIsOver(void);
#if 0
/* Don't need this function anymore--it was only ever used in gyahtzee.c, and
 * it's been commented out now. -- pschwan@cmu.edu */
extern int HumansAreDone(void);
#endif
extern int FindWinner(void);

/* Computer.c */
extern void ComputerTurn(int player);
extern void ComputerRolling(int player);

/* Specific to a windowing system: gyahtzee.c/cyahtzee.c */
extern void NewGame(void);
extern void UpdateDie(int no);
extern void ShowPlayer(int num, int field);
extern void NextPlayer(void);
extern void YahtzeeIdle(void);
extern void ShowHighScores(void);

enum { SCORE_OK=0, SLOT_USED, PLAYER_DONE, YAHTZEE_NEWGAME };


#endif /* _yahtzee_H_ */


/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
*/   
