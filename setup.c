/*
 * Gyatzee: Gnomified Yahtzee game.
 * (C) 1998 the Free Software Foundation
 *
 * File:   setup.c
 *
 * Author: Scott Heavner
 *
 *   Contains parser for command line arguments and 
 *   code for properties window.
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

#include "yahtzee.h"
#include "gyahtzee.h"

static error_t parse_an_arg (int key, char *arg, struct argp_state *state);

static void setupdialog_destroy(GtkWidget *widget, gint mode);
static GtkWidget *setupdialog = NULL;
static GtkWidget *sentry;


/* This describes all the arguments we understand.  */
static struct argp_option options[] =
{
  { NULL, 'd', NULL, 0, N_("Delay computer moves"), 0 },
  { NULL, 's', NULL, 0, N_("Show high scores and exit"), 0 },
  { NULL, 'r', NULL, 0, N_("Calculate random die throws (debug)"), 0 },
  { NULL, 'n', N_("NUMBER"), 0, N_("Number of opponents"), 1 },
  { NULL, 't', NULL, 0, N_("Display computer thoughts"), 0 },

  { NULL, 0, NULL, 0, NULL, 0 }
};

/* Our parser.  */
struct argp GyahtzeeParser =
{
  options,
  parse_an_arg,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};


static error_t
parse_an_arg (int key, char *arg, struct argp_state *state)
{
        static int np_set = 0;

	switch (key)
	{
	case 'd':
                DoDelay = 1;
		break;
	case 's':
                OnlyShowScores = 1;
		break;
	case 'n':
		np_set = 1;
		NumberOfComputers = atoi (arg);
		break;
	case 'r':
                calc_random();
                GyahtzeeAbort = 1;
		break;
	case 't':
		DisplayComputerThoughts = 1;
		break;
	case ARGP_KEY_SUCCESS:
		if (!np_set)
                        NumberOfComputers = 
                                gnome_config_get_int("/gyahtzee/Preferences/NumberOfComputerOpponents=1");
		break;
		
	default:
		return ARGP_ERR_UNKNOWN;
	}
	
	return 0;
}

static void
WarnNumPlayersChanged (void)
{
  GtkWidget *mb;

  mb = gnome_message_box_new (_("Current game will complete" 
				" with original number of player."),
                              GNOME_MESSAGE_BOX_INFO,
                              GNOME_STOCK_BUTTON_OK,
                              NULL);
  GTK_WINDOW(mb)->position = GTK_WIN_POS_MOUSE;
  gnome_dialog_set_modal(GNOME_DIALOG(mb));
  gtk_widget_show (mb);
}


static void 
do_setup(GtkWidget *widget, gpointer data)
{
        NumberOfComputers = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sentry));

	setupdialog_destroy(setupdialog, 1);

	gnome_config_set_int("/gyahtzee/Preferences/NumberOfComputerOpponents", NumberOfComputers);
	gnome_config_sync();

	WarnNumPlayersChanged();
}

static void 
setupdialog_destroy(GtkWidget *widget, gint mode)
{
	if (mode == 1) {
		gtk_widget_destroy(setupdialog);
	}
	setupdialog = NULL;
}


static void
set_delay (GtkWidget *widget, gpointer *data)
{
       DoDelay = GTK_TOGGLE_BUTTON (widget)->active;
}

static void
set_thoughts (GtkWidget *widget, gpointer *data)
{
        DisplayComputerThoughts = GTK_TOGGLE_BUTTON (widget)->active;
}

void setup_game(GtkWidget *widget, gpointer data)
{
        GtkWidget *all_boxes;
	GtkWidget *box, *box2;
        GtkWidget *label;
	GtkWidget *button;
	GtkWidget *frame;
        GtkObject *adj;

        if (setupdialog) return;

	setupdialog = gtk_window_new(GTK_WINDOW_DIALOG);

	gtk_container_border_width(GTK_CONTAINER(setupdialog), 10);
	GTK_WINDOW(setupdialog)->position = GTK_WIN_POS_MOUSE;
	gtk_window_set_title(GTK_WINDOW(setupdialog), _("Gyahtzee setup"));
	gtk_signal_connect(GTK_OBJECT(setupdialog),
			   "delete_event",
			   GTK_SIGNAL_FUNC(setupdialog_destroy),
			   0);

	all_boxes = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(setupdialog), all_boxes);

	frame = gtk_frame_new(_("Computer Opponents"));
	gtk_box_pack_start(GTK_BOX(all_boxes), frame, TRUE, TRUE, 0);
	
	box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), box);

        /*--- Button ---*/
	button = gtk_check_button_new_with_label (
                         _("Delay between rolls") );
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);
        gtk_toggle_button_set_state( GTK_TOGGLE_BUTTON(button),DoDelay);
	gtk_signal_connect (GTK_OBJECT(button), 
                            "clicked", (GtkSignalFunc)set_delay, NULL);
	gtk_widget_show (button);

        /*--- Button ---*/
	button = gtk_check_button_new_with_label (
                         _("Show thoughts during turn") );
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);
        gtk_toggle_button_set_state( GTK_TOGGLE_BUTTON(button),
                                     DisplayComputerThoughts);
	gtk_signal_connect (GTK_OBJECT(button), 
                            "clicked", (GtkSignalFunc)set_thoughts, NULL);
	gtk_widget_show (button);


        /*--- Spinner ---*/
	box2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), box2, TRUE, TRUE, 0);
	label = gtk_label_new(_("Number of opponents:"));
	gtk_box_pack_start(GTK_BOX(box2), label, TRUE, TRUE, 0);
	gtk_widget_show(label);
        adj = gtk_adjustment_new(NumberOfComputers, 1, 5, 1, 5, 1);
	sentry = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 10, 0);
#ifdef GTK_UPDATE_SNAP_TO_TICKS
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(sentry),
					  GTK_UPDATE_ALWAYS |
					  GTK_UPDATE_SNAP_TO_TICKS);
#else
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(sentry),
					  GTK_UPDATE_ALWAYS );
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(sentry), TRUE);
#endif
        gtk_box_pack_start(GTK_BOX(box2), sentry, FALSE, TRUE, 0);
	gtk_widget_show(sentry);
	gtk_widget_show(box2);


	gtk_widget_show(box);
	gtk_widget_show(frame);


	/*--- OK/CANCEL into "box" ---*/
	box = gtk_hbox_new(TRUE, 5);
	gtk_box_pack_start(GTK_BOX(all_boxes), box, TRUE, TRUE, 0);
        button = gnome_stock_button(GNOME_STOCK_BUTTON_OK);
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(do_setup), NULL);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 5);
        gtk_widget_show(button);
        button = gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL);
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
                           (GtkSignalFunc)setupdialog_destroy,
			   (gpointer)1);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 5);
        gtk_widget_show(button);
        gtk_widget_show(box);
	
	gtk_widget_show(all_boxes);

	gtk_widget_show(setupdialog);
}
