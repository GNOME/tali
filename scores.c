/*
 * Gyatzee: Gnomified Yahtzee game.
 * (C) 1998 the Free Software Foundation
 *
 * File:   scores.c
 *
 * Author: Scott Heavner
 *
 *   High score logging routines (not used with GNOME).
 *
 *   Variables are exported in scores.h
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
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>

#include <config.h>

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "yahtzee.h"
#include "cyahtzee.h"
#include "scores.h"

#define L_LOCK 0
#define L_UNLOCK 1

static void
lock(char *fname, int type)
{
	char lockfile[200];
	struct stat statbuf;
	int i;
	FILE *fp;

	strcpy(lockfile, fname);

	strcat(lockfile, ".L");

	if (type == L_LOCK)
	{
		for (i = 1; ;++i)
		{
			stat(lockfile, &statbuf);

			if (errno == ENOENT)
				break;

			say("Waiting for lock... (%d)", i);

			sleep(1);
		}

		fp = fopen(lockfile, "w");

		fclose(fp);
	}

	else
	{
		unlink(lockfile);
	}
}

static int
write_score(FILE *fp, char *name, char *date, int score)
{
	return fprintf(fp, "%s\001%s\001%d\n", name, date, score);
}

int
read_score(FILE *fp, char *name, char *date, int *score)
{
	char buf[200];
	char *sb, *se;

	if (!fgets(buf, sizeof(buf), fp))
		return -1;
	if (!(se = strchr(buf, '\001')))
		return -1;
	*se++ = '\0';
	strcpy(name, buf);
	if ((sb = se) > buf + sizeof(buf))
		return -1;
	if (!(se = strchr(sb, '\001')))
		return -1;
	*se++ = '\0';
	strcpy(date, sb);
	if ((sb = se) > buf + sizeof(buf))
		return -1;
	if (!(se = strchr(sb, '\n')))
		return -1;
	*se = '\0';
	*score = atoi(sb);
	return 0;
}

/*
**	we keep track of the top nnn persons.
*/
void
update_scorefile(void)
{
	FILE *fp;
	FILE *tp;
	char scorefile[200];
	char tmpfile[100];
	int numtop;
	int nump;
	int j;
	char name[20];
	char date[30];
	char *curdate;
	int score;
	int topscore;
	int tmptop;
	long clock;

	sprintf(tmpfile, "%s/y.%x", SCOREDIR, getpid());
	sprintf(scorefile, "%s/%s", SCOREDIR, SCOREFNAME);

	clock = time(0);

	curdate = (char *) ctime(&clock);

	if (strchr(curdate, '\n'))
		*strchr(curdate, '\n') = '\0';

	if ((tp = fopen(tmpfile, "w")) == NULL)
	{
		say("Can't update score file.");
		return;
	}

	lock(scorefile, L_LOCK);

	fp = fopen(scorefile, "r");

	for (j = 0; j < NumberOfPlayers; ++j)
		players[j].finished = -1;

	numtop = 0;
	nump = 0;
	topscore = 99999;

	for (;;)
	{
/*
**	get the next entry from the score file.  if there isn't any, then
**	we set the score to beat to -99 (everyone playing can beat it)
*/
		if (fp == NULL || read_score(fp, name, date, &score))
			score = -99;

/*
**	now, we search through all players to find out which ones have scores
**	higher than the one read (but less than topscore).  these will get
**	saved before the read entry does. now, we only do this if all players
**	haven't been accounted for.
*/
		for (; nump != NumberOfPlayers;)
		{
			tmptop = -99;

			for (j = 0; j < NumberOfPlayers; ++j)
				if (total_score(j) > tmptop &&
				  total_score(j) < topscore)
				{
					tmptop = total_score(j);
				}

			if (tmptop == -99)	/* everybody better */
				break;

			if (tmptop > score)
			{
				topscore = tmptop;

				for (j = 0; j < NumberOfPlayers; ++j)
					if (total_score(j) == tmptop)
					{
						if (numtop >= NUM_TOP_PLAYERS)
							break;

						write_score(tp,
						  players[j].name, curdate,
						  tmptop);

						players[j].finished = numtop;

						++nump;

						++numtop;
					}
			}

			else
				break;
		}

		if (score != -99 && numtop < NUM_TOP_PLAYERS)
		{
			write_score(tp, name, date, score);
			++numtop;
		}
/*
**	if we processed all top slots or processed all players (and there
**	was no score to beat), we stop.
*/
		if (numtop == NUM_TOP_PLAYERS ||
		  (nump == NumberOfPlayers && score == -99))
			break;
	}

	fclose(tp);
	if (fp)
		fclose(fp);

#ifdef HAVE_RENAME
	if (rename(tmpfile, scorefile))
	{
		say("rename failed!");
		unlink(tmpfile);
	}
#else
        {
                char scall[300];
                sprintf(scall, "mv %s %s", tmpfile, scorefile);
                system(scall);
        }
#endif

	lock(scorefile, L_UNLOCK);
}

/* Arrgh - lets all use the same tabs under emacs: 
Local Variables:
tab-width: 8
c-basic-offset: 8
indent-tabs-mode: nil
*/   
