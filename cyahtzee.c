/*
 * Gyatzee: Gnomified Yahtzee game.
 * (C) 1998 the Free Software Foundation
 *
 * File:   cyahtzee.c
 *
 * Author: Scott Heavner
 *
 *   Curses based yahtzee routines. 
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
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <config.h>
#include "yahtzee.h"
#include "cyahtzee.h"
#include "scores.h"


static char *DiceImage[6][3] =
{
	{ "     ",
	  "  o  ",
	  "     ",
	},

	{ "o    ",
	  "     ",
	  "    o"
	},

	{ "o    ",
	  "  o  ",
	  "    o"
	},

	{ "o   o",
	  "     ",
	  "o   o"
	},

	{ "o   o",
	  "  o  ",
	  "o   o"
	},

	{ "o   o",
	  "o   o",
	  "o   o"
	}
};

static int longest_header;
static int numlines;

static void abort_program(char *msg);

static void
init(void)
{
	int i;

        YahtzeeInit();

	for (i = 0; i < NUM_FIELDS; ++i)
		if (strlen(FieldLabels[i]) > longest_header)
			longest_header = strlen(FieldLabels[i]);

	/* Correct for leading "(X) " */
	longest_header += 4;

}

/*
**	SCREEN ORGANIZATION:
**		line 0:		version header
**		line 1:		-----------
**		line 2:		edit window
**		line 3:		-----------
**		line 4:		player names
**		line 5-10	upper bank
**		line 11:	upper total
**		line 12:	Bonus
**		line 13-19:	lower bank
**		line 20:	lower total
**		line 21:	total
*/

static void fill_box(int x,int y, int h, int w)
{
	int dx,dy;
	for(dy=y; dy<y+h; dy++)
	{
		for(dx=x;dx<x+w;dx++)
		{
			mvaddch(dx,dy,' ');
		}
	}
}

static void
setup_screen(void)
{
	int i;

	initscr();
	if (LINES < 23)
		abort_program(_("Not enough lines on the terminal"));
	numlines = LINES;

#ifndef NO_COLOR_CURSES
	if(has_colors())
	{
		start_color();
		init_pair(1, COLOR_BLACK, COLOR_WHITE);
		init_pair(2, COLOR_BLACK, COLOR_GREEN);
		init_pair(3, COLOR_WHITE, COLOR_GREEN);
		init_pair(4, COLOR_BLACK, COLOR_RED);
	}
#endif

	clear();
	BGColorOn();
	fill_box(0,0,COLS,LINES);
	mvaddstr(0, 9, ProgramHeader);
	move(1, 9);
	for (i = 9; i < COLS; ++i)
		addch(ACS_HLINE);
	refresh();
}

static void
yend(void)
{
	move(2, 0);
	clrtobot();
	refresh();
	endwin();
}

void
abort_program(char *msg)
{
	yend();
	fprintf(stderr,"\n%s\n",msg);
	exit(1);
}

void
say(char *fmt, ...)
{
	va_list ap;
	char buf[200];

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	/* Keep the color scheme going, clrtoeol() fills black */
	fill_box(2,10,COLS-10,1);

	mvaddstr(2, 10, buf);
	refresh();
}

/*
**	we have a trick in here - we will accept a '?' as the first character of
**	a human answer.  in that case, we ask the computer for the die to roll
**	or the place to put it.  then the human can decide what to do.
*/
static char *
query(int player, int question, char *prompt, char *ans, int len)
{
	int i;
	char c;
	int xpos;

	xpos = 10 + strlen(prompt);

        cbreak();
        noecho();
        
	/* Keep the color scheme going, clrtoeol() fills black */
	fill_box(2,10,COLS-10,1);

        mvaddstr(2, 10, prompt);
        refresh();
        
        i = 0;
        
        for (;;) {
                
                c = getchar();
                if (c == '\b' || c == 0x7f) {
                        
                        if (i == 0)
                                continue;
                        --i;
                        --xpos;
                        mvaddch(2, xpos, ' ');
                        move(2, xpos);
                } else if (c == 10 || c == 13) {
                        break;
                } else {
                        if (i == len) {
                                write(1, "\007", 1);
                        } else {
                                addch(c);
                                ans[i] = c;
                                ++xpos;
                                ++i;
                        }
                }
                refresh();
        }
        
        ans[i] = '\0';

        echo();
        nocbreak();
        
        return ans;
}

void
YahtzeeIdle()
{
}


void
UpdateDie(int num)
{
        int i;

	DiceColorOn();
        for (i = 0; i < 3; ++i) {
                move( num*4 + i + 1, 2);
                addstr(DiceImage[DiceValues[num].val - 1][i]);
        }
	DiceColorOff();
	refresh();
}

