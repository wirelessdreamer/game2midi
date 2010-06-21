/*
 *
 *	This Code is bassed off of the example provided with wiiuse
 *	
 *	wii2drums by Nathanael Anderson
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

/*
 *	These should be able to be set in the config file
 *	Note mappings
 *	which lights to turn on
 *	name for midi sequencer to system
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <cstdlib>

#ifndef WIN32
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#else
	#include <windows.h>
	#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds )
#endif

#include "RtMidi.h"
#include "wiiuse.h"
#include "ConfigFile.h" // from http://www-personal.umich.edu/~wagnerr/ConfigFile.html

#define MAX_WIIMOTES	1
#define VALUES 			1024
#define MAX_KITS		32

int init = 0;
int bb_threshold = 400;
int high_tracker = 180;
int tracker = 140;
int tracker_current = 140;
int sent = 0;
int tracker_low = 140;
int ir_tracker = 0;
int current_note = 30;
int midi_channel = 9;
int note = 40;
int tl_tracker = 0;
int tr_tracker = 0;
int bl_tracker = 0;
int br_tracker = 0;
int map_mode = 0;
int map[VALUES];
string kitnames[MAX_KITS];

string config_file = "wii2drum.cfg";

bool chooseMidiPort( RtMidiOut *rtmidi )
{
  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if ( nPorts == 0 ) {
    std::cout << "No output ports available!" << std::endl;
    return false;
  }

  if ( nPorts == 1 ) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  }
  else {
    for ( i=0; i<nPorts; i++ ) {
      portName = rtmidi->getPortName(i);
      std::cout << "  Output port #" << i << ": " << portName << '\n';
    }

    do {
      std::cout << "\nChoose a port number: ";
      std::cin >> i;
    } while ( i >= nPorts );
  }

  std::cout << "\n";
  rtmidi->openPort( i );

  return true;
}

int midi_close( RtMidiOut *midiout)
{
	
	delete midiout;
	return 0;
}

RtMidiOut midi_init(int controller_number)
{
	RtMidiOut *midiout = 0;
	try {
    	midiout = new RtMidiOut();
  	}
  	catch ( RtError &error ) {
    	error.printMessage();
    	exit( EXIT_FAILURE );
  	}

	try {
    	if ( chooseMidiPort( midiout ) == false ) { midi_close(midiout); }
  	}
  	catch ( RtError &error ) {
    	error.printMessage();
    	midi_close(midiout);
  	}
	return *midiout;
}

void load_config()
{
	int init_map, init_kit;
	int total_kits;
	ConfigFile config (config_file);
	if (config.readInto(total_kits, "total_kits")){
		if (total_kits > MAX_KITS){
			std::cout << "Please increase the value of MAX KITS, and recompile" << std::endl;
		}
	}

	// Read in kit names
	for (init_kit = 1; init_kit < total_kits; init_kit++){
		ostringstream lbuffer;
        lbuffer << init_kit;
        string tmpname = lbuffer.str();
        string kit_name = "kit" + tmpname;
		if (config.readInto(kit_names[init_kit], kit_name)){
		
	}
	
	std::cout << mapnames << std::endl;
	for (init_map = 0; init_map < 1024; init_map++)
	{
//	if (!(config.readInto(device_number, "0")));
//		map[init_map] = config_value;	
	}

}

int note_lookup(int total)
{

	return 40;
}
/*
int get_note(int total)
{
	char x[2];
	int num;
	
	while (true)
	{
		memset((void*)&x, 0, sizeof(char)*2);
		std::cout << "What note do you want to map " << total << " to ( 0-84 are valid ): ";
		std::cin.getline(x,3);
		num = atoi(x); 
		if (x[0] == '0'){
			map[total] = num;
			return 0;
		}
		if ((num <= 0) || (num > 84))
		{
			std::cout << "Enter only valid numbers (0 - 84)." << std::endl;
		}else{
			map[total] = num;
			return num;
		}
	}
}
*/
void midi_note_play( RtMidiOut *midiout, int note, int velocity, int channel)
{
	std::vector<unsigned char> message;
		
	message.push_back(0);
	message.push_back(0);
	message.push_back(0);

	std::cout << "Send note " << note << " with velocity " << velocity << " on channel " << channel << std::endl;

	message[0] = 144 + channel; // represents a note on event
	message[1] = note;
	message[2] = velocity;
	midiout->sendMessage( &message );

	message[0] = 128 + channel; // represents a note off event
	message[1] = note;
	message[2] = velocity;
	midiout->sendMessage( &message );
}

int find_velocity(int input)
{
	int calc_velocity = 150 - input;
	if (calc_velocity > 127) calc_velocity = 127;
	if (calc_velocity < 12) calc_velocity = 12;
	return calc_velocity;
}

