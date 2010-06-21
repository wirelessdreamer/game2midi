/*
 *	wiiuse
 *
 *	Written By:
 *		Michael Laforest	< para >
 *		Email: < thepara (--AT--) g m a i l [--DOT--] com >
 *
 *	Copyright 2006-2007
 *
 *	This file is part of wiiuse.
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
 *	$Header$
 *
 */

/**
 *	@file
 *
 *	@brief Example using the wiiuse API.
 *
 *	This file is an example of how to use the wiiuse library.
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
	#include <unistd.h>
	#include <alsa/asoundlib.h>
    #include <alsa/seqmid.h>
    #include <linux/uinput.h>
    #include <sys/types.h>
    #include <sys/stat.h>
#endif

#include "wiiuse.h"

#define MAX_WIIMOTES	4

#ifndef WIN32
    static int uinp_fd = -1;
    struct uinput_user_dev uinp;       // uInput device structure
    struct input_event event; // Input device structure
#endif

int midi_type = 0;
int init = 0;
int bb_threshold = 11;
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

snd_seq_t *seq_handle;
snd_seq_event_t ev;

int alsa_init(int controller_number){
    char client_name[32];
    char port_name[48];
    snd_seq_addr_t src;

    /* Create the sequencer port. */

    sprintf(client_name, "wii2midi%i", controller_number);
    sprintf(port_name , "%s Output", client_name);

    if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
        puts("Error: Failed to access the ALSA sequencer.");
        exit(-1);
    }

    snd_seq_set_client_name(seq_handle, client_name);
    src.client = snd_seq_client_id(seq_handle);
    src.port = snd_seq_create_simple_port(seq_handle, "Wiimote Output",
        SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_APPLICATION);

    /* Init the event structure */

    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, src.port);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    return 0;

}

int midi_note_on(int note, int velocity, int channel){
    if (0 == midi_type){
        printf("Send note %i with velocity %i\n", note, velocity);
        snd_seq_ev_set_noteon(&ev, channel, note, velocity);
        snd_seq_event_output_direct(seq_handle, &ev);
        snd_seq_ev_set_noteoff(&ev, channel, note, 0);
        snd_seq_event_output_direct(seq_handle, &ev);
    }
	printf("Returning\n");
    return 0;
}

int find_velocity(int input){
    int calc_velocity = 150 - input;
    if (calc_velocity > 127) calc_velocity = 127;
    if (calc_velocity < 12) calc_velocity = 12;
    return calc_velocity;
}

void write_uinput()
{
    int result = write(uinp_fd, &event, sizeof(event));
    if ( -1 == result)
    {
        fprintf(stderr, "Unable to write to uinput file handler\n");
    }
}

void ioctl_wrapper(int uinp_fd, int UI_SETBIT, int i)
{
    if ( -1 == (ioctl(uinp_fd, UI_SET_EVBIT, i)))
    {
        fprintf(stderr, "ioctl Error: %s\n", strerror(errno));
        exit (-2);
    }
}

int setup_uinput_device()
{
    if ( -1 == (uinp_fd = open("/dev/misc/uinput", O_WRONLY | O_NDELAY)) )
    {
        if ( -1 == (uinp_fd = open("/dev/input/uinput", O_WRONLY | O_NDELAY)) )
        {
            if ( -1 ==  (uinp_fd = open("/dev/uinput", O_WRONLY | O_NDELAY)) )
            {
                fprintf(stderr,"Unable to open /dev/misc/uinput\n");
                fprintf(stderr,"Unable to open /dev/input/uinput\n");
                fprintf(stderr,"Unable to open /dev/uinput\n");
                fprintf(stderr,"Unable to open any uinput device: %s\n", strerror(errno));
                fprintf(stderr,"Please make sure the uinput module is loaded in your kernel\n");
                exit(-2);
            }
        }
    }


    memset(&uinp,0,sizeof(uinp));
    strncpy(uinp.name, "Wiimote Beat to Keyboard Converter", UINPUT_MAX_NAME_SIZE);
    uinp.id.version = 4;
    uinp.id.bustype = BUS_USB;

    ioctl_wrapper(uinp_fd, UI_SET_EVBIT, EV_KEY);
    ioctl_wrapper(uinp_fd, UI_SET_EVBIT, EV_REL);
    ioctl_wrapper(uinp_fd, UI_SET_RELBIT, REL_X);
    ioctl_wrapper(uinp_fd, UI_SET_RELBIT, REL_Y);

    int i = 0;
    for (i=0; i < 256; i++) {
        ioctl(uinp_fd, UI_SET_KEYBIT, i);
    }
    write_uinput();
    if (-1 == ioctl(uinp_fd, UI_DEV_CREATE))
    {
        fprintf(stderr, "Unable to create UINPUT device. Error: %s\n", strerror(errno));
        close(uinp_fd);
        exit (-2);
    }
    return 1;
}

