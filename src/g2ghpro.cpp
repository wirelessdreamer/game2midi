/*****************************************
g2ghpro.cpp

Midi Guitar to Rock Band Pro Guitar converter

by Nathanael Anderson

wirelessdreamer @t gmai<L> d.t com

*****************************************/

#include <iostream>
#include <iomanip>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>  /* for sleep() in dos/mingw */
#define sleep(seconds) Sleep((seconds)*1000) /* from mingw.org list */
#endif

#include <cstdlib>
#include <limits>
#include "RtMidi.h"

RtMidiOut *midiout;
bool chooseMidiOutPort( RtMidiOut *rtmidi );
bool chooseMidiInPort( RtMidiIn *rtmidi );
int mode = 0; // This is used to change the mode between guitar (=0 default) and bass (=1 for 4 string and =2 5 string)
int count = 0; // This is used to count the number of played notes / events
int base[7]; // these are the notes the 6 strings have when played open, based on channel
int channel_offset = 0; // Default to 0 as guitar hex pickup covers all strings in centered position
int pitch_offset = 0; // Default to 0 for guitar mode
int min_velocity = 0; // Minimum velocity for a note on event
int tuning[6]={0,0,0,0,0,0}; // Standard tuning no note offset
int choice=1,end=0; //Initialize these integers as global variables as compiler complains. 
int pitch[6] = {0,0,0,0,0,0};
int rb_pitch[6]={0,0,0,0,0};
int velocity[6] = {0,0,0,0,0,0};
int note[6] ={0,0,0,0,0,0};
int bend[6] ={0,0,0,0,0,0};
int bend_range=24;
int bend_mode=0;
int verbose_mode=1;
int semitone =0; 
int tolerance=5; // Set the pitch tolerance value
int lower_tolerance=0;
int channel = -1;

void usage( void ) {
	// Error function in case of incorrect command-line
	// argument specifications.
	std::cout << "\nusage: g2ghpro [-m <GUITAR MODE> -b <BEND MODE>  -t <PITCH BEND TOLERANCE> -v <VERBOSE MODE>]\n\n";
	std::cout << "    Guitar Mode values:\n";
	std::cout << "    0 = electric guitar\n";
	std::cout << "    1 = 4 string bass\n";
	std::cout << "    2 = 5 string bass\n\n";
	std::cout << "    Pitch Bend Mode values:\n";
	std::cout << "    0 = (Default) For +/- 24 semitones (Roland GR55)\n";
	std::cout << "    1 = For +/- 12 semitones (Roland VG99/VB99) \n\n";
	std::cout << "    Pitch Bend Tolerance as a percentage of the next note above or below:\n";
	std::cout << "    0 = This disables the handling of pitch bend events.\n";
	std::cout << "    1 - 50 = Minimum value is 1% and maximum is 50%. The default value is 5%.\n\n";
	std::cout << "    Verbose Bend Mode values:\n";
	std::cout << "    0 = Silent: No output of midi messages to the terminal.\n";
	std::cout << "    1 = Default: Output note on, note off and pitch bend events being sent to rock band 3 midi pro adapter.\n";
	std::cout << "    2 = Extra verbose: High level of output for debugging. Raw midi messages and pitch bend information.\n\n";
	exit( 0 );
}

void cons_output( int note_event, int midi_status, int midi_pitch, int midi_channel, int midi_velocity, int guitar_fret ) {

	        char string;
		switch (midi_channel) {
		case 6:
		  string = 'B';
		  break;
		case 5: 
		  string = 'E';
		  break;
		case 4: 
		  string = 'A';
		  break;
		case 3: 
		  string = 'D';
		  break;
		case 2: 
		  string = 'G';
		  break;
		case 1: 
		  string = 'b';
		  break;
		case 0: 
		  string = 'e';
		  break;
		default:
		  string = '?';
		  guitar_fret = 0;
		  break;
		}
	  	
		std::cout << "#";
	  	std::cout.fill(' ');
	  	std::cout << std::setw(6) << count++;
		std::cout << "  Status message: " << std::setw(3) << midi_status;
		std::cout << "  on channel " << midi_channel << ". ";


		switch (note_event) {
		case 1: // Note on event output 
			std::cout << " Note On event ";
			std::cout << "  Pitch: " << std::setw(3) << midi_pitch;
			std::cout << "  Velocity: " << std::setw(3) << midi_velocity;
			std::cout << "  ->  ";
			std::cout << string << " " << std::setw(2) << guitar_fret;
			break;
		case 2: // Note off event output
			std::cout << " Note off event. Sending ";
			std::cout << string << " " << std::setw(2) << guitar_fret;
			break;
		case 3: // Note on event output 
			std::cout << " Bend event ";
			std::cout << "  Pitch: " << std::setw(3) << midi_pitch;
			std::cout << "  fret change: " << std::setw(3) << midi_velocity;
			std::cout << "  ->  ";
			std::cout << string << " " << std::setw(2) << guitar_fret;
			break;
		default:
			std::cout << " unknown ";
			break;
		}
		
		std::cout << std::endl;
	
}