void handle_event(struct wiimote_t* wm, RtMidiOut *midiout) 
{
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_A))		printf("A pressed\n");
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_B))
	{
		printf("B pressed\n");
		/*
  		note = 36;
		if (1 == ir_tracker) note +=3;
		midi_note_play(note,90, midi_channel);	
		*/
	}
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_UP))		printf("UP pressed\n");
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_DOWN))	printf("DOWN pressed\n");
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_LEFT))	printf("LEFT pressed\n");
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_RIGHT))	printf("RIGHT pressed\n");
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_MINUS))	printf("MINUS pressed\n");
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_PLUS))	printf("PLUS pressed\n");
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_ONE))	printf("ONE pressed\n");
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_TWO))	printf("TWO pressed\n");
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_HOME))	printf("HOME pressed\n");

//	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_B))
//		wiiuse_toggle_rumble(wm);

/*	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_UP))
		wiiuse_set_ir(wm, 1);
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_DOWN))
		wiiuse_set_ir(wm, 0);
*/
	/* if the accelerometer is turned on then print angles */
	if (WIIUSE_USING_ACC(wm)) {
		int total = 0;
		int note;
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_A)) 		total += 1;
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_B)) 		total += 2;
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_PLUS)) 	total += 4;
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_HOME))	total += 8;
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_MINUS))	total += 16;
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_UP))		total += 32;
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_DOWN))	total += 64;
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_LEFT))	total += 128;
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_RIGHT))	total += 256;
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_ONE))		total += 512;
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_TWO))		total += 1024;

		if (wm->accel.z	< tracker){
			if ((wm->accel.z < tracker_low) && (wm->accel.x > wm->accel.z)){
				tracker_low = wm->accel.z;
			}
			if ((wm->accel.z > tracker_low) && (0 == sent))
			{
				if (map_mode == 1)
				{
//					note = get_note(total);
					std::cout << "played note" << std::endl;
					sent = 1;
				}else{
					// if (1 == ir_tracker) note +=3;
					midi_note_play(midiout, note_lookup(total), find_velocity(tracker_low), midi_channel);
					sent = 1;
				}
			}
		}else{
			if (1 == sent){
				sent = 0;
				ir_tracker = 0;
				tracker_low = wm->accel.z;
			}		
		}
	}

	/*
	 *	If IR tracking is enabled then print the coordinates
	 *	on the virtual screen that the wiimote is pointing to.
	 *
	 *	Also make sure that we see at least 1 dot.
	 */
	if (WIIUSE_USING_IR(wm)) {
		int i = 0;

		/* go through each of the 4 possible IR sources */
		for (; i < 4; ++i) {
			/* check if the source is visible */
			if (wm->ir.dot[i].visible)
				ir_tracker = 1;
//				printf("IR source %i: (%u, %u)\n", i, wm->ir.dot[i].x, wm->ir.dot[i].y);
		}

//		printf("IR cursor: (%u, %u)\n", wm->ir.x, wm->ir.y);
//		printf("IR z distance: %f\n", wm->ir.z);
	}

	/* show events specific to supported expansions */
	if (wm->exp.type == EXP_NUNCHUK) {
		/* nunchuk */
		struct nunchuk_t* nc = (nunchuk_t*)&wm->exp.nunchuk;

		// this happens after the notes are sent, so it has no effect, fix this sometime
		if (IS_PRESSED(nc, NUNCHUK_BUTTON_C))		note += 2;
		if (IS_PRESSED(nc, NUNCHUK_BUTTON_Z))		note += 1;

		printf("nunchuk roll  = %f\n", nc->orient.roll);
		printf("nunchuk pitch = %f\n", nc->orient.pitch);
		printf("nunchuk yaw   = %f\n", nc->orient.yaw);

		printf("nunchuk joystick angle:     %f\n", nc->js.ang);
		printf("nunchuk joystick magnitude: %f\n", nc->js.mag);
	} else if (wm->exp.type == EXP_CLASSIC) {
		/* classic controller */
		struct classic_ctrl_t* cc = (classic_ctrl_t*)&wm->exp.classic;

		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_ZL))			printf("Classic: ZL pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_B))			printf("Classic: B pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_Y))			printf("Classic: Y pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_A))			printf("Classic: A pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_X))			printf("Classic: X pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_ZR))			printf("Classic: ZR pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_LEFT))		printf("Classic: LEFT pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_UP))			printf("Classic: UP pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_RIGHT))		printf("Classic: RIGHT pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_DOWN))		printf("Classic: DOWN pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_FULL_L))		printf("Classic: FULL L pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_MINUS))		printf("Classic: MINUS pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_HOME))		printf("Classic: HOME pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_PLUS))		printf("Classic: PLUS pressed\n");
		if (IS_PRESSED(cc, CLASSIC_CTRL_BUTTON_FULL_R))		printf("Classic: FULL R pressed\n");

		printf("classic L button pressed:         %f\n", cc->l_shoulder);
		printf("classic R button pressed:         %f\n", cc->r_shoulder);
		printf("classic left joystick angle:      %f\n", cc->ljs.ang);
		printf("classic left joystick magnitude:  %f\n", cc->ljs.mag);
		printf("classic right joystick angle:     %f\n", cc->rjs.ang);
		printf("classic right joystick magnitude: %f\n", cc->rjs.mag);
	} else if (wm->exp.type == EXP_GUITAR_HERO_3) {
		/* guitar hero 3 guitar */
		struct guitar_hero_3_t* gh3 = (guitar_hero_3_t*)&wm->exp.gh3;

		if (IS_PRESSED(gh3, GUITAR_HERO_3_BUTTON_STRUM_UP))		printf("Guitar: Strum Up pressed\n");
		if (IS_PRESSED(gh3, GUITAR_HERO_3_BUTTON_STRUM_DOWN))	printf("Guitar: Strum Down pressed\n");
		if (IS_PRESSED(gh3, GUITAR_HERO_3_BUTTON_YELLOW))		printf("Guitar: Yellow pressed\n");
		if (IS_PRESSED(gh3, GUITAR_HERO_3_BUTTON_GREEN))		printf("Guitar: Green pressed\n");
		if (IS_PRESSED(gh3, GUITAR_HERO_3_BUTTON_BLUE))			printf("Guitar: Blue pressed\n");
		if (IS_PRESSED(gh3, GUITAR_HERO_3_BUTTON_RED))			printf("Guitar: Red pressed\n");
		if (IS_PRESSED(gh3, GUITAR_HERO_3_BUTTON_ORANGE))		printf("Guitar: Orange pressed\n");
		if (IS_PRESSED(gh3, GUITAR_HERO_3_BUTTON_PLUS))			printf("Guitar: Plus pressed\n");
		if (IS_PRESSED(gh3, GUITAR_HERO_3_BUTTON_MINUS))		printf("Guitar: Minus pressed\n");

		printf("Guitar whammy bar:          %f\n", gh3->whammy_bar);
		printf("Guitar joystick angle:      %f\n", gh3->js.ang);
		printf("Guitar joystick magnitude:  %f\n", gh3->js.mag);
	} 

	if (wm->exp.type == EXP_WII_BOARD)
    {
		if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_A))
    	{
    	    printf("Balance Board Calibration Started\n");
    	    wiiuse_set_wii_board_calib(wm);
			printf("Balance Board Calibration Done\n");
    	}
		short ptr, ptl, pbr, pbl;
        struct wii_board_t* wb = (wii_board_t*)&wm->exp.wb;
        ptr = wb->tr;
        ptl = wb->tl;
        pbr = wb->br;
        pbl = wb->bl;
        printf("%u, %u, %u, %u, %f\n", ptl, ptr, pbl, pbr, ((ptr+ptl+pbr+pbl)/4*2.2046));
		if ((ptr > bb_threshold) && (0 == tr_tracker)){
			midi_note_play(midiout, (note - 6),100, midi_channel);
			tr_tracker = 1;
		}else{
			if (ptr < bb_threshold){
				tr_tracker = 0;
			}
		}
		if ((ptl > bb_threshold) && (0 == tl_tracker)){
			midi_note_play(midiout, (note - 1),100, midi_channel);
			tl_tracker = 1;
		}else{
			if (ptl < bb_threshold){
				tl_tracker = 0;
			}
		}
    }
}

