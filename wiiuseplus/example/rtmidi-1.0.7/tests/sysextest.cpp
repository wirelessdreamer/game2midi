//*****************************************//
//  sysextest.cpp
//  by Gary Scavone, 2003-2005.
//
//  Simple program to test MIDI sysex sending and receiving.
//
//*****************************************//

#include <iostream>
#include <cstdlib>
#include <typeinfo>
#include "RtMidi.h"

void usage( void ) {
  std::cout << "\nuseage: sysextest N\n";
  std::cout << "    where N = length of sysex message to send / receive.\n\n";
  exit( 0 );
}

// Platform-dependent sleep routines.
#if defined(__WINDOWS_MM__)
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

// This function should be embedded in a try/catch block in case of
// an exception.  It offers the user a choice of MIDI ports to open.
// It returns false if there are no ports available.
bool chooseMidiPort( RtMidi *rtmidi );

int main( int argc, char *argv[] )
{
  RtMidiOut *midiout = 0;
  RtMidiIn *midiin = 0;
  std::vector<unsigned char> message;
  double stamp;
  unsigned int i, nBytes;

  // Minimal command-line check.
  if ( argc != 2 ) usage();
  nBytes = (unsigned int) atoi( argv[1] );

  // RtMidiOut and RtMidiIn constructors
  try {
    midiout = new RtMidiOut();
    midiin = new RtMidiIn();
  }
  catch ( RtError &error ) {
    error.printMessage();
    goto cleanup;
  }

  // Don't ignore sysex, timing, or active sensing messages.
  midiin->ignoreTypes( false, true, true );

  // Call function to select ports
  try {
    if ( chooseMidiPort( midiin ) == false ) goto cleanup;
    if ( chooseMidiPort( midiout ) == false ) goto cleanup;
  }
  catch ( RtError &error ) {
    error.printMessage();
    goto cleanup;
  }

  // Create a long sysex messages of numbered bytes and send it out.
  message.push_back( 240 );
  for ( i=0; i<nBytes; i++ )
    message.push_back( i % 128 );
  message.push_back( 247 );
  midiout->sendMessage( &message );

  SLEEP( 50 ); // pause a little

  // Look for one message (hopefully the previously sent sysex if the
  // ports were connected together) and print out the values.
  stamp = midiin->getMessage( &message );
  nBytes = message.size();
  for ( i=0; i<nBytes; i++ )
    std::cout << "Byte " << i << " = " << (int)message[i] << std::endl;

  // Clean up
 cleanup:
  delete midiout;
  delete midiin;

  return 0;
}

bool chooseMidiPort( RtMidi *rtmidi )
{
  bool isInput = false;
  if ( typeid( *rtmidi ) == typeid( RtMidiIn ) )
    isInput = true;

  if ( isInput )
    std::cout << "\nWould you like to open a virtual input port? [y/N] ";
  else
    std::cout << "\nWould you like to open a virtual output port? [y/N] ";

  std::string keyHit;
  std::getline( std::cin, keyHit );
  if ( keyHit == "y" ) {
    rtmidi->openVirtualPort();
    return true;
  }

  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if ( nPorts == 0 ) {
    if ( isInput )
      std::cout << "No input ports available!" << std::endl;
    else
      std::cout << "No output ports available!" << std::endl;
    return false;
  }

  if ( nPorts == 1 ) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  }
  else {
    for ( i=0; i<nPorts; i++ ) {
      portName = rtmidi->getPortName(i);
      if ( isInput )
        std::cout << "  Input port #" << i << ": " << portName << '\n';
      else
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
