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
 *	@brief Motion Plus expansion device.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef WIN32
	#include <Winsock2.h>
#endif

#include "definitions.h"
#include "wiiuse_internal.h"
#include "dynamics.h"
#include "events.h"
#include "motion_plus.h"


/**
 *	@brief Handle the handshake data from the guitar.
 *
 *	@param cc		A pointer to a classic_ctrl_t structure.
 *	@param data		The data read in from the device.
 *	@param len		The length of the data block, in bytes.
 *
 *	@return	Returns 1 if handshake was successful, 0 if not.
 */
int motion_plus_handshake(struct wiimote_t* wm, struct motion_plus_t* mp, byte* data, unsigned short len) {
	int i;
	int offset = 0;

	mp->x = 0;
	mp->y = 0;
	mp->z = 0;
	mp->lx = 0;
	mp->ly = 0;
	mp->lz = 0;

	/* decrypt data */
	for (i = 0; i < len; ++i)
		data[i] = (data[i] ^ 0x17) + 0x17;

	if (data[offset] == 0xFF) {
		/*
		 *	Sometimes the data returned here is not correct.
		 *	This might happen because the wiimote is lagging
		 *	behind our initialization sequence.
		 *	To fix this just request the handshake again.
		 *
		 *	Other times it's just the first 16 bytes are 0xFF,
		 *	but since the next 16 bytes are the same, just use
		 *	those.
		 */
		if (data[offset + 16] == 0xFF) {
			/* get the calibration data */
			byte* handshake_buf = malloc(EXP_HANDSHAKE_LEN * sizeof(byte));

			WIIUSE_DEBUG("Motion Plus handshake appears invalid, trying again.");
			wiiuse_read_data_cb(wm, handshake_expansion, handshake_buf, WM_EXP_MEM_CALIBR, EXP_HANDSHAKE_LEN);

			return 0;
		} else
			offset += 16;
	}

	/* handshake done */
	wm->exp.type = EXP_MOTION_PLUS;

	#ifdef WIN32
	wm->timeout = WIIMOTE_DEFAULT_TIMEOUT;
	#endif

	return 1;
}


/**
 *	@brief The motion plusdisconnected.
 *
 *	@param cc		A pointer to a classic_ctrl_t structure.
 */
void motion_plus_disconnected(struct motion_plus_t* mp) {
	memset(mp, 0, sizeof(struct motion_plus_t));
}



/**
 *	@brief Handle guitar event.
 *
 *	@param cc		A pointer to a classic_ctrl_t structure.
 *	@param msg		The message specified in the event packet.
 */
void motion_plus_event(struct motion_plus_t* mp, byte* msg) {
	int i;

	/* decrypt data */
	for (i = 0; i < 6; ++i)
		msg[i] = (msg[i] ^ 0x17) + 0x17;
// TODO
//	mp->whammy_bar = (msg[3] - MOTION_PLUS_WHAMMY_BAR_MIN) / (float)(MOTION_PLUS_WHAMMY_BAR_MAX - MOTION_PLUS_WHAMMY_BAR_MIN);
}