void send_fret_to_rbmpa( int send_fret_channel, int send_fret_pitch ){

	std::vector<unsigned char> sysExMessage;
	sysExMessage.push_back( 240);
	sysExMessage.push_back( 8 );
	sysExMessage.push_back( 64 );
	sysExMessage.push_back( 10  );
	sysExMessage.push_back( 1); // 1 for sending pitch which results in the fret position
	sysExMessage.push_back( send_fret_channel + 1);
	sysExMessage.push_back( send_fret_pitch );  // Send a pitch that is calculated using the adjusted pitch, any pitch offset (guitar or bass ocatave) and any alternative tuning on that string
	sysExMessage.push_back( 247 );
	midiout->sendMessage( &sysExMessage );	
}

void send_note_to_rbmpa(int send_note_channel, int send_note_pitch, int send_note_velocity){

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

	send_fret_to_rbmpa( send_note_channel, send_note_pitch ); // Send the new position to the rock band midi pro adapter
	
	// Now send a strummed note using the midi velocity
	std::vector<unsigned char> sysExMessage;
	sysExMessage.push_back( 240);
	sysExMessage.push_back( 8 );
	sysExMessage.push_back( 64 );
	sysExMessage.push_back( 10  );
	sysExMessage.push_back( 5); // 5 for playing a string
	sysExMessage.push_back( send_note_channel + 1); // channel
	sysExMessage.push_back( send_note_velocity );
	sysExMessage.push_back( 247 );
	midiout->sendMessage( &sysExMessage );
}