void
ShowPlayer(int num, int field)
{
	int i;
	int line;
	int upper_tot;
	int lower_tot;
	int xpos;

	xpos = 10 + longest_header + (num * MAX_NAME_LENGTH);

	for (i = 0; i < NUM_FIELDS; ++i) {

		if (i == field || field == -1) {

			line = 5 + i;

			if (i >= NUM_UPPER)
				line += 2;

			move(line, xpos);

			if (players[num].used[i])
				printw(" %4d", players[num].score[i]);

			else
				addstr("     ");
		}
	}

	upper_tot = upper_total(num);
	lower_tot = lower_total(num);

	move(12, xpos);

	if (upper_tot >= 63) {
		printw("+%4d", 35);
		upper_tot += 35;
	} else {
		addstr("    ");
        }

	mvprintw(11, xpos, "(%4d)", upper_tot);
	mvprintw(20, xpos, "(%4d)", lower_tot);
	mvprintw(21, xpos, "[%4d]", upper_tot + lower_tot);

	refresh();
}

static void
setup_board(void)
{
        char buf[5] = "( ) ";
	int i;
	int j;

	/* Draw scoreboard */
	move(3, 9);
	for (i = 9; i < COLS; ++i)
		addch(ACS_HLINE);

	for (i = 0; i < NUM_UPPER; ++i) {
                buf[1] = i+'a';
		mvaddstr(i+5,  9, buf);
		mvaddstr(i+5, 13, FieldLabels[i]);
        }

	TotalsColorOn();
	move(11, 9);
	addstr(FieldLabels[F_UPPERT]);
	move(12, 9);
	addstr(FieldLabels[F_BONUS]);
	TotalsColorOff();


	for (i = 0; i < NUM_LOWER; ++i) {
                buf[1] = i+'a'+NUM_UPPER;
		mvaddstr(i+13,  9, buf);
		mvaddstr(i+13, 13, FieldLabels[i+NUM_UPPER]);
        }

	TotalsColorOn();
	move(20, 9);
	addstr(FieldLabels[F_LOWERT]);
	move(21, 9);
	addstr(FieldLabels[F_GRANDT]);
	TotalsColorOff();

	for (j = 0; j < NumberOfPlayers; ++j) {

                mvaddch(3, 9 + longest_header + (j * MAX_NAME_LENGTH),
                        ACS_TTEE);
		for (i = 4; i < 22; ++i)
			mvaddch(i, 9 + longest_header + (j * MAX_NAME_LENGTH),
				ACS_VLINE);
	}

	for (i = 0; i < NumberOfPlayers; ++i) {

		mvaddstr(4, 10 + longest_header + (i * MAX_NAME_LENGTH),
			 players[i].name);
		ShowPlayer(i, -1);
	}


	/* Draw die outlines */
        for (j = 0; j < NUMBER_OF_DICE; j++) {

                for (i = 0; i < 4; ++i) {

                        move((j * 4) + i, 0);

                        if (i == 2)
                                printw("%d", j+1);
                        else
                                addch(' ');
			DiceColorOn();
			addch(ACS_VLINE);
			addstr("     ");
			addch(ACS_VLINE);
			DiceColorOff();
                }

        } 
	DiceColorOn();
        for (j = 0; j < NUMBER_OF_DICE+1; j++) {
                mvaddch((j * 4), 1, ACS_LTEE);
                for (i=0; i<5; i++)
                        addch(ACS_HLINE);
                addch(ACS_RTEE);
        }
        mvaddch(20, 1, ACS_LLCORNER);
        mvaddch(20, 7, ACS_LRCORNER);
        mvaddch(0,  1, ACS_ULCORNER);
        mvaddch(0,  7, ACS_URCORNER);
	DiceColorOff();

	refresh();
}

static void
showoff(int p, short so)
{
	move(4, 10 + longest_header + (p * MAX_NAME_LENGTH));

	if (so) {
		PlayerColorOn();
	} else {
		PlayerColorOff();
        }

	addstr(players[p].name);

	if (so) {
		PlayerColorOff();
        }

	refresh();
}


static void
HumanTurn(int player)
{
	int i;
	char buf[50];
	char *cp;
	int done;

	while ( NumberOfRolls<3 ) {

                cp = query(player, 1, 
                           _("What dice to roll again (<RETURN> for none)? "),
                           buf, sizeof(buf));

                if (*cp == '\0') {	 /* Player pressed return */

                        break;

                } else if (*cp == '?') { /* Player is stupid and wants hint */

                        ComputerRolling(player);

                } else {
                        while (*cp != '\0') {
                                if (*cp >= '1' && *cp <= '5')
                                        DiceValues[*cp - '1'].sel = 1;
                                ++cp;
                        }
                }
                
		RollSelectedDice();
		
	}

	query(player, 2, _("Where do you want to put that? "),
	      buf, sizeof(buf));
	done = 0;

	for (;;) {

		if (buf[0] < 'a' || buf[0] > 'm') {
			query(player, 2, 
			      _("No good! Where do you want to put that? "),
                              buf, sizeof(buf));
			continue;
		}

		i = play_score(player,buf[0] - 'a');

                if (i==SCORE_OK)
                        break;
                else if (i==SLOT_USED)
                        query(player, 2, 
			      _("Already used! Where do you want"
				" to put that? "),
                              buf, sizeof(buf));
	}


}

