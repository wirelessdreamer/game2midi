/*****************************************
g2ghpro.cpp

Midi Guitar to Rock Band Pro Guitar converter

by Nathanael Anderson

wirelessdreamer @t gmai<L> d.t com

*****************************************/

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "RtMidi.h"

RtMidiOut *midiout;
bool chooseMidiOutPort( RtMidiOut *rtmidi );
bool chooseMidiInPort( RtMidiIn *rtmidi );
int mode = 0;

void usage( void ) {
	// Error function in case of incorrect command-line
	// argument specifications.
	std::cout << "\nPut a useage message here that is useful\n";
	exit( 0 );
}

void mycallback( double deltatime, std::vector< unsigned char > *message, void *userData)
{
	int note = (int)message->at(1);
	int velocity = (int)message->at(2);
	int type = (int)message->at(0);
	int channel = -1;

	/* Note on and off messages have this type of format in decimal
	 * Part     1  2  3
	 * Sample  144 40 67
	 *
	 * Part 1 - event type (See the midi specification for more details http://www.midi.org/techspecs/midimessages.php)
	 * 144-159 are note on events for channel 0 (144) - channel 16 (159)
	 *
	 * Part 2 - Note number
	 *
	 * Part 3 - Velocity
	*/
	if ((type >= 144) && (type <= 159)){ // Note On Event
		std::cout << "Note: " << note << std::endl;
		channel = type - 144; // based on the note type we figure out the channel, as mentioned above
	}else if ((type >= 128) && (type <= 143)){ // Note Off Event
		channel = type - 128;
	}
}

int main( int argc, char *argv[] )
{
	extern char *optarg;
	int c;
	// example passing option -m NUMBER
	while ((c = getopt(argc, argv, "m:")) != -1){
		switch (c){
			case 'm': 
				mode = atoi(optarg);
				if (mode == 0){
					std::cout << "Set mode to 6 string guitar" << std::endl;
				}else if (mode == 1){
					std::cout << "Set mode to 5 string Bass, ignoring the top string" << std::endl;
				}else if (mode == 2){
					std::cout << "Set mode to 6 string Bass, ignoring the top two strings" << std::endl;
				}
				break;
		}
	}

	RtMidiIn *midiin = 0;


	// RtMidiIn constructor
	try {
		midiin = new RtMidiIn();
	}
	catch ( RtError &error ) {
		error.printMessage();
		exit( EXIT_FAILURE );
	}

	// Check available ports vs. specified.
	// RtMidiOut constructor
	try {
		midiout = new RtMidiOut();
	}
	catch ( RtError &error ) {
		error.printMessage();
		exit( EXIT_FAILURE );
	}
	// Call functions to select port.
	try {
		if ( chooseMidiInPort( midiin ) == false ) goto cleanup;
	}
	catch ( RtError &error ) {
		error.printMessage();
		goto cleanup;
	}
	try {
		if ( chooseMidiOutPort( midiout ) == false ) goto cleanup;
	}
	catch ( RtError &error ) {
		error.printMessage();
		goto cleanup;
	}
	
	// Set our callback function.  This should be done immediately after
	// opening the port to avoid having incoming messages written to the
	// queue instead of sent to the callback function.
	midiin->setCallback( &mycallback );

	// Don't ignore sysex, Ignore timing, and active sensing messages
	// I was getting crashes with these activated, and a roland vg-99 connected
	midiin->ignoreTypes( false, true, true);

	std::cout << "\nProcessing Midi Messages ... press <enter> to quit.\n";
	char input;
	std::cin.get(input);
	std::cin.get(input);

	// Clean up
cleanup:
	delete midiin;
	delete midiout;

	return 0;
}

bool chooseMidiInPort( RtMidiIn *rtmidiin )
{
	std::string portName;
	unsigned int i = 0, nPorts = rtmidiin->getPortCount();
	if ( nPorts == 0 ) {
		std::cout << "No output ports available!" << std::endl;
		return false;
	}

	if ( nPorts == 1 ) {
		std::cout << "\nOpening " << rtmidiin->getPortName() << std::endl;
	}
	else {
		for ( i=0; i<nPorts; i++ ) {
			portName = rtmidiin->getPortName(i);
			std::cout << "  Input port #" << i << ": " << portName << '\n';
		}

		do {
			std::cout << "\nChoose input port number: ";
			std::cin >> i;
		} while ( i >= nPorts );
	}

	std::cout << "\n";
	rtmidiin->openPort( i );

	return true;
}


bool chooseMidiOutPort( RtMidiOut *rtmidi )
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
			std::cout << "\nChoose output port number: ";
			std::cin >> i;
		} while ( i >= nPorts );
	}

	std::cout << "\n";
	rtmidi->openPort( i );

	return true;
}