void mycallback( double deltatime, std::vector< unsigned char > *message, void *userData)
{

	/* Midi messages are constructed from 3 data packets:
	 *  Status: Contains the event type and the channel. We are interested in note on, note off and pitch bend events)
	 *  Data1: This contains the first data packet which is a 7 bits long. This contains the key note for note on and off events and the least significant bits of a pitch bend.
	 *  Data2: This contains the second data packet and is also 7 bits long. This contains the velocity for note on and off events and the most significant bits of a pitch bend.
	 */
	int status = (int)message->at(0);
	int data1 = (int)message->at(1);
	int data2 = (int)message->at(2);

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
	 *
	 * Note off events have a value ranging from 128 - 143. For rock band we want to translate these into fret 0 positions
	 *
	 * Pitch bend events have a value ranging from 224 - 239. The two data packets form a 14bit signed int value for a pitch shift.
	*/
	
	if (verbose_mode > 1){
		std::cout << "Raw midi. Status: " << status << ". Data1: " << data1 << " Data2: " << data2 << ".\n";
		}

	if ((status >=144) && (status <=150)) {
		channel = status - 144 + channel_offset;  // Calcuate which channel on the note has been played from using the status data
		pitch[channel] = data1;
		velocity[channel] = data2;
		note[channel] = 1; // Note on event
		rb_pitch[channel] = pitch[channel] + pitch_offset + tuning[channel];
		
	}
	
	if ((status >=128) && (status <=134)){
		channel = status - 128 + channel_offset;  // Calculate the channel stopped playing a note. This time we don't care about the pitch and the velocity.
		note[channel] = 2; // Note off event
	}
	
	if ((status >=144) && (status <=150) && (data2 == 0)) { // VB99 does not send note off events instead it sends a zero velocity to the playing note.
		channel = status - 144 + channel_offset;  // Calcuate which channel on the note has been played from using the status data
		note[channel] = 2; // Note off event

	}
	
	if (tolerance >0){  // Pitch bend events are not processed when the tolerance is set to 0. 
		if ((status >= 224) && (status <= 230)){
			channel = status - 224 + channel_offset; // Calculate which channel sent the pitch bend 
			note[channel] = 3; // Pitch bend event
		}
	}


	if ((note[channel] == 1) && (velocity[channel] > min_velocity)  )  { // Note On Event only if the velocity is above the minumium velocity value
		
		int fret = pitch[channel] - base[channel] + tuning[channel]; // Calculates the fret and offsets any changes to standard tuning.
		

		if (( fret >= 0) && ( fret <= 22 )){ // Make sure that the pitch bend is in range of the Squier open to fret 22.
			
			if (verbose_mode > 0){
				cons_output( note[channel], status, rb_pitch[channel], channel, velocity[channel], fret);
			}	
			
			send_note_to_rbmpa( channel, rb_pitch[channel], velocity[channel]); // Send a strummed note to rock band midi pro adapter
		}else {
			if (verbose_mode >1){
				std::cout << "Pitch note out of range\n";
			}
		}


		note[channel]=0;


	}else if (note[channel] == 2){ // Note Off Event
		/* 	Here if a note other then the open string was played 
			we return the state of that string to open 
			This is for the display in game of what frets are 
			pushed down, but easily gets out of sync from 
			ghost notes
	 	*/
		if (pitch[channel]  != base[channel]){ // no need to return the fret pushed down back to open, if already open

			rb_pitch[channel] = base[channel] + pitch_offset; // Calcuate the open note based on the open string pitch definition and the pitch offset
			if (verbose_mode >0){
				int fret = rb_pitch[channel] - base[channel] - pitch_offset ; // Should always be fret 0
				cons_output( note[channel], status, rb_pitch[channel], channel, 0, fret);
			}

		send_fret_to_rbmpa( channel, rb_pitch[channel] ); // Send new position without a strum to rock band midi pro adapter.

			note[channel]=0;

			
		}

	}else if (note[channel] == 3)  { 
		/* 
		 * Pitch bend event. This code handles pitch bend midi input.
		 * This needs to translate slides, hammer ons and pull offs 
		 * to semitone fret events. Make sure a note is being played too. 
		 *
		 *  Guitar synth (GR55) allows for +/-24 semitones and the max value for
		 *  a signed 14bit number is +8192 to -8192.
		 *
		 * Calculate the shift in pitch based on the first data packet 
		 * (7bits) and the second (7bits) which is shifted 7 bits and 
		 * added to the first data packet. Then the center pitch value 
		 * (8192) of an unsigned int is subtracted to give the relative 
		 * pitch change in the range of +/- 8192 as a signed int. 
		 *
		 */
		int pitch_shift = data1 + (data2 << 7) - 8192; 		

		if (verbose_mode >1){
			std::cout << "Total shift in pitch is: pitch " << pitch[channel] << " " << pitch_shift << " on channel "<< channel << ".\n"; 		
		}
				
	
		int fret_change = (pitch_shift / semitone); // integer division should only give the whole semitone changes
		
		int remainder = (pitch_shift % semitone); // Calculate the remainder to see if it is withing the tolerance range

		if (remainder / lower_tolerance == 1) {
			fret_change += 1; 
		}else if (remainder /  lower_tolerance == -1 ) {
			fret_change -=1;
		}
		

		int new_pitch = pitch[channel] + pitch_offset + tuning[channel] + fret_change;

				
				
		if (rb_pitch[channel] != new_pitch) {
			if (verbose_mode >1){
				std::cout << "Current midi pitch = " << pitch[channel] << " on channel = " << channel << " Note status = " << note[channel] << "   New pitch = " << pitch[channel] << " + " << fret_change <<  " rock band pitch " << rb_pitch[channel] << " new rb pitch " << new_pitch <<" \n";
			}

			rb_pitch[channel] = new_pitch;
			int fret = rb_pitch[channel] - base[channel] - pitch_offset; // Calculates the fret and offsets any changes to standard tuning.
			

			if (( fret >= 0) && ( fret <= 22 )){ // Make sure that the pitch bend is in range of the Squier open to fret 22.
				if ( verbose_mode > 0 ) {
					cons_output( note[channel], status, rb_pitch[channel], channel, fret_change, fret);
				}
			
				send_fret_to_rbmpa( channel, rb_pitch[channel] ); // Send new position without a strum to rock band midi pro adapter.
			}else {
				if (verbose_mode >1){
					std::cout << "Pitch bend out of range\n";
				}
			}
			
			note[channel]=0;
		}
		

	}
}


