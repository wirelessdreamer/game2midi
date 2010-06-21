/*
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

#include <stdio.h>
#include <stdlib.h>
#include <lo/lo.h>
#define PI 3.141592653589793;

int x_orient = 0;
int x_last = 0;
int y_orient = 0;
int y_last;
int smoothing = 3;

#ifndef WIN32
	#include <unistd.h>
#endif

#include "wiiuse.h"

#define MAX_WIIMOTES 4

void handle_event(struct wiimote_t* wm, lo_address osc) {
	if (wm->exp.type == EXP_WII_BOARD)
	{
		if (IS_PRESSED(wm, WIIMOTE_BUTTON_A))
		{
			printf("Wii board calibration pressed\n");
			wiiuse_set_wii_board_calib(wm);
		}
		short fr, fl, br, bl, weight, right, left, front, back;
		int x,y;
		struct wii_board_t* wb = (wii_board_t*)&wm->exp.wb;
		fr = wb->tr;
		fl = wb->tl;
		br = wb->br;
		bl = wb->bl;
		weight = (fr+fl+br+bl)/4*2.2046;
		front = fr + fl;
		back = br + bl;
		right = fr + br;
		left = fl + bl;
		y = float(front - back) * -1;
		x = float(right - left);
		lo_send(osc, "/x", "f", float(x));	
		lo_send(osc, "/y", "f", float(y));	
		lo_send(osc, "/weight", "f", float(weight));
		printf("Orientation x: %3d y: %3d\n", x, y);
		printf("fr: %5d fl: %5d br: %5d bl: %5d\nWeight: %d\n\n",fr, fl, br, bl, weight);
		
	}else{
		if (WIIUSE_USING_ACC(wm)){
			float x = (131 - wm->accel.x) * PI;
			x *= -1;
			float y = (134 - wm->accel.y) * PI;
			float orientation = (156 - wm->accel.z) * PI;
	//		printf("Gforce: %f, %f, %f\n", wm->gforce.x, wm->gforce.y, wm->gforce.z);
			printf("X: %f, %f Y: %f, %f\n", wm->orient.roll, wm->orient.a_roll, wm->orient.pitch, wm->orient.a_pitch);

			x = wm->orient.roll;
			y = wm->orient.pitch * -1;
		
			if (orientation > 90){
				if (abs(x) > abs(y)){
					if (x > 0){
						x = 180 - x;
					}
					if (x < 0){
						x = (180 + x) * -1;
					}
				}else{
					if (y > 0){
						y = 180 - y;
					}
					if (y < 0){
						y = (180 + y) * -1; 
					}
				}
			
			}	
			if (x_last > (x + smoothing))
				x_orient = 1;	
			if (x_last < (x + smoothing))
				x_orient = -1;
			if (y_last > (y + smoothing))
				y_orient = 1;
			if (y_last < (y + smoothing))
				y_orient = -1;
printf("%i, %i\n", x_orient, y_orient);

			if ((x_orient == 1) && (x > x_last)){
				lo_send(osc, "/x", "f", x);	
			}
			if ((x_orient == -1) && (x < x_last)){	
				lo_send(osc, "/x", "f", x);	
			}			
			if ((y_orient == 1) && (y > y_last)){
				lo_send(osc, "/y", "f", y);	
			}
			if ((y_orient == -1) && (y < y_last)){	
				lo_send(osc, "/y", "f", y);	
			}
			printf("%f, %f, %f\n", x, y, orientation);
			lo_send(osc, "/x", "f", x);	
			lo_send(osc, "/y", "f", y);	
//			lo_send(osc, "/x", "f", wm->orient.roll);	
//			lo_send(osc, "/y", "f", -1 * wm->orient.pitch);	
			x_last = x;
			y_last = y;
		}
	}
}

void handle_disconnect(wiimote* wm) {
	printf("\n\n--- DISCONNECTED [wiimote id %i] ---\n", wm->unid);
}

int main(int argc, char** argv) {
	wiimote** wiimotes;
	int found, connected;
	wiimotes =  wiiuse_init(MAX_WIIMOTES);

	found = wiiuse_find(wiimotes, MAX_WIIMOTES, 5);
	if (!found) {
		printf ("No wiimotes found.");
		return 0;
	}

	connected = wiiuse_connect(wiimotes, MAX_WIIMOTES);
	if (connected)
		printf("Connected to %i wiimotes (of %i found).\n", connected, found);
	else {
		printf("Failed to connect to any wiimote.\n");
		return 0;
	}

	wiiuse_set_leds(wiimotes[0], WIIMOTE_LED_1);
	wiiuse_motion_sensing(wiimotes[0], 1);
//	wiiuse_motion_plus(wiimotes[0], 1);

	#ifndef WIN32
		usleep(200000);
	#else
		Sleep(200);
	#endif

	lo_address osc = lo_address_new(NULL, "9000");
	while (1) {
		if (wiiuse_poll(wiimotes, MAX_WIIMOTES)) {
			int i = 0;
			for (; i < MAX_WIIMOTES; ++i) {
				switch (wiimotes[i]->event) {
					case WIIUSE_EVENT:
						handle_event(wiimotes[i], osc);
						break;

					case WIIUSE_STATUS:
	//					handle_ctrl_status(wiimotes[i]);
						break;

					case WIIUSE_DISCONNECT:
					case WIIUSE_UNEXPECTED_DISCONNECT:
						handle_disconnect(wiimotes[i]);
						break;

					case WIIUSE_READ_DATA:
						break;

					case WIIUSE_NUNCHUK_INSERTED:
						printf("Nunchuk inserted.\n");
						break;

					case WIIUSE_CLASSIC_CTRL_INSERTED:
						printf("Classic controller inserted.\n");
						break;

					case WIIUSE_WII_BOARD_CTRL_INSERTED:
						printf("Wii Balance Board detected.\n");
						break;

					case WIIUSE_MOTION_PLUS_CTRL_INSERTED:
						printf("Wii Motion Plus detected.\n");
						break;
					case WIIUSE_NUNCHUK_REMOVED:
					case WIIUSE_CLASSIC_CTRL_REMOVED:
					case WIIUSE_GUITAR_HERO_3_CTRL_REMOVED:
					case WIIUSE_WII_BOARD_CTRL_REMOVED:
					case WIIUSE_MOTION_PLUS_CTRL_REMOVED:
//						handle_ctrl_status(wiimotes[i]);
						printf("An expansion was removed.\n");
						break;

					default:
						break;
				}
			}
		}
	}

	wiiuse_cleanup(wiimotes, MAX_WIIMOTES);

	return 0;
}
