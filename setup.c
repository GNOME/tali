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

static gint setupdialog_destroy(GtkWidget *widget, gint mode);
static GtkWidget *setupdialog = NULL;
static GtkWidget *HumanSpinner, *ComputerSpinner;
static GtkObject *HumanAdj, *ComputerAdj;

static int OriginalNumberOfComputers = -1;
static int OriginalNumberOfHumans    = -1;

static void
parse_an_arg (poptContext ctx,
	      enum poptCallbackReason reason,
	      const struct poptOption *opt,
	      const char *arg, void *data)
{

	switch (opt->shortName)
	{
	case 'r':
                calc_random();
                GyahtzeeAbort = 1;
		break;
	default:
	}

	return;
}

static struct poptOption cb_options[] = {
  {NULL, '\0', POPT_ARG_CALLBACK, &parse_an_arg, 0},
  {NULL, 'r', POPT_ARG_NONE, &GyahtzeeAbort, 0, N_("Calculate random die throws (debug)"), NULL},
  {NULL, '\0', 0, NULL, 0}
};

const struct poptOption yahtzee_options[] = {
  {NULL, '\0', POPT_ARG_INCLUDE_TABLE, cb_options, 0, NULL, NULL},
  {NULL, 'd', POPT_ARG_NONE, &DoDelay, 0, N_("Delay computer moves"), NULL},
  {NULL, 's', POPT_ARG_NONE, &OnlyShowScores, 0, N_("Show high scores and exit"), NULL},
  {NULL, 't', POPT_ARG_NONE, &DisplayComputerThoughts, 0, N_("Display computer thoughts"), NULL},
  {NULL, 'n', POPT_ARG_INT, &NumberOfComputers, 0, N_("Number of computer opponents"), N_("NUMBER")},
  {NULL, 'p', POPT_ARG_INT, &NumberOfHumans, 0, N_("Number of human opponents"), N_("NUMBER")},
  {NULL, '\0', 0, NULL, 0}
};

static void
WarnNumPlayersChanged (void)
{
  GtkWidget *mb;

#if 0   /* Message box crashes my system!!!!!!!!!!!! */
  say(_("Current game will complete" 
	" with original number of players."));
#else
  mb = gnome_message_box_new (_("Current game will complete" 
				" with original number of players."),
                              GNOME_MESSAGE_BOX_INFO,
                              GNOME_STOCK_BUTTON_OK,
                              NULL);
  GTK_WINDOW(mb)->position = GTK_WIN_POS_MOUSE;
  gnome_dialog_set_modal(GNOME_DIALOG(mb));
  gtk_widget_show (mb);
#endif
}


static void
do_setup(GtkWidget *widget, gpointer data)
{
        NumberOfComputers = 
                gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ComputerSpinner));
        NumberOfHumans = 
                gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(HumanSpinner));

	setupdialog_destroy(setupdialog, 1);

	gnome_config_set_int("/gyahtzee/Preferences/NumberOfComputerOpponents",
                             NumberOfComputers);
	gnome_config_set_int("/gyahtzee/Preferences/NumberOfHumanOpponents",
                             NumberOfHumans);

	gnome_config_sync();

        if ( ( (NumberOfComputers!=OriginalNumberOfComputers)||
	       (NumberOfHumans!=OriginalNumberOfHumans) ) &&
	     !GameIsOver() )
                WarnNumPlayersChanged();
}

static gint
setupdialog_destroy(GtkWidget *widget, gint mode)
{
	if (mode == 1) {
		gtk_widget_destroy(setupdialog);
	}
	setupdialog = NULL;

	return FALSE;
}


static gint
set_delay (GtkWidget *widget, gpointer *data)
{
       DoDelay = GTK_TOGGLE_BUTTON (widget)->active;
       return FALSE;
}

static gint
set_thoughts (GtkWidget *widget, gpointer *data)
{
        DisplayComputerThoughts = GTK_TOGGLE_BUTTON (widget)->active;
	return FALSE;
}