void
handle_play(int player)
{

	if (players[player].finished)	/* all finished */
		return;

	showoff(player, 1);

	say(_("Rolling for %s"), players[player].name);

	NumberOfRolls=0;
	SelectAllDice();
	RollSelectedDice();

        if (players[player].comp)
                ComputerTurn(player);
        else
                HumanTurn(player);

	showoff(player, 0);

}


static void
PlayNewGame(void)
{
	int i;
	int winner;

        NewGame();

	for (;;) {

		for (i = 0; i < NumberOfPlayers; i++)
			handle_play(i);

		for (i = 0; i < NumberOfPlayers; i++)
			if (!players[i].finished)
				break;

		if (i == NumberOfPlayers)
			break;
	}

        winner = FindWinner();

        if (players[winner].name)
                say(_("%s wins the game with %d points"),
                    players[winner].name,
		    WinningScore);
	else
                say(_("Game over!"));

        sleep(5);
}


static void
show_top_scores(void)
{
	FILE *fp;
	char scorefile[200];
	int i, j, k, score;
	char name[32], date[32];

	printf("%s...\n",_("Tali top scores"));

	sprintf(scorefile, "%s/%s", SCOREDIR, SCOREFNAME);

	if ((fp = fopen(scorefile, "r")) == NULL) {
		printf("%s\n",_("Can't get at score file."));
		return;
	}

	j = 0;

	for (i = 0; i < NUM_TOP_PLAYERS; ++i) {
		if (read_score(fp, name, date, &score))
			break;

		if (j >= numlines - 4) {
			printf("<%s>",_("Hit Return"));
			fflush(stdout);
			getchar();
			j = 0;
		}

		for (k = 0; k < NumberOfPlayers; ++k)
			if (players[k].finished == i)
				break;

		printf("%3d : %-10s %s %d\n", i + 1, name, date, score);

		++j;
	}

	fclose(fp);

	printf("<%s>...",_("Hit Return"));
	fflush(stdout);
	getchar();
}

static void
signal_trap(int ignored)
{
	yend();
	exit(0);
}

static void
set_signal_traps(void)
{
	signal(SIGHUP, signal_trap);
	signal(SIGINT, signal_trap);
	signal(SIGQUIT, signal_trap);
}

int
main(int argc, char **argv)
{
	char num[10];
	int i;
	short onlyshowscores = 0;

	while (--argc > 0) {

		if ((*++argv)[0] == '-') {

			switch ((*argv)[1]) {

                        case 's':
                                onlyshowscores = 1;
                                break;
                                
                        case 'n':
                                printf("%s\n",_("obsolete function - delay"
						" turned off by default)"));
                                break;
                                
                        case 'd':
                                DoDelay = 1;
                                break;
                                
                        case 'r':
                                calc_random();
                                exit(0);
                                
                        default:
                                printf("usage: ctali [-s] [-d] [-r]\n");
                                printf("\t-s\tonly show scores\n");
                                printf("\t-d\tcomputer move delay\n");
                                printf("\t-r\tcalculate random die throws (debug)\n");
                                exit(0);
			}
		}
	}

	if (!onlyshowscores) {
		printf("\n\n%s...\n\n",_("Welcome to the game of Tali"));
                
		init();
                
		do {
			printf(_("How many wish to play (max of %d)? "),
			  MAX_NUMBER_OF_PLAYERS);
			fflush(stdout);

			fgets(num, 10, stdin);

			if (strchr(num, '\n'))
				*strchr(num, '\n') = '\0';

			NumberOfHumans = atoi(num);

			if (NumberOfHumans == 0)
				break;

		} while (NumberOfHumans < 1 || NumberOfHumans > MAX_NUMBER_OF_PLAYERS);

		for (i = 0; i < NumberOfHumans; ++i) {

			printf(_("What is the name of player #%d ? "), i + 1);
			fflush(stdout);

                        players[i].name = (char *)malloc(MAX_NAME_LENGTH);

			fgets(players[i].name, MAX_NAME_LENGTH, stdin);

			if (strchr(players[i].name, '\n'))
				*strchr(players[i].name, '\n') = '\0';

                        players[i].comp = 0;
		}

		if (NumberOfHumans == MAX_NUMBER_OF_PLAYERS) {

			printf(_("Boo hoo... I can't play...\n"));

		} else {
			do {
				printf(_("How many computers to "
					 "play (max of %d) ? "),
				  MAX_NUMBER_OF_PLAYERS - NumberOfComputers);
				fflush(stdout);

				fgets(num, sizeof(num), stdin);

				NumberOfComputers = atoi(num);
			} while (NumberOfComputers < 0 ||
                                 NumberOfPlayers + NumberOfComputers > MAX_NUMBER_OF_PLAYERS);

		}

                NumberOfPlayers = NumberOfHumans + NumberOfComputers;

		if (NumberOfPlayers == 0) {
			printf(_("Well, why did you run this anyways???\n\n"));
			exit(8);
		}
	}

	setup_screen();

	set_signal_traps();

	if (!onlyshowscores) {
		setup_board();
		PlayNewGame();
		update_scorefile();
	}

	yend();
	show_top_scores();

	return 0;
}

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
*/   
