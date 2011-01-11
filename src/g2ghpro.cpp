/*****************************************
g2ghpro.cpp

Midi Guitar to Rock Band Pro Guitar converter

by Nathanael Anderson

wirelessdreamer @t gmai<L> d.t com

*****************************************/

#include <iostream>
#include <cstdlib>
#include "RtMidi.h"

RtMidiOut *midiout;
bool chooseMidiOutPort( RtMidiOut *rtmidi );
bool chooseMidiInPort( RtMidiIn *rtmidi );

void usage( void ) {
	// Error function in case of incorrect command-line
	// argument specifications.
	std::cout << "\nuseage: g2ghpro <port>\n";
	std::cout << "    where port = the device to use (default = 0).\n\n";
	exit( 0 );
}

void mycallback( double deltatime, std::vector< unsigned char > *message, void *userData)
{
	int base[6];
	base[5] = 40;
	base[4] = 45;
	base[3] = 50;
	base[2] = 55;
	base[1] = 59;
	base[0] = 64;

	std::vector<unsigned char> oMessage;
	std::vector<unsigned char> o1Message;
	std::vector<unsigned char> nMessage;
	int note = (int)message->at(1);
	int velocity = (int)message->at(2);
	velocity = 110;
	int type = (int)message->at(0);
	int channel = -1;

	if ((type >= 144) && (type <= 159)){ // Note On Event
		std::cout << "Note: " << note << std::endl;
		channel = type - 144;
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
	RtMidiIn *midiin = 0;

	// Minimal command-line check.
	if ( argc > 2 ) usage();

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

	// Don't ignore sysex, timing, or active sensing messages.

	midiin->ignoreTypes( false, false, false );

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