int main( int argc, char *argv[] )
{
	extern char *optarg;
	int c, tolerance_input=0;


	while ((c = getopt(argc, argv, "m:b:v:it:")) != -1){
		switch (c){
			case 'm': 
				mode = atoi(optarg);
				break;
			case 'b':
				bend_mode=atoi(optarg);
				break;				
			case 'v':
				verbose_mode=atoi(optarg);
				break;				
			case 't':
				tolerance=atoi(optarg);
				break;				
			case '?': 
				usage();
				break;
		}
	}
	if (mode == 0){
	  std::cout << "Set mode to 6 string guitar.\n" << std::endl;
	}else if (mode == 1){
	  std::cout << "Set mode to 4 string bass.\n" << std::endl;
	}else if (mode == 2){
	  std::cout << "Set mode to 5 string bass, ignoring the top string.\n" << std::endl;
	}else if (mode == 3){
	  std::cout << "Set mode to 6 string bass, ignoring the top two strings.\n" << std::endl;
	} else {
	  std::cout << "error: undefined mode\n" << std::endl;
	  usage();
	}
	

	switch (verbose_mode){
	case 0:
		std::cout << "Silent mode. No midi output messages to the terminal.\n";
		break;
	case 2:
		std::cout << "Extra verbose mode. Raw midi and pitch bend messages sent to the terminal.\n";
		break;
	default:
		verbose_mode = 1; // Default verbose level and all other cases fallback to verbose mode 1
		break;
	}


	if (bend_mode == 0){
		bend_range = 24; // Set maximum pitch bend range to 24 semitone range such as the GR55
	}
	else if (bend_mode == 1){
		bend_range = 12; // Set maximum pitch bend range of semitones to 12 for VB99 or similar devices.
		std::cout << "Setting pitch bend range to +/- 12 semitones for VB99 or similar midi devices.\n";
	}
	else {
		bend_range=24; // Default to 24 semitone range if other value has been set by accident.
	}
	// 8192 / 24 = 340.5 bend (rounded down) per semitone
	semitone = 8192 / bend_range; // Calculate the value of a semitone (1 fret) based on the pitch bend range of the device (default = 24 for a GR55, while the VB99 is only +/-12 semitones before it sends a new note on event)
	
	if (verbose_mode >1){
		std::cout << "The semitone value is " << semitone << ".\n";
	}

	
	if (tolerance < 1){
		tolerance =0; // Minimum value for pitch bend
		std::cout <<"The minimum value for the pitch bend tolerance is 1%. Disabling pitch bend.\n";
	}else if (tolerance > 50){
		tolerance =50; // Maximum value for pitch bend
		std::cout <<"The maxiumum value for the pitch bend tolerance is 50%. Using 50%.\n";
	}	

	lower_tolerance = semitone - ( semitone * tolerance / 100);

	if ((verbose_mode >1)&&(tolerance>0)){
		std::cout << "The pitch bend semitone tolerance is " << tolerance << "% = " << semitone << " +/- " << semitone - lower_tolerance <<  ".\n";
	}

	if (mode == 0){ // guitar mode
		base[5] = 40; // channel 6 - E (low) midi note is 40
		base[4] = 45; // channel 5 - A midi note is 45
		base[3] = 50; // channel 4 - D midi note is 50
		base[2] = 55; // channel 3 - G midi note is 55
		base[1] = 59; // channel 2 - b midi note is 59
		base[0] = 64; // channel 1 - e' (high) midi note is 64
	}else if(mode == 1){ // 4 string bass mode
		base[5] = 28; // channel 5 - E (low) midi note is 28 (one octave lower and the channel is one less than guitar with centered pickup)
		base[4] = 33; // channel 4 - A midi note is 33
		base[3] = 38; // channel 3 - D midi note is 38
		base[2] = 43; // channel 2 - G midi note is 43
		channel_offset = 1;  // bass hex pickup is assumed to be installed as 4STR-2 (centered) position
		pitch_offset = 40 - 28;  // shift bass note to guitar note by one octave
	}else if(mode == 2){ // 5 string bass mode 5STR-Lo2 (Low B string at channel 5)
		base[6] = 23; // channel 6 - B (low) midi note is 23. This is ignored by rock band as there is no channel above E
		base[5] = 28; // channel 5 - E midi note is 28
		base[4] = 33; // channel 4 - A midi note is 33
		base[3] = 38; // channel 3 - D midi note is 38
		base[2] = 43; // channel 2 - G midi note is 43
		channel_offset = 1;  // The bass hex pickup covers low B to G when installed in 5STR-Lo2 (centered) position
		pitch_offset = 40 - 28;  // shift bass note to guitar note by one octave
 	}

	for (channel=0; channel <6; channel++){ // Initalise the midi pitch and rock band pitch to open strings before opening the midi port.
		pitch[channel]=base[channel];
		rb_pitch[channel]=base[channel]+pitch_offset;
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
	midiin->ignoreTypes( true, true, true);

	std::cout << "\nProcessing MIDI messages.\n";
	std::cout << "\nTo change tuning type <num> and press <enter>. Any other key to quit\n";
	std::cout << "1: Standard Tuning EADG(be)\n";
	std::cout << "2: Drop D Tuning DADG(be)\n";
	std::cout << "3: Drop 1 Step Tuning D#G#C#F#(a#d#)\n";
	std::cout << "4: Drop 2 Step Tuning DGCF(ad)\n";

	char input[2];
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); // Clear the  standard input buffer after inputting MIDI devices.

	while (!end){
		std::cin.get(input, 2); // Only get 1 character from standard input. A buffer size of 2 seems to be necessary.
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); // Clear the rest of the buffer and the new line ready for new input.
                choice = atoi(input); // Convert string to integer value.
                if (!choice){
                        std::cout << "Quitting.\n"; 
                        end=1; // Signal to end the loop
                        }
		switch (choice){
			case 1:
				tuning[0]=0; // e string no offset
				tuning[1]=0; // b string no offset
				tuning[2]=0; // G string no offset
				tuning[3]=0; // D string no offset
				tuning[4]=0; // A string no offset
				tuning[5]=0; // E string no offset
				std::cout << "Changing to standard Tuning: EADG(be)\n";
				break;
			case 2:
				tuning[0]=0; // e string no offset
				tuning[1]=0; // b string no offset
				tuning[2]=0; // G string no offset
				tuning[3]=0; // D string no offset
				tuning[4]=0; // A string no offset
				tuning[5]=2; // Drop D tuning on E string +2 offset
                                std::cout << "Changing to Drop D Tuning DADG(be)\n";
                                break;
			case 3:
                               	tuning[0]=1; // d# tuning on e string +1 offset
				tuning[1]=1; // a# tuning on b string +1 offset
				tuning[2]=1; // F# tuning on G string +1 offset
				tuning[3]=1; // C# tuning on D string +1 offset
				tuning[4]=1; // G# tuning on A string +1 offset
				tuning[5]=1; // D# tining on E string +1 offset
				std::cout << "Changing to Drop 1 Step Tuning D#G#C#F#(a#d#)\n";
                                break;
			case 4:
                                tuning[0]=2; // d tuning on e string +2 offset
				tuning[1]=2; // a tuning on b string +2 offset
				tuning[2]=2; // F tuning on G string +2 offset
				tuning[3]=2; // C tuning on D string +2 offset
				tuning[4]=2; // G tuning on A string +2 offset
				tuning[5]=2; // D tuning on E string +2 offset
				std::cout << "Changing to Drop 2 Step Tuning DGCF(ad)\n";
                                break;
			default:
				break;
			}

                sleep(1); // Only check for input once a second to prevent a busy loop.
       	}	

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


