/**
 * aseqjoy - Tiny Jostick -> MIDI Controller Tool
 * Copyright 2003 by Alexander Koenig - alex@lisas.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Note: that these sources contain a few lines of Vojtech Pavlik's jstest.c 
 * example, which is GPL'd, too and available from:
 * http://atrey.karlin.mff.cuni.cz/~vojtech/joystick/
 *
 * Button to midi note code added by wirelessdreamer gmail com
 */

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#include <linux/joystick.h>
#include <alsa/asoundlib.h>
#include <alsa/seqmid.h>

#define NAME_LENGTH 128

#define TOOL_NAME "aseqjoy"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

int joystick_no=0;

typedef struct {
	int controller;
	int last_value;
}  val;

snd_seq_t *seq_handle;
snd_seq_event_t ev;
int controllers[4] = {11,12,13,14}; // something should be changed with this i'm redundantly refering to the controllers being used
int button_mode = 0;
int button_channel = 9;
int button_velocity = 100;
int button_note_base = 36;
int button_state[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int holdnote[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int holdchannel[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int lastcontroller[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int verbose = 0;

	// turn this section into a loop to allow more then 2 analog sticks
int current_x[2] = {0,0};		// analog sticks need position tracked if we're going to be able assign virtual notes to their positions
int current_y[2] = {0,0};		// more track* variables and such would need to be added to support more the 2 analog sticks
// Zones are set up in the same area's as the normal D-pad, break the analog stick down into an X, 1 is up top, 2 on the left, 3 to the right, 4 on the bottom
	

int open_alsa_seq()
{
	char client_name[32];
	char port_name[48];
	snd_seq_addr_t src;
	
	/* Create the sequencer port. */
	
	sprintf(client_name, "Joystick%i", joystick_no);
	sprintf(port_name , "%s Output", client_name);

	if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
		puts("Error: Failed to access the ALSA sequencer.");
		exit(-1);
	}

	snd_seq_set_client_name(seq_handle, client_name);
	src.client = snd_seq_client_id(seq_handle);
	src.port = snd_seq_create_simple_port(seq_handle, "Joystick Output",
		SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_APPLICATION);

	/* Init the event structure */
	
	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_source(&ev, src.port);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);

	return 0;
}

int axes;
int joy_fd;
int buttons;

int open_joystick()
{

	char device[256];
	char name[NAME_LENGTH] = "Unknown";	
	
	sprintf(device, "/dev/input/js%i", joystick_no);

	if ((joy_fd = open(device, O_RDONLY)) < 0) {
		fprintf(stderr, "%s: ", TOOL_NAME); perror(device);
		sprintf(device, "/dev/js%i", joystick_no);
		
		if ((joy_fd = open(device, O_RDONLY)) < 0) {	
			fprintf(stderr, "%s: ", TOOL_NAME); perror(device);
			exit(-3);
		}
	}

	ioctl(joy_fd, JSIOCGAXES, &axes);
	ioctl(joy_fd, JSIOCGBUTTONS, &buttons);
	ioctl(joy_fd, JSIOCGNAME(NAME_LENGTH), name);

	printf("Using Joystick (%s) through device %s with %i axes and %i buttons.\n", name, device, axes, buttons);

	return 0;
}

void loop()
{
	struct js_event js;
	int current_channel=1;
	double val_d;
	int val_i;
	int i;
	val *values;
	int data[256][256];
	// these should be dynamically initilized
	int zone[axes/2]; 		// active zone on the first analog stick
	int lastzone[axes/2];		// last active zone on each analog stick
	int lastnote=0;
	int pick_in_use=0;
	values = calloc(axes, sizeof(val));
	
	puts("Axis -> MIDI controller mapping:");
	
	for (i=0; i<axes; i++) {
		zone[i]=0;
		lastzone[i]=0;
		if (i<4) {
			values[i].controller=controllers[i];
		} else {
			values[i].controller=10+i;
		}
		printf("  %2i -> %3i\n", i, values[i].controller);
		values[i].last_value=0;		
	}
	
	puts("Ready, entering loop - use Ctrl-C to exit.");	

	while (1) {
		if (read(joy_fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
			perror(TOOL_NAME ": error reading from joystick device");
			exit (-5);
		}

		switch(js.type & ~JS_EVENT_INIT) {		
			case JS_EVENT_BUTTON:
//				if (verbose){
//					printf("Current button mode is %i.\n",button_mode);
//				}
				if ((button_mode == 0) && (js.value)) {			
					current_channel=js.number+1;
					if (verbose) {
						printf("Switched to MIDI channel %i.\n", current_channel);
					}
				}
				if ((button_mode == 1) && (js.value)) {
						snd_seq_ev_set_noteon(&ev, button_channel, js.number+button_note_base, button_velocity);
						snd_seq_event_output_direct(seq_handle, &ev);
						if (verbose) {
							printf("Sent noteon %i on channel %i with velocity: %i .\n", js.number+button_note_base, button_channel, button_velocity);
						}
						snd_seq_ev_set_noteoff(&ev, button_channel, js.number+button_note_base, 0);
						snd_seq_event_output_direct(seq_handle, &ev);
						if (verbose) {
							printf("Sent noteoff %i on channel %i with velocity: %i .\n", js.number+button_note_base, button_channel, button_velocity);
						}
				}
				
				if (button_mode == 2) {
					int hold0 = 0;
					int hold1 = 1;
					if (js.value) { // when js value is present it means the button was pushed down, when it isn't present it means the button was released
						if (holdnote[hold0] == 0){ // i don't feel like trying to make it so more then 2 are remembered, dynamically linked lists would work, but this kind of logic wouldn't
							holdnote[hold0]=js.number+button_note_base;
							if (verbose){
								printf("Hold note 1 set to %i\n", holdnote[hold0]);
							}
						}
						if ((holdnote[hold0] !=0) && (holdnote[hold1] == 0) && (holdnote[hold0] != js.number+button_note_base)){
							holdnote[hold1]=js.number+button_note_base;
							if (verbose){
								printf("Hold note 2 set to %i\n", holdnote[1]);
							}
						}
						snd_seq_ev_set_noteon(&ev, button_channel, js.number+button_note_base, button_velocity);
						snd_seq_event_output_direct(seq_handle, &ev);
						if (verbose) {
							printf("Sent noteon %i on channel %i with velocity: %i .\n", js.number+button_note_base, button_channel, button_velocity);
						}
				   	}else{
						if (holdnote[hold0] == js.number+button_note_base){
							holdnote[hold0] = 0;
							if (verbose){
								printf("Hold note 1 unset\n");
							}
						}
						if (holdnote[hold1] == js.number+button_note_base){
							holdnote[hold1] = 0;
							if (verbose){
								printf("Hold note 2 unset\n");
							}
						}
						snd_seq_ev_set_noteoff(&ev, button_channel, js.number+button_note_base, 0);
						snd_seq_event_output_direct(seq_handle, &ev);
						if (verbose) {
							printf("Sent noteoff %i on channel %i with velocity: %i .\n", js.number+button_note_base, button_channel, button_velocity);
						}
					}
				}	
				if (button_mode == 3){
				// Hard coded ugly stuff, this shouldn't be like this
						int guitar_f1 = 5;
						int guitar_f2 = 1;
						int guitar_f3 = 0;
						int guitar_f4 = 2;
						int guitar_f5 = 3;
						int guitar_up = 12;
						int guitar_down = 14;
						int guitar_start = 9;
						int guitar_select = 8;
					// i'm doing 4 bits for button combinations, so here are all the combinations
						int guitar_loop, guitar_loop1;
						int gcount=5;	
						int mynote=0;
					if (js.value){
						if ((js.number == guitar_up) ||(js.number == guitar_down)) {
							printf("pick in use 1\n");
							pick_in_use=1;
						}
						if (js.number == guitar_f1){
							// this is going to be for mode switches
						}
						if (js.number == guitar_f2){
							// printf("saw 2\n");
							holdnote[1] = 1;
						}
						if (js.number == guitar_f3){
							// printf("saw 3\n");
							holdnote[2] = 2;
						}
						if (js.number == guitar_f4){
							// printf("saw 4\n");
							holdnote[3] = 4;
						}
						if (js.number == guitar_f5){
							// printf("saw 5\n");
							holdnote[4] = 8;
						}
			
					}else{ // !js.value
						if ((js.number == guitar_up) ||(js.number == guitar_down)) {
							printf("pick not in use 1\n");
							pick_in_use=0;
						if (js.number == guitar_f1){
							// this is going to me for mode switches
						}
						if (js.number == guitar_f2){
							holdnote[1] = 0;
						}
						if (js.number == guitar_f3){
							holdnote[2] = 0;
						}
						if (js.number == guitar_f4){
							holdnote[3] = 0;
						}
						if (js.number == guitar_f5){
							holdnote[4] = 0;
						}	
					}
					mynote = holdnote[1] + holdnote[2] + holdnote[3] + holdnote[4] + button_note_base;
					printf("last note: %i\tthis note %i\n", lastnote, mynote);
					if ((pick_in_use) && ((js.value) && (mynote != button_note_base)) ){
						// printf("mynote: %i\n",mynote);
						// printf("js.numer: %i\n", js.number);
						snd_seq_ev_set_noteon(&ev, button_channel, mynote, button_velocity);
						snd_seq_event_output_direct(seq_handle, &ev);
						if (verbose) {
							printf("Sent noteon %i on channel %i with velocity: %i .\n", mynote, button_channel, button_velocity);
						}
						lastnote = mynote;
					}else{
						//pick_in_use=0;
					}
					if (lastnote != mynote){
						if (pick_in_use == 1){
							printf("Pick in use\n");
							snd_seq_ev_set_noteoff(&ev, button_channel, lastnote, 0);
							snd_seq_event_output_direct(seq_handle, &ev);
							if (verbose) {
								printf("Sent noteoff %i on channel %i with velocity: %i .\n", lastnote, button_channel, button_velocity);
							}
							lastnote=mynote;
							snd_seq_ev_set_noteoff(&ev, button_channel, lastnote, 0);
							snd_seq_event_output_direct(seq_handle, &ev);
							if (verbose) {
								printf("Sent noteon %i on channel %i with velocity: %i .\n", mynote, button_channel, button_velocity);
							}	
						}else{
							if (lastnote != 0){
								printf("Pick Not in use\n");
								snd_seq_ev_set_noteoff(&ev, button_channel, lastnote, 0);
								snd_seq_event_output_direct(seq_handle, &ev);
								if (verbose) {
									printf("Sent noteoff %i on channel %i with velocity: %i .\n", lastnote, button_channel, button_velocity);
								}
							}
						}
					}
					}	
				} // button mode 3
			break;
			
			case JS_EVENT_AXIS:

			if (button_mode == 0){
				val_d=(double) js.value;
				val_d+=SHRT_MAX;
				val_d=val_d/((double) USHRT_MAX);
				val_d*=127.0;
			
				val_i=(int) val_d;
			
				if (values[js.number].last_value!=val_i) {
					values[js.number].last_value!=val_i; // why is this here?
					snd_seq_ev_set_controller(&ev, current_channel, values[js.number].controller, val_i);
					snd_seq_event_output_direct(seq_handle, &ev);
					
					if (verbose) {
						printf("Sent controller %i with value: %i.\n", values[js.number].controller, val_i);
					}
				}
			}
		
			if (button_mode == 1){
				val_d=(double) js.value;
				val_d+=SHRT_MAX;
                                val_d=val_d/((double) USHRT_MAX);
                                val_d*=127.0;

                                val_i=(int) val_d;
				int current_a=0;	

				int x_axis = 11;
				int y_axis = 12;
				int position = 0;
				int note[8] = {36,36,40,49,55,56,57,58};
				while (position <2){
				if (values[js.number].controller == x_axis){ // if this is a x axis 
						current_x[position] = val_i;
				}
				if (values[js.number].controller == y_axis){ // if this is a y axis
						current_y[position] = val_i;
				}
				// printf("Reading %i Controller 11:\tx: %i\tController: 12\ty:%i\n",values[js.number].controller,current_x[0],current_y[0]);
			    	if ((values[js.number].controller == x_axis) || (values[js.number].controller == y_axis)){
					if ( (current_y[position] == 0) && (current_x[position] >= 0) && (current_x[position] <= 126) && (values[js.number].controller == y_axis)){
						zone[position] = 1;
					} else if ((current_x[position] == 0) && (current_y[position] <= 126) && (current_y[position] >= 0) && (values[js.number].controller == x_axis)){
						zone[position] = 2;
					} else if ((current_x[position] == 126) && (current_y[position] >= 0) && (current_y[position] <= 126) && (values[js.number].controller == x_axis)){
						zone[position] = 3;
					} else if ((current_y[position] == 126) && (current_x[position] <= 126) && (current_x[position] >= 0) && (values[js.number].controller == y_axis)){
						zone[position] = 4;
					} else {
						zone[position] = 0;
					}
					if (zone[position] != 0){
						snd_seq_ev_set_noteon(&ev, button_channel, note[zone[position]-1], button_velocity);
						snd_seq_event_output_direct(seq_handle, &ev);
						if (verbose) {
							printf("Sent noteon %i on channel %i with velocity: %i .\n", note[zone[position]-1], button_channel, button_velocity);
						}
					}
					printf("joystick 1 quadrant %i\n", zone[position]);
				}	
				if ((lastzone[position] != zone[position]) && (holdnote[position] != 0) && (zone[position] != 0)){
					snd_seq_ev_set_noteon(&ev, button_channel, holdnote[position], button_velocity);
					snd_seq_event_output_direct(seq_handle, &ev);
					if (verbose) {
						printf("Sent noteon %i on channel %i with velocity: %i .\n", holdnote[position], button_channel, button_velocity);
					}
					snd_seq_ev_set_noteoff(&ev, button_channel, holdnote[position], 0);
					snd_seq_event_output_direct(seq_handle, &ev);
					if (verbose) {
						printf("Sent noteoff %i on channel %i with velocity: %i .\n", holdnote[position], button_channel, button_velocity);
					}
				} // if zone has changed 
				
				lastzone[position] = zone[position];
				position++;
				} // while position < 2
			break;
			
			} // if button_mode = 1 

			if (button_mode == 2){
				val_d=(double) js.value;
				val_d+=SHRT_MAX;
                                val_d=val_d/((double) USHRT_MAX);
                                val_d*=127.0;

                                val_i=(int) val_d;
				int current_a=0;	

				int x_axis[2] = {11,14};
				int y_axis[2] = {12,13};
				int position = 0;
			   while (position < 2){
				printf("x1: %i y1: %i\n", x_axis[position], y_axis[position]);

				if (values[js.number].controller == x_axis[position]){ // x axis 
						current_x[position] = val_i;
				}
				if (values[js.number].controller == y_axis[position]){ // y axis
						current_y[position] = val_i;
				}
				 printf("Reading %i Controller 11:\tx: %i\tController: 12\ty:%i\n",values[js.number].controller,current_x[0],current_y[0]);
			    	if ((values[js.number].controller == x_axis[position]) || (values[js.number].controller == y_axis[position])){
					if ( (current_y[position] == 0) && (current_x[position] >= 0) && (current_x[position] <= 126) && (values[js.number].controller == y_axis[position])){
						zone[position] = 1;
					} else if ((current_x[position] == 0) && (current_y[position] <= 126) && (current_y[position] >= 0) && (values[js.number].controller == x_axis[position])){
						zone[position] = 2;
					} else if ((current_x[position] == 126) && (current_y[position] >= 0) && (current_y[position] <= 126) && (values[js.number].controller == x_axis[position])){
						zone[position] = 3;
					} else if ((current_y[position] == 126) && (current_x[position] <= 126) && (current_x[position] >= 0) && (values[js.number].controller == y_axis[position])){
						zone[position] = 4;
					} else {
						zone[position] = 0;
					}
					// printf("joystick 1 quadrant %i\n", zone[0]);
				}	
				if ((lastzone[position] != zone[position]) && (holdnote[position] != 0) && (zone[position] != 0)){
					snd_seq_ev_set_noteon(&ev, button_channel, holdnote[position], button_velocity);
					snd_seq_event_output_direct(seq_handle, &ev);
					if (verbose) {
						printf("Sent noteon %i on channel %i with velocity: %i .\n", holdnote[position], button_channel, button_velocity);
					}
					snd_seq_ev_set_noteoff(&ev, button_channel, holdnote[position], 0);
					snd_seq_event_output_direct(seq_handle, &ev);
					if (verbose) {
						printf("Sent noteoff %i on channel %i with velocity: %i .\n", holdnote[position], button_channel, button_velocity);
					}
				} // if zone has changed 
				
				lastzone[position] = zone[position];
				position++;
			    }// while position < 2
			break;
			
			default:
				printf("Unknown event recieved %s\n", js.type);
			
			} // if button_mode = 2
		} // switch statement
	} // while loop
} // loop() function

int main (int argc, char **argv)
{
        fprintf(stderr, "%s Version %s - Copyright (C) 2003 by Alexander König\n",  TOOL_NAME, VERSION);
        fprintf(stderr, "%s comes with ABSOLUTELY NO WARRANTY - for details read the license.\n", TOOL_NAME);

	while (1) {
		int i=getopt(argc, argv, "vhd:1:2:3:4:c:l:n:b:");
		if (i==-1) break;
		
		switch (i) {
			case '?':
			case 'h':
				printf("usage: %s [-d joystick_no] [-v] [-b mode] [-c channel] [-l velocity] [-n base note] [-0 ctrl0] [-1 ctrl1] [-2 ctrl2] [-3 ctrl3]\n\n", TOOL_NAME);
				puts("\t-d Select the Joystick to use: 0..3");
				puts("\t-0 Select the Controller for Axis 0 (1-127).");
				puts("\t-1 Select the Controller for Axis 1 (1-127). Etc.");
				puts("\t-b Map joystick buttons to midi notes.");
				puts("\t   1 - when a button is pushed a note on is sent followed by a note off.");
				puts("\t   2 - when a button is pushed a note on is sent, when it is released");
				puts("\t       a note off is sent.");
				puts("\t   3 - Guitar mode, for Guitar hero type controllers.");
				puts("\t-c channel to send events on in button mode (9 is default) (0-15) .");
				puts("\t-l fixed velocity to send in button mode (100 is default) (0-127).");
				puts("\t-n base note to use for midi notes sent in button mode (60 is middle c) (0-84).");
				puts("\t-v Verbose mode.");
				exit(-2);
			break;
			
			case '0':
				controllers[0]=atoi(optarg);
			break;

			case '1':
				controllers[1]=atoi(optarg);
			break;

			case '2':
				controllers[2]=atoi(optarg);
			break;

			case '3':
				controllers[3]=atoi(optarg);
			break;
			
			case 'v':
				verbose=1;
			break;
			
			case 'd':
				joystick_no=atoi(optarg);
			break;

			case 'b':
				button_mode=atoi(optarg);
				printf("Button Mode %i Enabled.\n", button_mode);
			break;
			
			case 'l':
				button_velocity=atoi(optarg);
			break;

			case 'c':
				button_channel=atoi(optarg);
			break;

			case 'n':
				button_note_base=atoi(optarg);
			break;
		}
	}
	snd_seq_addr_t dest;

	open_joystick();
	open_alsa_seq();
	
	loop();

	return 0;
}
