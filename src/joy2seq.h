#ifndef _JOY2CHORD_H_
#define _JOY2CHORD_H_

#include <string>
#include <map>
#include <alsa/asoundlib.h>
#include <alsa/seqmid.h>

#define TOOL_NAME "joy2seq"

const int MAX_CODES = 512; // input.h can define up to 512 input buttons, so using 512 here too
const int MAX_BUTTONS = 16;// I ran into problems dynamically allocating how many array positions there would be for holding buttons, so using this for now
const int MAX_MODES = 16; // same problem with dynamically allocating
const int MAX_AXES = 64; // same problem with dynamically allocating
const int MAX_BAD = 40000; // A workaround for handling device initilization input, This should only be a Temp Solution
const int MAX_MACROS = 16; // How many macros can be understood
const int MAX_MODIFIERS = 8; // How many modifier codes can be used (ctrl, alt, meta, etc) these keys are held down untill a non modifier is held down

class joy2seq
{
public:
	std::string device_name;
	std::string config_file;

	// mapping variables
	int modes[MAX_MODES][MAX_CODES];
	int simple_modes[MAX_MODES][MAX_CODES][2];
	int modifier[MAX_MODIFIERS];
	int total_modifiers;
	int modifier_state[MAX_MODIFIERS];
	int total_modes;
	int mode_code[MAX_MODES];
	int total_macros;
	int macro_values[MAX_MACROS];
	int mode;
	int button_state[MAX_BUTTONS];
	int holdnote[MAX_BUTTONS];
	int holdchannel[MAX_BUTTONS];
	int lastcontroller[MAX_BUTTONS];
	int send_code[MAX_BUTTONS];
	int modecount[MAX_MODES];
	int button_code;
	int verbose;
	int debug;
	int calibration;
	
	// joystick variables
	int device_number; 
	int total_chorded_buttons; // how many buttons are treated as a chorded keyboard
	int total_simple_buttons; // how many buttons are treated as a normal key
	int controller_buttons; // how many buttons the hardware controller provides
	int axes; // how many axes we defined functions for
	int total_axes; // how many analog axes the hardware controller provides
	int chord_values[MAX_CODES];
	int simple_values[MAX_CODES];
	int joy_fd;
	int lastnote;
	int thisnote;
	int justpressed;
	snd_seq_addr_t dest;
	snd_seq_t *seq_handle;
	snd_seq_event_t ev;
	
	int open_joystick();
	int read_config(std::map<string, int> & chordmap);
	int open_alsa_seq();
	void send_click_events();
	void send_note_down(int note, int velocity, int channel);
	void send_note_up(int note, int channel);
	void process_events(js_event js);
	int valid_note(std::string newnote);
	void main_loop(std::map<string, int> chordmap);
	void macro_parser(std::string macro);
};

#endif