void send_key_down(__u16 key_code)
{
    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = key_code;
    event.value = 1;
    write_uinput();
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write_uinput();
}

void send_key_up(__u16 key_code)
{
    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = key_code;
    event.value = 0;
    write_uinput();
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write_uinput();
}

/**
 *	@brief Callback that handles an event.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *
 *	This function is called automatically by the wiiuse library when an
 *	event occurs on the specified wiimote.
 */
void handle_event(struct wiimote_t* wm) {

	/* if a button is pressed, report it */
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_A))
	{
		printf("A pressed\n");
		wiiuse_set_wii_board_calib(wm);
	}

	/* show events specific to supported expansions */
	if (wm->exp.type == EXP_WII_BOARD)
	{
		short ptr, ptl, pbr, pbl;
		struct wii_board_t* wb = (wii_board_t*)&wm->exp.wb;
		ptr = wb->tr;
		ptl = wb->tl;
		pbr = wb->br;
		pbl = wb->bl;
		printf("%5d, %5d, %5d, %5d, %f\n",ptr, ptl, pbr, pbl, (ptr+ptl+pbr+pbl)/4*2.2046);
		if ((ptr > bb_threshold) && (0 == tr_tracker)){
            midi_note_on((note + 1),100, midi_channel);
            tr_tracker = 1;
        }else{
            if (ptr < bb_threshold){
                tr_tracker = 0;
            }
        }
        if ((ptl > bb_threshold) && (0 == tl_tracker)){
            midi_note_on((note - 6),100, midi_channel);
            tl_tracker = 1;
        }else{
            if (ptl < bb_threshold){
                tl_tracker = 0;
            }
        }
	}
}


/**
 *	@brief Callback that handles a read event.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param data		Pointer to the filled data block.
 *	@param len		Length in bytes of the data block.
 *
 *	This function is called automatically by the wiiuse library when
 *	the wiimote has returned the full data requested by a previous
 *	call to wiiuse_read_data().
 *
 *	You can read data on the wiimote, such as Mii data, if
 *	you know the offset address and the length.
 *
 *	The \a data pointer was specified on the call to wiiuse_read_data().
 *	At the time of this function being called, it is not safe to deallocate
 *	this buffer.
 */
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


/**
 *	@brief Callback that handles a controller status event.
 *
 *	@param wm				Pointer to a wiimote_t structure.
 *	@param attachment		Is there an attachment? (1 for yes, 0 for no)
 *	@param speaker			Is the speaker enabled? (1 for yes, 0 for no)
 *	@param ir				Is the IR support enabled? (1 for yes, 0 for no)
 *	@param led				What LEDs are lit.
 *	@param battery_level	Battery level, between 0.0 (0%) and 1.0 (100%).
 *
 *	This occurs when either the controller status changed
 *	or the controller status was requested explicitly by
 *	wiiuse_status().
 *
 *	One reason the status can change is if the nunchuk was
 *	inserted or removed from the expansion port.
 */
void handle_ctrl_status(struct wiimote_t* wm) {
	printf("\n\n--- CONTROLLER STATUS [wiimote id %i] ---\n", wm->unid);

	printf("attachment:      %i\n", wm->exp.type);
	printf("speaker:         %i\n", WIIUSE_USING_SPEAKER(wm));
	printf("ir:              %i\n", WIIUSE_USING_IR(wm));
	printf("leds:            %i %i %i %i\n", WIIUSE_IS_LED_SET(wm, 1), WIIUSE_IS_LED_SET(wm, 2), WIIUSE_IS_LED_SET(wm, 3), WIIUSE_IS_LED_SET(wm, 4));
	printf("battery:         %f %%\n", wm->battery_level);
}


