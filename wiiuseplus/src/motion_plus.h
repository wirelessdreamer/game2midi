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

#ifndef MOTION_PLUS_H_INCLUDED
#define MOTION_PLUS_H_INCLUDED

#include "wiiuse_internal.h"

#define MOTION_PLUS_JS_MIN_X				0xC5
#define MOTION_PLUS_JS_MAX_X				0xFC
#define MOTION_PLUS_JS_CENTER_X			0xE0
#define MOTION_PLUS_JS_MIN_Y				0xC5
#define MOTION_PLUS_JS_MAX_Y				0xFA
#define MOTION_PLUS_JS_CENTER_Y			0xE0
#define MOTION_PLUS_WHAMMY_BAR_MIN		0xEF
#define MOTION_PLUS_WHAMMY_BAR_MAX		0xFA

#ifdef __cplusplus
extern "C" {
#endif

int motion_plus_handshake(struct wiimote_t* wm, struct motion_plus_t* mp, byte* data, unsigned short len);

void motion_plus_disconnected(struct motion_plus_t* mp);

void motion_plus_event(struct motion_plus_t* mp, byte* msg);

#ifdef __cplusplus
}
#endif

#endif // MOTION_PLUS_H_INCLUDED
