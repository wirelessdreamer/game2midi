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

int main( int argc, char *argv[] )
{
	try {
		midiout = new RtMidiOut();
	}
	catch ( RtError &error ) {
		error.printMessage();
		exit(1);
	}
	try {
		if ( chooseMidiOutPort( midiout ) == false ){
			delete midiout;
		}
	}
	catch ( RtError &error ) {
		error.printMessage();
	}


	int channel = 0;
	int note = 41;

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


	return 0;
}