static gint
MaxPlayersCheck (GtkWidget *widget, gpointer *data)
{
        int numc, numh;

        numc = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ComputerSpinner));
        numh = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(HumanSpinner));

        if ( (numc+numh) > MAX_NUMBER_OF_PLAYERS) {
                if (GTK_ADJUSTMENT(data)==GTK_ADJUSTMENT(HumanAdj)) {
                        gtk_adjustment_set_value(GTK_ADJUSTMENT(ComputerAdj),
                                                 (gfloat)(numc-1));
                } else {
                        gtk_adjustment_set_value(GTK_ADJUSTMENT(HumanAdj),
                                                 (gfloat)(numh-1));
                }
                        
        }

	return FALSE;
}

gint 
setup_game(GtkWidget *widget, gpointer data)
{
        GtkWidget *all_boxes;
	GtkWidget *box, *box2;
        GtkWidget *label;
	GtkWidget *button;
	GtkWidget *frame;

        if (setupdialog) 
                return FALSE;

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


        /*--- Spinner (number of computers) ---*/
        OriginalNumberOfComputers = NumberOfComputers;
	box2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), box2, TRUE, TRUE, 0);
	label = gtk_label_new(_("Number of opponents:"));
	gtk_box_pack_start(GTK_BOX(box2), label, TRUE, TRUE, 0);
	gtk_widget_show(label);
        ComputerAdj = gtk_adjustment_new((gfloat)NumberOfComputers, 
                                         0.0, 6.0, 1.0, 6.0, 1.0);
	ComputerSpinner = gtk_spin_button_new(GTK_ADJUSTMENT(ComputerAdj),
                                              10, 0);
#ifndef HAVE_GTK_SPIN_BUTTON_SET_SNAP_TO_TICKS
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(ComputerSpinner),
					  GTK_UPDATE_ALWAYS |
					  GTK_UPDATE_SNAP_TO_TICKS);
#else
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(ComputerSpinner),
					  GTK_UPDATE_ALWAYS );
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(ComputerSpinner),
					  TRUE);
#endif
	gtk_signal_connect (GTK_OBJECT(ComputerAdj), 
                            "value_changed", (GtkSignalFunc)MaxPlayersCheck,
			    ComputerAdj);
        gtk_box_pack_start(GTK_BOX(box2), ComputerSpinner, FALSE, TRUE, 0);
	gtk_widget_show(ComputerSpinner);
	gtk_widget_show(box2);

	gtk_widget_show(box);
	gtk_widget_show(frame);


	frame = gtk_frame_new(_("Human Players"));
	gtk_box_pack_start(GTK_BOX(all_boxes), frame, TRUE, TRUE, 0);
	
	box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), box);

        /*--- Spinner (number of humans) ---*/
        OriginalNumberOfHumans = NumberOfHumans;
	box2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), box2, TRUE, TRUE, 0);
	label = gtk_label_new(_("Number of players:"));
	gtk_box_pack_start(GTK_BOX(box2), label, TRUE, TRUE, 0);
	gtk_widget_show(label);
        HumanAdj = gtk_adjustment_new((gfloat)NumberOfHumans, 0.0,
                                      6.0, 1.0, 6.0, 1.0);
	HumanSpinner = gtk_spin_button_new(GTK_ADJUSTMENT(HumanAdj), 10, 0);
#ifndef HAVE_GTK_SPIN_BUTTON_SET_SNAP_TO_TICKS
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(HumanSpinner),
					  GTK_UPDATE_ALWAYS |
					  GTK_UPDATE_SNAP_TO_TICKS);
#else
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(HumanSpinner),
					  GTK_UPDATE_ALWAYS );
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(HumanSpinner),
					  TRUE);
#endif
	gtk_signal_connect (GTK_OBJECT(HumanAdj), 
                            "value_changed", (GtkSignalFunc)MaxPlayersCheck,
			    HumanAdj);
        gtk_box_pack_start(GTK_BOX(box2), HumanSpinner, FALSE, TRUE, 0);
	gtk_widget_show(HumanSpinner);
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

        return FALSE;
}



