 /**************************************************************************
 *   Copyright (C) 2007-2010 by Nathanael Anderson                         *
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
 **************************************************************************/

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
#include "game2midi.h"
#include <alsa/asoundlib.h>
#include <alsa/seqmid.h>

/* Globals */

using namespace std;

int game2midi::open_joystick()
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
		cerr << "controller_buttons is greater then " << MAX_BUTTONS << " This is likely an error, but if you need support for more buttons, recompile game2midi with a higher value for MAX_BUTTONS" << endl;
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

int game2midi::open_alsa_seq()
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

int game2midi::read_config()
{
/*	// string manipulation example code
		ostringstream lbuffer;
		lbuffer << mode_loop;
		string tmpname = lbuffer.str();
		string filename = "game2midi-mode" + tmpname;
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
			simple[mode_loop][key_loop].channel = atoi(readvalue.substr(0,pos).c_str());
			string half = readvalue.substr(pos+1).c_str();
			int npos = half.find(",");
			simple[mode_loop][key_loop].note = atoi(half.substr(0,npos).c_str());
			simple[mode_loop][key_loop].velocity = atoi(half.substr(npos+1,half.length()).c_str());
			if (debug){
				cout << "Channel: " << simple[mode_loop][key_loop].channel << " Note: " << simple[mode_loop][key_loop].note << " Velocity: " << simple[mode_loop][key_loop].velocity << endl;
			}
			if ((verbose) && (readvalue != ""))
			{ // only read valid entries
				cout 	<< "Added " << "Channel: " << simple[mode_loop][key_loop].channel
					<< " Note: " <<  simple[mode_loop][key_loop].note
					<< " Velocity: " << simple[mode_loop][key_loop].velocity << endl;
			}
		}
	}

	if (debug)
	{
		cout << "Done Loading simple config values" << endl;	
	}
}

void game2midi::send_note_down(int channel, int note, int velocity)
{
	cout << "Note on event: Channel: " << channel << " Note: " << note << " Velocity: " << velocity << endl;
	snd_seq_ev_set_noteon(&ev, channel, note, velocity);
	snd_seq_event_output_direct(seq_handle, &ev);
}

void game2midi::send_note_up(int channel, int note)
{
	cout << "Note off event: Channel: " << channel << " Note: " << note << endl;
	snd_seq_ev_set_noteoff(&ev, channel, note, 0);
	snd_seq_event_output_direct(seq_handle, &ev);
}

int game2midi::valid_note( string newkey){
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

void game2midi::main_loop()
{
	struct js_event js;
	
	for (int cleararray = 0; cleararray <= MAX_MODES; cleararray++){
		modecount[cleararray] = 0;
	}
	
	while (1){
		if (read(joy_fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) 
		{
			perror(TOOL_NAME ": error reading from joystick device");
			exit (-5);
		}
		process_events(js);
	}//while
}

void game2midi::process_note_button(int on, int number){

}

void game2midi::process_simple_button(int on, int number){
	if (on) 
	{ // button was pressed 
		if (debug)
		{
			cout << "Pressed: " << number + 1 << endl;
		}
		for (int allsimple = 1; allsimple <= total_simple_buttons; allsimple++)
		{
			if (simple_values[allsimple] == number)
			{
				if (verbose){
					cout << "Data: " << simple[mode][allsimple].channel << "," << simple[mode][allsimple].note << "," << simple[mode][allsimple].velocity << endl;
				}
				send_note_down(simple[mode][allsimple].channel,simple[mode][allsimple].note, simple[mode][allsimple].velocity);
			}
		}
	}else{ // button was released
		if (debug)
		{
			cout << "Released: " << number + 1 << endl;
		}
		for (int allsimple = 1; allsimple <= total_simple_buttons; allsimple++)
		{
			if (simple_values[allsimple] == number)
			{
				send_note_up(simple[mode][allsimple].channel,simple[mode][allsimple].note);
			}
		}
	}

}

void game2midi::process_events(js_event js)
{
	switch(js.type & ~JS_EVENT_INIT) 
	{
               	case JS_EVENT_BUTTON:
			if (mode == 1){
				process_simple_button(js.value, js.number);
			}else if (mode == 2){
				process_note_button(js.value, js.number);
			}
                        break;
	}
}

int main( int argc, char *argv[])
{
	int c;
	extern char *optarg;
	string setconfig = "game2midi-config";
	int setdevice = 0;
	bool setverbose = 0;
	bool setdebug = 0;
	
	while ((c = getopt(argc, argv, "hdvc:j:")) != -1)
        {
          switch (c)
                {
                case 'h':
                  cout << "Usage: " << TOOL_NAME << " -d -v -c [keymap_file] -j [joystick_number]" << endl;
                  cout << "     -d Enable Debug output" << endl;
                  cout << "     -v Enable Verbose output" << endl;
                  cout << "     -c Specify a keymap file to use" << endl;
                  cout << "     -j Specify the joystick number to use" << endl;
                  exit(-2);
                  break;
                case 'd':
                  cout << "Debug messages enabled" << endl;
                  setdebug = 1;
                  break;
                case 'v':
                  cout << "Verbose messages enabled" << endl;
                  setverbose = 1;
                  break;
                case 'c':
                  setconfig = optarg;
                  break;
                case 'j':
                  setdevice = atoi (optarg);
                  cout << "Joystick number set to " << setdevice << endl;
                  break;
                }
        }	

	game2midi myjoy(setconfig, setdevice, setverbose, setdebug);
	for (int init=0; init <= MAX_BUTTONS; init++){
		myjoy.button_state[init] = 0;
		myjoy.holdnote[init] = 0;
		myjoy.holdchannel[init] = 0;
		myjoy.lastcontroller[init] = 0;
	}
	myjoy.main_loop();
}