/**
 *	@brief Callback that handles a disconnection event.
 *
 *	@param wm				Pointer to a wiimote_t structure.
 *
 *	This can happen if the POWER button is pressed, or
 *	if the connection is interrupted.
 */
void handle_disconnect(wiimote* wm) {
	printf("\n\n--- DISCONNECTED [wiimote id %i] ---\n", wm->unid);
}


void test(struct wiimote_t* wm, byte* data, unsigned short len) {
	printf("test: %i [%x %x %x %x]\n", len, data[0], data[1], data[2], data[3]);
}



/**
 *	@brief main()
 *
 *	Connect to up to two wiimotes and print any events
 *	that occur on either device.
 */
int main(int argc, char** argv) {
	wiimote** wiimotes;
	int found, connected;

	/*
	 *	Initialize an array of wiimote objects.
	 *
	 *	The parameter is the number of wiimotes I want to create.
	 */
	wiimotes =  wiiuse_init(MAX_WIIMOTES);

	/*
	 *	Find wiimote devices
	 *
	 *	Now we need to find some wiimotes.
	 *	Give the function the wiimote array we created, and tell it there
	 *	are MAX_WIIMOTES wiimotes we are interested in.
	 *
	 *	Set the timeout to be 5 seconds.
	 *
	 *	This will return the number of actual wiimotes that are in discovery mode.
	 */
	found = wiiuse_find(wiimotes, MAX_WIIMOTES, 5);
	if (!found) {
		printf ("No wiimotes found.");
		return 0;
	}

	/*
	 *	Connect to the wiimotes
	 *
	 *	Now that we found some wiimotes, connect to them.
	 *	Give the function the wiimote array and the number
	 *	of wiimote devices we found.
	 *
	 *	This will return the number of established connections to the found wiimotes.
	 */
	connected = wiiuse_connect(wiimotes, MAX_WIIMOTES);
	if (connected)
		printf("Connected to %i wiimotes (of %i found).\n", connected, found);
	else {
		printf("Failed to connect to any wiimote.\n");
		return 0;
	}

	/*
	 *	Now set the LEDs and rumble for a second so it's easy
	 *	to tell which wiimotes are connected (just like the wii does).
	 */
	wiiuse_set_leds(wiimotes[0], WIIMOTE_LED_1);
	wiiuse_set_leds(wiimotes[1], WIIMOTE_LED_2);
	wiiuse_set_leds(wiimotes[2], WIIMOTE_LED_3);
	wiiuse_set_leds(wiimotes[3], WIIMOTE_LED_4);
	wiiuse_rumble(wiimotes[0], 1);
	wiiuse_rumble(wiimotes[1], 1);

	#ifndef WIN32
		usleep(200000);
	#else
		Sleep(200);
	#endif

	wiiuse_rumble(wiimotes[0], 0);
	wiiuse_rumble(wiimotes[1], 0);

	/*
	 *	Maybe I'm interested in the battery power of the 0th
	 *	wiimote.  This should be WIIMOTE_ID_1 but to be sure
	 *	you can get the wiimote assoicated with WIIMOTE_ID_1
	 *	using the wiiuse_get_by_id() function.
	 *
	 *	A status request will return other things too, like
	 *	if any expansions are plugged into the wiimote or
	 *	what LEDs are lit.
	 */
	//wiiuse_status(wiimotes[0]);

	/*
	 *	This is the main loop
	 *
	 *	wiiuse_poll() needs to be called with the wiimote array
	 *	and the number of wiimote structures in that array
	 *	(it doesn't matter if some of those wiimotes are not used
	 *	or are not connected).
	 *
	 *	This function will set the event flag for each wiimote
	 *	when the wiimote has things to report.
	 */

	int init_loop = 0;
    for (; init_loop < MAX_WIIMOTES; ++init_loop){
        if (0 == alsa_init(init_loop)){
            printf("Midi started for wiimote %i\n", init_loop);
        }
        wiiuse_motion_sensing(wiimotes[init_loop], 1);
        wiiuse_set_ir(wiimotes[init_loop], 1);
    }


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
						handle_event(wiimotes[i]);
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

					case WIIUSE_WII_BOARD_CTRL_INSERTED:
						/* some expansion was inserted */
						printf("Wii Balance Board detected.\n");
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

	/*
	 *	Disconnect the wiimotes
	 */
	wiiuse_cleanup(wiimotes, MAX_WIIMOTES);

	return 0;
}