void handle_read(struct wiimote_t* wm, byte* data, unsigned short len) {
	int i = 0;

	printf("\n\n--- DATA READ [wiimote id %i] ---\n", wm->unid);
	printf("finished read of size %i\n", len);
	for (; i < len; ++i) {
		if (!(i%16))
			printf("\n");
		printf("%x ", data[i]);
	}
	printf("\n\n");
}

void handle_ctrl_status(struct wiimote_t* wm) {
	printf("\n\n--- CONTROLLER STATUS [wiimote id %i] ---\n", wm->unid);

	printf("attachment:      %i\n", wm->exp.type);
	printf("speaker:         %i\n", WIIUSE_USING_SPEAKER(wm));
	printf("ir:              %i\n", WIIUSE_USING_IR(wm));
	printf("leds:            %i %i %i %i\n", WIIUSE_IS_LED_SET(wm, 1), WIIUSE_IS_LED_SET(wm, 2), WIIUSE_IS_LED_SET(wm, 3), WIIUSE_IS_LED_SET(wm, 4));
	printf("battery:         %f %%\n", wm->battery_level);
}

void handle_disconnect(wiimote* wm) {
	printf("\n\n--- DISCONNECTED [wiimote id %i] ---\n", wm->unid);
}

void test(struct wiimote_t* wm, byte* data, unsigned short len) {
	printf("test: %i [%x %x %x %x]\n", len, data[0], data[1], data[2], data[3]);
}

