 /***************************************************************************
 *   Copyright (C) 2007 by Nathanael Anderson                              *
 *   wirelessdreamer AT gmail DOT com                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

// Uinput code based off example in January 2007 Dashboard Issue by Mehul Patel
// http://www.einfochips.com/download/dash_jan_tip.pdf

#include <string>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <linux/joystick.h>
#include <math.h>
#include <sstream>
#include <map>
#include <errno.h>

#include "ConfigFile.h" // from http://www-personal.umich.edu/~wagnerr/ConfigFile.html
#include "joy2seq.h"
#include <alsa/asoundlib.h>
#include <alsa/seqmid.h>

/* Globals */

using namespace std;

int joy2seq::open_joystick()
{
    char device[256];
	char name[128];

        sprintf(device, "/dev/input/js%i", device_number);

        if (0 > (joy_fd = open(device, O_RDONLY))) 
	{
		cerr << " error opening device " << device << endl;
                sprintf(device, "/dev/js%i", device_number);

                if ((joy_fd = open(device, O_RDONLY)) < 0) 
		{
                       	cerr << "Error opening Device " << device << endl; 
                        exit(-3);
                }
        }

        if (0 > ioctl(joy_fd, JSIOCGAXES, &axes))
	{
		cerr << "Invalid Value from JSIOCGAXES" << endl;
	}
	if ( 0 > ioctl(joy_fd, JSIOCGBUTTONS, &controller_buttons))
	{
		cerr << "Invalid Value from JSIOCGBUTTONS" << endl;
	}
	if (controller_buttons > MAX_BUTTONS)
	{
		cerr << "controller_buttons is greater then " << MAX_BUTTONS << " This is likely an error, but if you need support for more buttons, recompile joy2seq with a higher value for MAX_BUTTONS" << endl;
	}
        if ((total_chorded_buttons + total_simple_buttons) > controller_buttons)
	{
		cerr << "More buttons (" << total_chorded_buttons << ") defined then controller supports (" << controller_buttons << ")" << endl;
	}
	if ( 0 > ioctl(joy_fd, JSIOCGNAME(128), name))
	{
		cerr << "Invalid Value from JSIOCGNAME" << endl;
	}

	device_name.assign(name);

	if (verbose)
	{
        	cout << "Using Joystick " << device_number << " ( " << device_name << ") through device " << device << " with " << total_axes << " axes and " << controller_buttons <<" chorded buttons." << endl;
	}

        return 0;
}

