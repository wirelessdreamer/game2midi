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
	std::cout << "\nuseage: g2ghpro <-m MODE>\n";
	std::cout << "    Mode Values:\n";
	std::cout << "    0 = electric guitar\n";
	std::cout << "    1 = 4 string bass\n";
	std::cout << "    2 = 5 string bass\n";
	std::cout << "    3 = 6 string bass\n";
	exit( 0 );
}

void mycallback( double deltatime, std::vector< unsigned char > *message, void *userData)
{
	int base[6]; // these are the notes the 6 strings have when played open, based on channel
	if (mode == 0){ // guitar mode
		base[5] = 40; // so channel 5's (low e) midi note number is 40
		base[4] = 45; // channel 4 - A midi note is 45
		base[3] = 50; // channel 3 - D midi note is 45
		base[2] = 55; // G
		base[1] = 59; // B
		base[0] = 64; // E (high)
	}else if(mode == 1){ // 4 string bass mode
		base[3] = 40; // so channel 5's (low e) midi note number is 40
		base[2] = 45; // channel 4 - A midi note is 45
		base[1] = 50; // channel 3 - D midi note is 45
		base[0] = 55; // G
	}else if(mode == 2){ // 5 string bass mode
		base[4] = 40; // so channel 5's (low e) midi note number is 40
		base[3] = 45; // channel 4 - A midi note is 45
		base[2] = 50; // channel 3 - D midi note is 45
		base[1] = 55; // G
	}else if(mode == 3){ // 6 string bass mode
		base[5] = 40; // so channel 5's (low e) midi note number is 40
		base[4] = 45; // channel 4 - A midi note is 45
		base[3] = 50; // channel 3 - D midi note is 45
		base[2] = 55; // G
	}

	std::vector<unsigned char> oMessage;
	std::vector<unsigned char> o1Message;
	std::vector<unsigned char> nMessage;
	int note = (int)message->at(1);
	int velocity = (int)message->at(2);
	velocity = 110;
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

/* I don't think this was really needed - TODO still need to test this
  		std::vector<unsigned char> XsysExMessage;
		XsysExMessage.push_back( 240);
		XsysExMessage.push_back( 8 );
		XsysExMessage.push_back( 64 );
		XsysExMessage.push_back( 10  );
		XsysExMessage.push_back(1);
		XsysExMessage.push_back( channel + 1);
		XsysExMessage.push_back( velocity );
		XsysExMessage.push_back( 247 );
		midiout->sendMessage( &XsysExMessage );
*/

		/* Rock Band 3 Pro guitar SysEx Message Format
		 *
		 * Part    1  2  3  4 5 6  7  8
		 * Sample 240 8 64 10 1 1 43 247
		 *
		 * Part 1 - Starting byte of a SysEx Message (always 240)
		 * Part 2,3,4 - Identifiers that this is a SysEx message used for Pro Guitar 
		 * Part 5 - Message type (1 = set fret position, 5 = play string)
		 * Part 6 - Midi Channel
		 * Part 7 - Midi Note
		 * Part 8 - End SysEx Message (always 247)
		*/

		std::vector<unsigned char> sysExMessage;
		sysExMessage.push_back( 240);
		sysExMessage.push_back( 8 );
		sysExMessage.push_back( 64 ); 
		sysExMessage.push_back( 10  );
		sysExMessage.push_back( 1); // 1 sets fret position
		sysExMessage.push_back( channel + 1 ); // channel
		sysExMessage.push_back( note );
		sysExMessage.push_back( 247 );
		midiout->sendMessage( &sysExMessage );

		std::vector<unsigned char> sysExMessage1;
		sysExMessage1.push_back( 240);
		sysExMessage1.push_back( 8 );
		sysExMessage1.push_back( 64 );
		sysExMessage1.push_back( 10  );
		sysExMessage1.push_back( 5); // 5 for playing a string
		sysExMessage1.push_back( channel + 1 ); // channel
		sysExMessage1.push_back( velocity );
		sysExMessage1.push_back( 247 );
		midiout->sendMessage( &sysExMessage1 );
	}else if ((type >= 128) && (type <= 143)){ // Note Off Event
		channel = type - 128;
		/* 	Here if a note other then the open string was played 
			we return the state of that string to open 
			This is for the display in game of what frets are 
			pushed down, but easily gets out of sync from 
			ghost notes
		 */
		if (note != base[channel]){ // no need to return the fret pushed down back to open, if already open
			std::vector<unsigned char> sysExMessage;
			sysExMessage.push_back( 240);
			sysExMessage.push_back( 8 );
			sysExMessage.push_back( 64 );
			sysExMessage.push_back( 10  );
			sysExMessage.push_back(1);
			sysExMessage.push_back( channel + 1);
			sysExMessage.push_back( velocity );
			sysExMessage.push_back( 247 );
			midiout->sendMessage( &sysExMessage );
		}
	}
}

int main( int argc, char *argv[] )
{
	extern char *optarg;
	int c;

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