int main(int argc, char** argv) {

	int cloop;
	for (cloop = 0; cloop < VALUES; cloop++)
	{
		map[cloop] = -1;
	}

	if ((midi_channel < 0) || (midi_channel > 16))
	{
		printf("Invalid Midi Channel, set midi channel to 0");
		midi_channel = 0;
	}
	
	wiimote** wiimotes;
	int found, connected;

	wiimotes =  wiiuse_init(MAX_WIIMOTES);

	found = wiiuse_find(wiimotes, MAX_WIIMOTES, 5);
	if (!found) {
		printf ("No wiimotes found.");
		return 0;
	}

	connected = wiiuse_connect(wiimotes, MAX_WIIMOTES);
	if (connected)
		printf("Connected to %i wiimotes (of %i found).\n", connected, found);
	else {
		printf("Failed to connect to any wiimote.\n");
		return 0;
	}

	wiiuse_set_leds(wiimotes[0], WIIMOTE_LED_1);
	wiiuse_rumble(wiimotes[0], 1);

	#ifndef WIN32
		usleep(200000);
	#else
		Sleep(200);
	#endif

	wiiuse_rumble(wiimotes[0], 0);
	load_config();
	RtMidiOut midiout = midi_init(0);
	std::cout << "Midi started for wiimote" << std::endl;
	wiiuse_motion_sensing(wiimotes[0], 1);
	wiiuse_set_ir(wiimotes[0], 1);
	
	while (1) {
		if (wiiuse_poll(wiimotes, MAX_WIIMOTES)) {
			/*
			 *	This happens if something happened on any wiimote.
			 *	So go through each one and check if anything happened.
			 */
			int i = 0;
			for (; i < MAX_WIIMOTES; ++i) {
				switch (wiimotes[i]->event) {
					case WIIUSE_EVENT:
						/* a generic event occured */
						handle_event(wiimotes[i], &midiout);
						break;

					case WIIUSE_STATUS:
						/* a status event occured */
						handle_ctrl_status(wiimotes[i]);
						break;

					case WIIUSE_DISCONNECT:
					case WIIUSE_UNEXPECTED_DISCONNECT:
						/* the wiimote disconnected */
						handle_disconnect(wiimotes[i]);
						break;

					case WIIUSE_READ_DATA:
						/*
						 *	Data we requested to read was returned.
						 *	Take a look at wiimotes[i]->read_req
						 *	for the data.
						 */
						break;

					case WIIUSE_NUNCHUK_INSERTED:
						/*
						 *	a nunchuk was inserted
						 *	This is a good place to set any nunchuk specific
						 *	threshold values.  By default they are the same
						 *	as the wiimote.
						 */
						 //wiiuse_set_nunchuk_orient_threshold((struct nunchuk_t*)&wiimotes[i]->exp.nunchuk, 90.0f);
						 //wiiuse_set_nunchuk_accel_threshold((struct nunchuk_t*)&wiimotes[i]->exp.nunchuk, 100);
						printf("Nunchuk inserted.\n");
						break;

					case WIIUSE_CLASSIC_CTRL_INSERTED:
						printf("Classic controller inserted.\n");
						break;

					case WIIUSE_GUITAR_HERO_3_CTRL_INSERTED:
						/* some expansion was inserted */
						handle_ctrl_status(wiimotes[i]);
						printf("Guitar Hero 3 controller inserted.\n");
						break;

					case WIIUSE_WII_BOARD_CTRL_INSERTED:
						handle_ctrl_status(wiimotes[i]);
						printf("Balance Board controller inserted.\n");
						break;

					case WIIUSE_NUNCHUK_REMOVED:
					case WIIUSE_CLASSIC_CTRL_REMOVED:
					case WIIUSE_GUITAR_HERO_3_CTRL_REMOVED:
					case WIIUSE_WII_BOARD_CTRL_REMOVED:
						/* some expansion was removed */
						handle_ctrl_status(wiimotes[i]);
						printf("An expansion was removed.\n");
						break;

					default:
						break;
				}
			}
		}
	}

	wiiuse_cleanup(wiimotes, MAX_WIIMOTES);

	return 0;
}