int joy2seq::open_alsa_seq()
{
        char client_name[32];
        char port_name[48];
        snd_seq_addr_t src;

        /* Create the sequencer port. */

        sprintf(client_name, "Joystick%i", device_number);
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

int joy2seq::read_config(map<string,int>  & chordmap)
{
/*	// string manipulation example code
		ostringstream lbuffer;
		lbuffer << mode_loop;
		string tmpname = lbuffer.str();
		string filename = "joy2seq-mode" + tmpname;
		ConfigFile config(filename);
*/

	ConfigFile config (config_file);
	
	if (!(config.readInto(device_number, "jsdev")))
	{
		cerr << "Invalid entry for jsdev" << endl;
	}
	
	if (!(config.readInto(total_modes, "total_modes")))
	{
		cerr << "Invalid code for total_modes" << endl;
	}
	
	if (verbose)
	{
		cout << "Using " << config_file << " for configuration information" << endl;
	}
	
		if (!(config.readInto(total_simple_buttons, "total_simple_buttons"))) // we don't care about how many buttons the controller provides, only about how many we have values defined for
	{
		cerr << "Invalid entry for total_simple_buttons" << endl;
	}else{
		if (verbose)
		{
			cout << total_simple_buttons << " simple buttons defined by the config file" << endl; 
		}
	}

	for (int mode_loop = 1; mode_loop <= total_simple_buttons; mode_loop++)
	{	
		ostringstream lbuffer;
		lbuffer << mode_loop;
		string button_name = "simple_b" + lbuffer.str();
		if (!(config.readInto(simple_values[mode_loop], button_name)))
		{
			cerr << "Invalid code entered for " << button_name << " of: " << simple_values[mode_loop] << endl;
		}else{
			if (debug)
			{
				if (1 == mode_loop)
				{
					cout << "Simple Button Values: B" << lbuffer.str() << ": " << simple_values[mode_loop];
				}else{
					cout << " B" << lbuffer.str() << ": " << simple_values[mode_loop];
				}
				if (mode_loop == total_simple_buttons)
				{
					cout << endl;
				}
			}
		}
	}

	for (int mode_loop = 1; mode_loop <= total_modes; mode_loop++)
	{	
		//lbuffer.str() = "";
		ostringstream lbuffer;
		lbuffer << mode_loop;
		if (debug)
		{
			cout << "Mode " << lbuffer.str() << endl;
		}
		for (int key_loop = 1; key_loop <= total_simple_buttons; key_loop++)
		{// position 0 isn't used on key loop
			ostringstream tbuffer;
			tbuffer << key_loop;
			string itemname = lbuffer.str() + "simple" + tbuffer.str();
			string readvalue = "";
			if (!(config.readInto(readvalue,itemname)))
			{
				cerr << "Invalid entry for value: " << itemname << endl;
			}
//			cout << "Line: " << readvalue << endl;
			int pos = readvalue.find(",");
			int channel = atoi(readvalue.substr(0,pos).c_str());
			string half = readvalue.substr(pos+1).c_str();
			int npos = half.find(",");
			int note = atoi(half.substr(0,npos).c_str());
			int velocity = atoi(half.substr(npos+1).c_str());
			cout << "Channel: " << channel << " Note: " << note << " Velocity: " << velocity << endl;
			simple_modes[mode_loop][key_loop][0] = channel;
			simple_modes[mode_loop][key_loop][1] = note;
			simple_modes[mode_loop][key_loop][2] = velocity;
			if ((verbose) && (readvalue != ""))
			{ // only read valid entries
				cout << "Adding " << readvalue << " to simple[" << mode_loop << "][" << key_loop << "] " << endl;
			}
		}
	}

	if (debug)
	{
		cout << "Done Loading simple config values" << endl;	
	}
	if (!(config.readInto(total_macros, "total_macros")))
	{
		cerr << "Invalid code for total_macros" << endl;
	}
	if (!(config.readInto(total_modifiers, "total_modifiers")))
	{
		cerr << "Invalid code for total_modifiers" << endl;
	}else{
		for(int loadmacro = 1; loadmacro <= total_modifiers; loadmacro++)
		{
			ostringstream mbuffer;
			mbuffer << loadmacro;
			string current_modifier_code = mbuffer.str() + "modifier";	
			string tmpstring = "";
			if (!(config.readInto(tmpstring, current_modifier_code)))
			{
				cerr << " Invalid code for " << current_modifier_code << endl;
			}else{
				int ukeyvalue = chordmap.find(tmpstring)->second;
				modifier[loadmacro] = ukeyvalue;
			}
		}
	}
	
	if (!(config.readInto(total_chorded_buttons, "total_chorded_buttons"))) // we don't care about how many buttons the controller provides, only about how many we have values defined for
	{
		cerr << "Invalid entry for total_chorded_buttons" << endl;
	}else{
		if (verbose)
		{
			cout << total_chorded_buttons << " chorded buttons defined by the config file" << endl; 
		}
	}

	string button_name;
	ostringstream buttonbuffer;
	for (int load_codes = 1; load_codes <= total_chorded_buttons; load_codes++)
	{
		button_name.empty();
		buttonbuffer.str("");
		buttonbuffer << load_codes;
		button_name = "chord_b" + buttonbuffer.str();
		if (!(config.readInto(chord_values[load_codes - 1], button_name)))
		{
			cerr << "Invalid code entered for " << button_name << " of: " << chord_values[load_codes] << endl;
		}
		if (debug)
			{
				if (1 == load_codes)
				{
					cout << "Chord Button Values: B" << buttonbuffer.str() << ": " << chord_values[load_codes];
				}else{
					cout << " B" << buttonbuffer.str() << ": " << chord_values[load_codes];
				}
				if (load_codes == total_chorded_buttons)
				{
					cout << endl;
				}
			}
		}
	
	if (debug)
	{
		cout << "Starting to load chorded config values from 0 to " << pow(2,total_chorded_buttons) << "(2^" << total_chorded_buttons << ")" << endl;
	}
	for (int mode_loop = 1; mode_loop <= total_modes; mode_loop++)
	{	
		ostringstream lbuffer;
		lbuffer << mode_loop;
		string current_mode_code = lbuffer.str() + "modecode";
		if (!(config.readInto(mode_code[mode_loop], current_mode_code )))
		{ // code used for changing modes
			cerr << "Invalid code for " << current_mode_code << endl;
		}
		if (debug)
		{
			cout << "Adding Mode " << mode_code[mode_loop] << " into " << current_mode_code << endl;
		}
		for (int key_loop = 1; key_loop < (pow(2,total_chorded_buttons)); key_loop++)
		{// position 0 isn't used on key loop
			ostringstream tbuffer;
			tbuffer << key_loop;
			string itemname = lbuffer.str() + "chord" + tbuffer.str();
			string readvalue = "";
			if (!(config.readInto(readvalue,itemname)))
			{
				cerr << "Invalid entry for value: " << itemname << endl;
			}
			int ukeyvalue = chordmap.find(readvalue)->second;
			modes[mode_loop][key_loop] = ukeyvalue;
			if ((debug) && (readvalue != ""))
			{ // only read valid entries
				cout << "Adding " << readvalue << "[" << ukeyvalue << "]" << " to chorded[" << mode_loop << "][" << key_loop << "] " << endl;
			}
		}
	}
	if (debug)
	{
		cout << "Done Loading chorded config values" << endl;	
	}
}

void joy2seq::send_click_events() //not implemented yet
{

}

void joy2seq::send_note_down(int channel, int note, int velocity)
{
	cout << "Note on event: Channel: " << channel << " Note: " << note << " Velocity: " << velocity << endl;
//	snd_seq_ev_set_noteon(&ev, channel, note, velocity);
	snd_seq_ev_set_noteon(&ev, channel, note, 100);
	snd_seq_event_output_direct(seq_handle, &ev);
}

void joy2seq::send_note_up(int channel, int note)
{
	snd_seq_ev_set_noteoff(&ev, channel, note, 0);
	snd_seq_event_output_direct(seq_handle, &ev);
}

int joy2seq::valid_note( string newkey){
	if (newkey.length() > 3)
	{
		string tempkey,t1,t2,t3;
		t1 = newkey[0];
		t2 = newkey[1];
		t3 = newkey[2];
		tempkey = t1 + t2 + t3;
		if (tempkey == "key")
		{
			newkey.erase(0,3);
			istringstream buffer(newkey);
			int keyvalue;
			buffer >> keyvalue;
			return keyvalue;
		}
	}
	return -1;
}

void joy2seq::main_loop(map<string,int> chordmap)
{
	struct js_event js;
	
	// make an initilization function for these?

	for (int cleararray = 0; cleararray <= MAX_BUTTONS; cleararray++){
		button_state[cleararray] = 0;
	}
	for (int cleararray = 0; cleararray <= MAX_BUTTONS; cleararray++){
		send_code[cleararray] = 0;
	}	

	for (int cleararray = 0; cleararray <= MAX_MODES; cleararray++){
		modecount[cleararray] = 0;
	}
	
	lastnote = KEY_RESERVED; // initilize the last key to nothing
	thisnote = KEY_RESERVED; // for tracking the current key

	justpressed = 0;

	while (1){
		if (read(joy_fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) 
		{
			perror(TOOL_NAME ": error reading from joystick device");
			exit (-5);
		}
		process_events(js);
	}//while
}

void joy2seq::process_events(js_event js)
{
	switch(js.type & ~JS_EVENT_INIT) 
	{
               	case JS_EVENT_BUTTON:
			if (js.value) 
			{ 
				if (verbose)
				{
					printf("Pressed: %i\n",js.number + 1);
				}
				// if a button is pressed down remember its state until all buttons are released
				for ( int allbuttons = 0; allbuttons < total_chorded_buttons; allbuttons++)
				{
					if( js.number == chord_values[allbuttons])
					{
						
						button_state[allbuttons] = 1;
						send_code[allbuttons] = 1;
					}
				}		
				for (int allsimple = 1; allsimple <= total_simple_buttons; allsimple++)
				{
						printf("Data: %i,%i,%i\n", simple_modes[mode][allsimple][0],simple_modes[mode][allsimple][1], simple_modes[mode][allsimple][2]);
					if (simple_values[allsimple] == js.number)
					{
						send_note_down(simple_modes[mode][allsimple][0],simple_modes[mode][allsimple][1], simple_modes[mode][allsimple][2]);
					}
				}
			}else{ // track when buttons are released
				for ( int allbuttons = 0; allbuttons < total_chorded_buttons; allbuttons++)
				{
		
					{
						button_state[allbuttons] = 0;
					}
				}
				for (int allsimple = 1; allsimple <= total_simple_buttons; allsimple++)
				{
					if (simple_values[allsimple] == js.number)
					{
						send_note_up(simple_modes[mode][allsimple][0],simple_modes[mode][allsimple][1]);
					}
				}
			}
			// sanity checker, to make sure no bad data get through
			for (int allbuttons = 0; allbuttons < total_chorded_buttons; allbuttons++)
			{
				if (send_code[allbuttons] > MAX_BAD)
				{
					// this is a workaround for when bad values are input
					send_code[allbuttons] = 0;
				}
			}
			
			if (debug)
			{
				cout << "Button State: Chord: ";
				for (int allbuttons = 0; allbuttons < total_chorded_buttons; allbuttons++){
					cout << allbuttons << ":" << send_code[allbuttons] << " ";
				}
				cout << endl;
			}
                        // this is the "key code" that is going to be sent
			button_code = 0;
			for (int allbuttons = 0; allbuttons < total_chorded_buttons; allbuttons++)
			{
				if (allbuttons == 0 )
				{
					button_code += send_code[allbuttons];
				}
				else
				{
					button_code += (send_code[allbuttons] * pow(2,allbuttons));
				}
			}
			thisnote = modes[mode][button_code];	
			int clear = 0;
			if (0 == button_code)
			{
				clear++;
			}
			for ( int allbuttons = 0; allbuttons < total_chorded_buttons; allbuttons++)
			{
				if (button_state[allbuttons] != 0)
				{
					clear++;
				}
			}
			if (clear == 0)
			{ // if all buttons are released then send the code and clear everything
				for (int allbuttons = 0; allbuttons < total_chorded_buttons; allbuttons++)
				{
					send_code[allbuttons] = 0;
				}
				for (int macro_loop = 0; macro_loop < total_macros; macro_loop++)
				{
					if (button_code == macro_values[macro_loop])
					{ // use a seprate loop for macros and modes
						if (verbose)
						{
							cout << "Macro " << macro_loop  << endl;
						}
						// mode = 2;
						// mode2count++;
						for (int cleararray = 0; cleararray <= MAX_MODES; cleararray++)
						{
							modecount[cleararray] = 0;
						}
						/* mode3count = 0;
						if (mode2count == 2)
						{
							send_note_down(KEY_LEFTCTRL);
							send_note_down(KEY_LEFTALT);
							send_note_down(KEY_S);
							send_note_up(KEY_S);
							mode = 3;
						}
						if (mode2count == 3)
						{
							send_note_up(KEY_LEFTALT);
							send_note_up(KEY_LEFTCTRL);
							mode2count = 0;
							mode = 1;
						}	*/
					}
				}				
				for (int mode_loop = 0; mode_loop < MAX_MODES; mode_loop++)
				{
					if( mode_code[mode_loop] == button_code)
					{
						if (verbose)
						{
							cout << "Mode changed to " << mode_loop << endl;
						}
						mode = mode_loop;
					}
				}
				justpressed = 0;
				int senddown = 0;
				for (int allbuttons = 0; allbuttons < total_modes; allbuttons++)
				{
					// if no mode button is pressed down, and key is not defined as KEY_RESERVED
					if(( button_code == mode_code[allbuttons]) || (button_code == KEY_RESERVED ))
					{
						senddown++;
					}
				}
				if (0 == senddown)
				{
					if (debug)
					{
						cout << "Sending Mode[" << mode << "] Down Code: " <<  button_code << endl;
					}
//					send_note_down(simple_modes[mode][allsimple][0],simple_modes[mode][allsimple][1],simple_modes[mode][allsimple][2]);
					lastnote= thisnote;
				}
					for (int mbuttons = 1; mbuttons <= total_modifiers; mbuttons++)
				{
					if (thisnote == modifier[mbuttons])
					{
						modifier_state[mbuttons] = 1;
						justpressed = 1;
						if (debug)
						{
							cout << "Set Modifier State " << mbuttons << endl;
						}
					}
				}
				int sendup = 0;
				for (int allbuttons = 0; allbuttons < total_modes; allbuttons++)
				{
					// if no mode button is pressed down, and key is not defined as KEY_RESERVED, and button was just pressed
					if((( button_code == mode_code[allbuttons]) || (button_code == KEY_RESERVED )) && (0 == justpressed))
					{
						sendup++;
					}
					if (1 == justpressed) 
					{
						sendup++;
					}
				}
				if (0 == sendup)
				{
					if (debug)
					{
						cout << "Sending Mode[" << mode << "] Up Code: " <<  button_code << endl;
					}
//					send_note_up(simple_modes[mode][allsimple][0],simple_modes[mode][allsimple][1]);	
					
					for (int mbuttons = 1; mbuttons <= total_modifiers; mbuttons++)
					{
						if (1 == modifier_state[mbuttons])
						{
							if (debug)
							{
								cout << "Cleared Modifier State " << mbuttons << endl;
							}
							modifier_state[mbuttons] = 0;
//							send_note_up(simple_modes[mode][allsimple][0],simple_modes[mode][allsimple][1]);
						}
					}
				}	

				int clearl;
				for (clearl=0; clearl < total_chorded_buttons; clearl++){
					send_code[clearl] = 0;
				}
			}
                        break;
	}
}

int main( int argc, char *argv[])
{
	int c;
	extern char *optarg;
	
	int init_device_number = 0;
	string init_config_file = "joy2seq-config";
	int setverbose = 0;
	int setdebug = 0;
	int setcalibration = 0;
	
	while ((c = getopt(argc, argv, "hvdbc:j:")) != -1){
		switch (c){
			case 'h':
				cout << "Useage: " << TOOL_NAME << " -d -v -b -c [keymap_file] -j [joystick_number]" << endl;
				cout << "	-d Enable Debug output" << endl;
				cout << "	-v Enable Verbose output" << endl;
				cout << "	-b Enable Calibration output" << endl;
				cout << "	-c Specify a keymap file to use" << endl;
				cout << "	-j Specify the joystick number to use" << endl;
				exit(-2);
				break;
			case 'd':
				cout << "debug messages enabled" << endl;
				setdebug = 1; 
				break;
			case 'v':
				cout << "verbose messages enabled" << endl;
				setverbose = 1; 
				break;
			case 'c':
				init_config_file = optarg; 
				break;			
			case 'b':
				setcalibration = 1;
				break;
			case 'j':
				init_device_number = atoi (optarg);
				cout << "joystick number set to " << init_device_number << endl;
				break;
		}
	}

	map<string,int> chordmap;
	
	joy2seq myjoy;

	// these should happen in the constructor
	myjoy.total_modes = 0;
	myjoy.config_file = init_config_file;
	myjoy.axes = 0;
	myjoy.total_simple_buttons = 0;
	myjoy.total_chorded_buttons = 0;
	myjoy.mode = 1;
	myjoy.calibration = setcalibration;
	myjoy.debug = setdebug;
	myjoy.verbose = setverbose;
	for (int allmodifier = 0; allmodifier < MAX_MODIFIERS; allmodifier++)
	{
		myjoy.modifier[allmodifier] = 0;
	}
        
	myjoy.read_config(chordmap);
	myjoy.device_number = init_device_number; // once the device number is pulled from the config file, store it with the other information in the class
	
	for (int init=0; init <= MAX_BUTTONS; init++){
		myjoy.button_state[init] = 0;
		myjoy.holdnote[init] = 0;
		myjoy.holdchannel[init] = 0;
		myjoy.lastcontroller[init] = 0;
	}
	myjoy.open_joystick();
	myjoy.open_alsa_seq();
	myjoy.main_loop(chordmap);
}
