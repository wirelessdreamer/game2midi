 /***************************************************************************
 *   Copyright (C) 2007-2009 by Nathanael Anderson                              *
 *   wirelessdreamer AT gmail DOT com                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

int joy2chord::read_config(map<string,__u16>  & chordmap)
{
/*	// string manipulation example code
		ostringstream lbuffer;
		lbuffer << mode_loop;
		string tmpname = lbuffer.str();
		string filename = "joy2chord-mode" + tmpname;
		ConfigFile config(filename);
*/
	// these values come from /usr/include/linux/input.h
	chordmap["KEY_RESERVED"] = 0;
	chordmap["KEY_ESC"] = 1;
	chordmap["KEY_1"] = 2;
	chordmap["KEY_2"] = 3;
	chordmap["KEY_3"] = 4;
	chordmap["KEY_4"] = 5;
	chordmap["KEY_5"] = 6;
	chordmap["KEY_6"] = 7;
	chordmap["KEY_7"] = 8;
	chordmap["KEY_8"] = 9;
	chordmap["KEY_9"] = 10;
	chordmap["KEY_0"] = 11;
	chordmap["KEY_MINUS"] = 12;
	chordmap["KEY_EQUAL"] = 13;
	chordmap["KEY_BACKSPACE"] = 14;
	chordmap["KEY_TAB"] = 15;
	chordmap["KEY_Q"] = 16;
	chordmap["KEY_W"] = 17;
	chordmap["KEY_E"] = 18;
	chordmap["KEY_R"] = 19;
	chordmap["KEY_T"] = 20;
	chordmap["KEY_Y"] = 21;
	chordmap["KEY_U"] = 22;
	chordmap["KEY_I"] = 23;
	chordmap["KEY_O"] = 24;
	chordmap["KEY_P"] = 25;
	chordmap["KEY_LEFTBRACE"] = 26;
	chordmap["KEY_RIGHTBRACE"] = 27;
	chordmap["KEY_ENTER"] = 28;
	chordmap["KEY_LEFTCTRL"] = 29;
	chordmap["KEY_A"] = 30;
	chordmap["KEY_S"] = 31;
	chordmap["KEY_D"] = 32;
	chordmap["KEY_F"] = 33;
	chordmap["KEY_G"] = 34;
	chordmap["KEY_H"] = 35;
	chordmap["KEY_J"] = 36;
	chordmap["KEY_K"] = 37;
	chordmap["KEY_L"] = 38;
	chordmap["KEY_SEMICOLON"] = 39;
	chordmap["KEY_APOSTROPHE"] = 40;
	chordmap["KEY_GRAVE"] = 41;
	chordmap["KEY_LEFTSHIFT"] = 42;
	chordmap["KEY_BACKSLASH"] = 43;
	chordmap["KEY_Z"] = 44;
	chordmap["KEY_X"] = 45;
	chordmap["KEY_C"] = 46;
	chordmap["KEY_V"] = 47;
	chordmap["KEY_B"] = 48;
	chordmap["KEY_N"] = 49;
	chordmap["KEY_M"] = 50;
	chordmap["KEY_COMMA"] = 51;
	chordmap["KEY_DOT"] = 52;
	chordmap["KEY_SLASH"] = 53;
	chordmap["KEY_RIGHTSHIFT"] = 54;
	chordmap["KEY_KPASTERISK"] = 55;
	chordmap["KEY_LEFTALT"] = 56;
	chordmap["KEY_SPACE"] = 57;
	chordmap["KEY_CAPSLOCK"] = 58;
	chordmap["KEY_F1"] = 59;
	chordmap["KEY_F2"] = 60;
	chordmap["KEY_F3"] = 61;
	chordmap["KEY_F4"] = 62;
	chordmap["KEY_F5"] = 63;
	chordmap["KEY_F6"] = 64;
	chordmap["KEY_F7"] = 65;
	chordmap["KEY_F8"] = 66;
	chordmap["KEY_F9"] = 67;
	chordmap["KEY_F10"] = 68;
	chordmap["KEY_NUMLOCK"] = 69;
	chordmap["KEY_SCROLLLOCK"] = 70;
	chordmap["KEY_KP7"] = 71;
	chordmap["KEY_KP8"] = 72;
	chordmap["KEY_KP9"] = 73;
	chordmap["KEY_KPMINUS"] = 74;
	chordmap["KEY_KP4"] = 75;
	chordmap["KEY_KP5"] = 76;
	chordmap["KEY_KP6"] = 77;
	chordmap["KEY_KPPLUS"] = 78;
	chordmap["KEY_KP1"] = 79;
	chordmap["KEY_KP2"] = 80;
	chordmap["KEY_KP3"] = 81;
	chordmap["KEY_KP0"] = 82;
	chordmap["KEY_KPDOT"] = 83;
	chordmap["KEY_ZENKAKUHANKAKU"] = 85;
	chordmap["KEY_102ND"] = 86;
	chordmap["KEY_F11"] = 87;
	chordmap["KEY_F12"] = 88;
	chordmap["KEY_RO"] = 89;
	chordmap["KEY_KATAKANA"] = 90;
	chordmap["KEY_HIRAGANA"] = 91;
	chordmap["KEY_HENKAN"] = 92;
	chordmap["KEY_KATAKANAHIRAGANA"] = 93;
	chordmap["KEY_MUHENKAN"] = 94;
	chordmap["KEY_KPJPCOMMA"] = 95;
	chordmap["KEY_KPENTER"] = 96;
	chordmap["KEY_RIGHTCTRL"] = 97;
	chordmap["KEY_KPSLASH"] = 98;
	chordmap["KEY_SYSRQ"] = 99;
	chordmap["KEY_RIGHTALT"] = 100;
	chordmap["KEY_LINEFEED"] = 101;
	chordmap["KEY_HOME"] = 102;
	chordmap["KEY_UP"] = 103;
	chordmap["KEY_PAGEUP"] = 104;
	chordmap["KEY_LEFT"] = 105;
	chordmap["KEY_RIGHT"] = 106;
	chordmap["KEY_END"] = 107;
	chordmap["KEY_DOWN"] = 108;
	chordmap["KEY_PAGEDOWN"] = 109;
	chordmap["KEY_INSERT"] = 110;
	chordmap["KEY_DELETE"] = 111;
	chordmap["KEY_MACRO"] = 112;
	chordmap["KEY_MUTE"] = 113;
	chordmap["KEY_VOLUMEDOWN"] = 114;
	chordmap["KEY_VOLUMEUP"] = 115;
	chordmap["KEY_POWER"] = 116;
	chordmap["KEY_KPEQUAL"] = 117;
	chordmap["KEY_KPPLUSMINUS"] = 118;
	chordmap["KEY_PAUSE"] = 119;
	chordmap["KEY_KPCOMMA"] = 121;
	chordmap["KEY_HANGEUL"] = 122;
	chordmap["KEY_HANGUEL"] = KEY_HANGEUL;
	chordmap["KEY_HANJA"] = 123;
	chordmap["KEY_YEN"] = 124;
	chordmap["KEY_LEFTMETA"] = 125;
	chordmap["KEY_RIGHTMETA"] = 126;
	chordmap["KEY_COMPOSE"] = 127;
	chordmap["KEY_STOP"] = 128;
	chordmap["KEY_AGAIN"] = 129;
	chordmap["KEY_PROPS"] = 130;
	chordmap["KEY_UNDO"] = 131;
	chordmap["KEY_FRONT"] = 132;
	chordmap["KEY_COPY"] = 133;
	chordmap["KEY_OPEN"] = 134;
	chordmap["KEY_PASTE"] = 135;
	chordmap["KEY_FIND"] = 136;
	chordmap["KEY_CUT"] = 137;
	chordmap["KEY_HELP"] = 138;
	chordmap["KEY_MENU"] = 139;
	chordmap["KEY_CALC"] = 140;
	chordmap["KEY_SETUP"] = 141;
	chordmap["KEY_SLEEP"] = 142;
	chordmap["KEY_WAKEUP"] = 143;
	chordmap["KEY_FILE"] = 144;
	chordmap["KEY_SENDFILE"] = 145;
	chordmap["KEY_DELETEFILE"] = 146;
	chordmap["KEY_XFER"] = 147;
	chordmap["KEY_PROG1"] = 148;
	chordmap["KEY_PROG2"] = 149;
	chordmap["KEY_WWW"] = 150;
	chordmap["KEY_MSDOS"] = 151;
	chordmap["KEY_COFFEE"] = 152;
	chordmap["KEY_SCREENLOCK"] = KEY_COFFEE;
	chordmap["KEY_DIRECTION"] = 153;
	chordmap["KEY_CYCLEWINDOWS"] = 154;
	chordmap["KEY_MAIL"] = 155;
	chordmap["KEY_BOOKMARKS"] = 156;
	chordmap["KEY_COMPUTER"] = 157;
	chordmap["KEY_BACK"] = 158;
	chordmap["KEY_FORWARD"] = 159;
	chordmap["KEY_CLOSECD"] = 160;
	chordmap["KEY_EJECTCD"] = 161;
	chordmap["KEY_EJECTCLOSECD"] = 162;
	chordmap["KEY_NEXTSONG"] = 163;
	chordmap["KEY_PLAYPAUSE"] = 164;
	chordmap["KEY_PREVIOUSSONG"] = 165;
	chordmap["KEY_STOPCD"] = 166;
	chordmap["KEY_RECORD"] = 167;
	chordmap["KEY_REWIND"] = 168;
	chordmap["KEY_PHONE"] = 169;
	chordmap["KEY_ISO"] = 170;
	chordmap["KEY_CONFIG"] = 171;
	chordmap["KEY_HOMEPAGE"] = 172;
	chordmap["KEY_REFRESH"] = 173;
	chordmap["KEY_EXIT"] = 174;
	chordmap["KEY_MOVE"] = 175;
	chordmap["KEY_EDIT"] = 176;
	chordmap["KEY_SCROLLUP"] = 177;
	chordmap["KEY_SCROLLDOWN"] = 178;
	chordmap["KEY_KPLEFTPAREN"] = 179;
	chordmap["KEY_KPRIGHTPAREN"] = 180;
	chordmap["KEY_NEW"] = 181;
	chordmap["KEY_REDO"] = 182;
	chordmap["KEY_F13"] = 183;
	chordmap["KEY_F14"] = 184;
	chordmap["KEY_F15"] = 185;
	chordmap["KEY_F16"] = 186;
	chordmap["KEY_F17"] = 187;
	chordmap["KEY_F18"] = 188;
	chordmap["KEY_F19"] = 189;
	chordmap["KEY_F20"] = 190;
	chordmap["KEY_F21"] = 191;
	chordmap["KEY_F22"] = 192;
	chordmap["KEY_F23"] = 193;
	chordmap["KEY_F24"] = 194;
	chordmap["KEY_PLAYCD"] = 200;
	chordmap["KEY_PAUSECD"] = 201;
	chordmap["KEY_PROG3"] = 202;
	chordmap["KEY_PROG4"] = 203;
	chordmap["KEY_SUSPEND"] = 205;
	chordmap["KEY_CLOSE"] = 206;
	chordmap["KEY_PLAY"] = 207;
	chordmap["KEY_FASTFORWARD"] = 208;
	chordmap["KEY_BASSBOOST"] = 209;
	chordmap["KEY_PRINT"] = 210;
	chordmap["KEY_HP"] = 211;
	chordmap["KEY_CAMERA"] = 212;
	chordmap["KEY_SOUND"] = 213;
	chordmap["KEY_QUESTION"] = 214;
	chordmap["KEY_EMAIL"] = 215;
	chordmap["KEY_CHAT"] = 216;
	chordmap["KEY_SEARCH"] = 217;
	chordmap["KEY_CONNECT"] = 218;
	chordmap["KEY_FINANCE"] = 219;
	chordmap["KEY_SPORT"] = 220;
	chordmap["KEY_SHOP"] = 221;
	chordmap["KEY_ALTERASE"] = 222;
	chordmap["KEY_CANCEL"] = 223;
	chordmap["KEY_BRIGHTNESSDOWN"] = 224;
	chordmap["KEY_BRIGHTNESSUP"] = 225;
	chordmap["KEY_MEDIA"] = 226;
	chordmap["KEY_SWITCHVIDEOMODE"] = 227;
	chordmap["KEY_KBDILLUMTOGGLE"] = 228;
	chordmap["KEY_KBDILLUMDOWN"] = 229;
	chordmap["KEY_KBDILLUMUP"] = 230;
	chordmap["KEY_SEND"] = 231;
	chordmap["KEY_REPLY"] = 232;
	chordmap["KEY_FORWARDMAIL"] = 233;
	chordmap["KEY_SAVE"] = 234;
	chordmap["KEY_DOCUMENTS"] = 235;
	chordmap["KEY_BATTERY"] = 236;
	chordmap["KEY_BLUETOOTH"] = 237;
	chordmap["KEY_WLAN"] = 238;
	chordmap["KEY_UNKNOWN"] = 240;
	chordmap["KEY_VIDEO_NEXT"] = 241;
	chordmap["KEY_VIDEO_PREV"] = 242;
	chordmap["KEY_BRIGHTNESS_CYCLE"] = 243;
	chordmap["KEY_BRIGHTNESS_ZERO"] = 244;
	chordmap["KEY_DISPLAY_OFF"] = 245;
	chordmap["KEY_OK"] = 0x160;
	chordmap["KEY_SELECT"] = 0x161;
	chordmap["KEY_GOTO"] = 0x162;
	chordmap["KEY_CLEAR"] = 0x163;
	chordmap["KEY_POWER2"] = 0x164;
	chordmap["KEY_OPTION"] = 0x165;
	chordmap["KEY_INFO"] = 0x166;
	chordmap["KEY_TIME"] = 0x167;
	chordmap["KEY_VENDOR"] = 0x168;
	chordmap["KEY_ARCHIVE"] = 0x169;
	chordmap["KEY_PROGRAM"] = 0x16a;
	chordmap["KEY_CHANNEL"] = 0x16b;
	chordmap["KEY_FAVORITES"] = 0x16c;
	chordmap["KEY_EPG"] = 0x16d;
	chordmap["KEY_PVR"] = 0x16e;
	chordmap["KEY_MHP"] = 0x16f;
	chordmap["KEY_LANGUAGE"] = 0x170;
	chordmap["KEY_TITLE"] = 0x171;
	chordmap["KEY_SUBTITLE"] = 0x172;
	chordmap["KEY_ANGLE"] = 0x173;
	chordmap["KEY_ZOOM"] = 0x174;
	chordmap["KEY_MODE"] = 0x175;
	chordmap["KEY_KEYBOARD"] = 0x176;
	chordmap["KEY_SCREEN"] = 0x177;
	chordmap["KEY_PC"] = 0x178;
	chordmap["KEY_TV"] = 0x179;
	chordmap["KEY_TV2"] = 0x17a;
	chordmap["KEY_VCR"] = 0x17b;
	chordmap["KEY_VCR2"] = 0x17c;
	chordmap["KEY_SAT"] = 0x17d;
	chordmap["KEY_SAT2"] = 0x17e;
	chordmap["KEY_CD"] = 0x17f;
	chordmap["KEY_TAPE"] = 0x180;
	chordmap["KEY_RADIO"] = 0x181;
	chordmap["KEY_TUNER"] = 0x182;
	chordmap["KEY_PLAYER"] = 0x183;
	chordmap["KEY_TEXT"] = 0x184;
	chordmap["KEY_DVD"] = 0x185;
	chordmap["KEY_AUX"] = 0x186;
	chordmap["KEY_MP3"] = 0x187;
	chordmap["KEY_AUDIO"] = 0x188;
	chordmap["KEY_VIDEO"] = 0x189;
	chordmap["KEY_DIRECTORY"] = 0x18a;
	chordmap["KEY_LIST"] = 0x18b;
	chordmap["KEY_MEMO"] = 0x18c;
	chordmap["KEY_CALENDAR"] = 0x18d;
	chordmap["KEY_RED"] = 0x18e;
	chordmap["KEY_GREEN"] = 0x18f;
	chordmap["KEY_YELLOW"] = 0x190;
	chordmap["KEY_BLUE"] = 0x191;
	chordmap["KEY_CHANNELUP"] = 0x192;
	chordmap["KEY_CHANNELDOWN"] = 0x193;
	chordmap["KEY_FIRST"] = 0x194;
	chordmap["KEY_LAST"] = 0x195;
	chordmap["KEY_AB"] = 0x196;
	chordmap["KEY_NEXT"] = 0x197;
	chordmap["KEY_RESTART"] = 0x198;
	chordmap["KEY_SLOW"] = 0x199;
	chordmap["KEY_SHUFFLE"] = 0x19a;
	chordmap["KEY_BREAK"] = 0x19b;
	chordmap["KEY_PREVIOUS"] = 0x19c;
	chordmap["KEY_DIGITS"] = 0x19d;
	chordmap["KEY_TEEN"] = 0x19e;
	chordmap["KEY_TWEN"] = 0x19f;
	chordmap["KEY_VIDEOPHONE"] = 0x1a0;
	chordmap["KEY_GAMES"] = 0x1a1;
	chordmap["KEY_ZOOMIN"] = 0x1a2;
	chordmap["KEY_ZOOMOUT"] = 0x1a3;
	chordmap["KEY_ZOOMRESET"] = 0x1a4;
	chordmap["KEY_WORDPROCESSOR"] = 0x1a5;
	chordmap["KEY_EDITOR"] = 0x1a6;
	chordmap["KEY_SPREADSHEET"] = 0x1a7;
	chordmap["KEY_GRAPHICSEDITOR"] = 0x1a8;
	chordmap["KEY_PRESENTATION"] = 0x1a9;
	chordmap["KEY_DATABASE"] = 0x1aa;
	chordmap["KEY_NEWS"] = 0x1ab;
	chordmap["KEY_VOICEMAIL"] = 0x1ac;
	chordmap["KEY_ADDRESSBOOK"] = 0x1ad;
	chordmap["KEY_MESSENGER"] = 0x1ae;
	chordmap["KEY_DISPLAYTOGGLE"] = 0x1af;
	chordmap["KEY_DEL_EOL"] = 0x1c0;
	chordmap["KEY_DEL_EOS"] = 0x1c1;
	chordmap["KEY_INS_LINE"] = 0x1c2;
	chordmap["KEY_DEL_LINE"] = 0x1c3;
	chordmap["KEY_FN"] = 0x1d0;
	chordmap["KEY_FN_ESC"] = 0x1d1;
	chordmap["KEY_FN_F1"] = 0x1d2;
	chordmap["KEY_FN_F2"] = 0x1d3;
	chordmap["KEY_FN_F3"] = 0x1d4;
	chordmap["KEY_FN_F4"] = 0x1d5;
	chordmap["KEY_FN_F5"] = 0x1d6;
	chordmap["KEY_FN_F6"] = 0x1d7;
	chordmap["KEY_FN_F7"] = 0x1d8;
	chordmap["KEY_FN_F8"] = 0x1d9;
	chordmap["KEY_FN_F9"] = 0x1da;
	chordmap["KEY_FN_F10"] = 0x1db;
	chordmap["KEY_FN_F11"] = 0x1dc;
	chordmap["KEY_FN_F12"] = 0x1dd;
	chordmap["KEY_FN_1"] = 0x1de;
	chordmap["KEY_FN_2"] = 0x1df;
	chordmap["KEY_FN_D"] = 0x1e0;
	chordmap["KEY_FN_E"] = 0x1e1;
	chordmap["KEY_FN_F"] = 0x1e2;
	chordmap["KEY_FN_S"] = 0x1e3;
	chordmap["KEY_FN_B"] = 0x1e4;
	chordmap["KEY_BRL_DOT1"] = 0x1f1;
	chordmap["KEY_BRL_DOT2"] = 0x1f2;
	chordmap["KEY_BRL_DOT3"] = 0x1f3;
	chordmap["KEY_BRL_DOT4"] = 0x1f4;
	chordmap["KEY_BRL_DOT5"] = 0x1f5;
	chordmap["KEY_BRL_DOT6"] = 0x1f6;
	chordmap["KEY_BRL_DOT7"] = 0x1f7;
	chordmap["KEY_BRL_DOT8"] = 0x1f8;
	chordmap["KEY_BRL_DOT9"] = 0x1f9;
	chordmap["KEY_BRL_DOT10"] = 0x1fa;
	chordmap["KEY_MIN_INTERESTING"] = KEY_MUTE;
	chordmap["KEY_MAX"] = 0x1ff;
	chordmap["BTN_MISC"] = 0x100;
	chordmap["BTN_0"] = 0x100;
	chordmap["BTN_1"] = 0x101;
	chordmap["BTN_2"] = 0x102;
	chordmap["BTN_3"] = 0x103;
	chordmap["BTN_4"] = 0x104;
	chordmap["BTN_5"] = 0x105;
	chordmap["BTN_6"] = 0x106;
	chordmap["BTN_7"] = 0x107;
	chordmap["BTN_8"] = 0x108;
	chordmap["BTN_9"] = 0x109;
	chordmap["BTN_JOYSTICK"] = 0x120;
	chordmap["BTN_TRIGGER"] = 0x120;
	chordmap["BTN_THUMB"] = 0x121;
	chordmap["BTN_THUMB2"] = 0x122;
	chordmap["BTN_TOP"] = 0x123;
	chordmap["BTN_TOP2"] = 0x124;
	chordmap["BTN_PINKIE"] = 0x125;
	chordmap["BTN_BASE"] = 0x126;
	chordmap["BTN_BASE2"] = 0x127;
	chordmap["BTN_BASE3"] = 0x128;
	chordmap["BTN_BASE4"] = 0x129;
	chordmap["BTN_BASE5"] = 0x12a;
	chordmap["BTN_BASE6"] = 0x12b;
	chordmap["BTN_DEAD"] = 0x12f;
	chordmap["BTN_GAMEPAD"] = 0x130;
	chordmap["BTN_A"] = 0x130;
	chordmap["BTN_B"] = 0x131;
	chordmap["BTN_C"] = 0x132;
	chordmap["BTN_X"] = 0x133;
	chordmap["BTN_Y"] = 0x134;
	chordmap["BTN_Z"] = 0x135;
	chordmap["BTN_TL"] = 0x136;
	chordmap["BTN_TR"] = 0x137;
	chordmap["BTN_TL2"] = 0x138;
	chordmap["BTN_TR2"] = 0x139;
	chordmap["BTN_SELECT"] = 0x13a;
	chordmap["BTN_START"] = 0x13b;
	chordmap["BTN_MODE"] = 0x13c;
	chordmap["BTN_THUMBL"] = 0x13d;
	chordmap["BTN_THUMBR"] = 0x13e;
	chordmap["BTN_DIGI"] = 0x140;
	chordmap["BTN_TOOL_PEN"] = 0x140;
	chordmap["BTN_TOOL_RUBBER"] = 0x141;
	chordmap["BTN_TOOL_BRUSH"] = 0x142;
	chordmap["BTN_TOOL_PENCIL"] = 0x143;
	chordmap["BTN_TOOL_AIRBRUSH"] = 0x144;
	chordmap["BTN_TOOL_FINGER"] = 0x145;
	chordmap["BTN_TOOL_MOUSE"] = 0x146;
	chordmap["BTN_TOOL_LENS"] = 0x147;
	chordmap["BTN_TOUCH"] = 0x14a;
	chordmap["BTN_STYLUS"] = 0x14b;
	chordmap["BTN_STYLUS2"] = 0x14c;
	chordmap["BTN_TOOL_DOUBLETAP"] = 0x14d;
	chordmap["BTN_TOOL_TRIPLETAP"] = 0x14e;
	chordmap["BTN_WHEEL"] = 0x150;
	chordmap["BTN_GEAR_DOWN"] = 0x150;
	chordmap["BTN_GEAR_UP"] = 0x151;

	ConfigFile config (config_file);
	
	if (!(config.readInto(total_controllers, "total_controllers")))
	{
		cerr << "Invalid entry for total_controllers" << endl;
	}

	if (!(config.readInto(device_type, "device_type")))
	{
		cerr << "Invalid entry for controller_type" << endl;
	}

	if (!(config.readInto(device_number, "jsdev")))
	{
		cerr << "Invalid entry for jsdev" << endl;
	}
	
	if (!(config.readInto(total_modes, "total_modes")))
	{
		cerr << "Invalid code for total_modes" << endl;
	}
	
	if (verbose)
	{
		cout << "Using " << config_file << " for configuration information" << endl;
	}
	
		if (!(config.readInto(total_simple_buttons, "total_simple_buttons"))) // we don't care about how many buttons the controller provides, only about how many we have values defined for
	{
		cerr << "Invalid entry for total_simple_buttons" << endl;
	}else{
		if (verbose)
		{
			cout << total_simple_buttons << " simple buttons defined by the config file" << endl; 
		}
	}

	for (int mode_loop = 1; mode_loop <= total_simple_buttons; mode_loop++)
	{	
		ostringstream lbuffer;
		lbuffer << mode_loop;
		string button_name = "simple_b" + lbuffer.str();
		if (!(config.readInto(simple_values[mode_loop], button_name)))
		{
			cerr << "Invalid code entered for " << button_name << " of: " << simple_values[mode_loop] << endl;
		}else{
			if (debug)
			{
				if (1 == mode_loop)
				{
					cout << "Simple Button Values: B" << lbuffer.str() << ": " << simple_values[mode_loop];
				}else{
					cout << " B" << lbuffer.str() << ": " << simple_values[mode_loop];
				}
				if (mode_loop == total_simple_buttons)
				{
					cout << endl;
				}
			}
		}
	}

	if (debug)
	{
		cout << "Done Loading simple config values" << endl;	
	}

	for (int mode_loop = 1; mode_loop <= total_modes; mode_loop++)
	{	
		//lbuffer.str() = "";
		ostringstream lbuffer;
		lbuffer << mode_loop;
		if (debug)
		{
			cout << "Mode " << lbuffer.str() << endl;
		}
		for (int key_loop = 1; key_loop <= total_simple_buttons; key_loop++)
		{// position 0 isn't used on key loop
			ostringstream tbuffer;
			tbuffer << key_loop;
			string itemname = lbuffer.str() + "simple" + tbuffer.str();
			string readvalue = "";
			if (!(config.readInto(readvalue,itemname)))
			{
				cerr << "Invalid entry for value: " << itemname << endl;
			}
			__u16 ukeyvalue = chordmap.find(readvalue)->second;
			simple_modes[mode_loop][key_loop] = ukeyvalue;
			//SOMETHING [mode_loop][key_loop] = readvalue;
			if ((debug) && (readvalue != ""))
			{ // only read valid entries
				cout << "Adding " << readvalue << "[" << ukeyvalue << "]" << " to simple[" << mode_loop << "][" << key_loop << "] " << endl;
			}
		}
	}

	if (!(config.readInto(total_macros, "total_macros")))
	{
		cerr << "Invalid code for total_macros" << endl;
	}
	if (!(config.readInto(total_modifiers, "total_modifiers")))
	{
		cerr << "Invalid code for total_modifiers" << endl;
	}else{
		for(int loadmacro = 1; loadmacro <= total_modifiers; loadmacro++)
		{
			ostringstream mbuffer;
			mbuffer << loadmacro;
			string current_modifier_code = mbuffer.str() + "modifier";	
			string tmpstring = "";
			if (!(config.readInto(tmpstring, current_modifier_code)))
			{
				cerr << " Invalid code for " << current_modifier_code << endl;
			}else{
				__u16 ukeyvalue = chordmap.find(tmpstring)->second;
				modifier[loadmacro] = ukeyvalue;
			}
		}
	}
	
	if (!(config.readInto(total_chorded_buttons, "total_chorded_buttons"))) // we don't care about how many buttons the controller provides, only about how many we have values defined for
	{
		cerr << "Invalid entry for total_chorded_buttons" << endl;
	}else{
		if (verbose)
		{
			cout << total_chorded_buttons << " chorded buttons defined by the config file" << endl; 
		}
	}

	string button_name;
	ostringstream buttonbuffer;
	for (int load_codes = 1; load_codes <= total_chorded_buttons; load_codes++)
	{
		button_name.empty();
		buttonbuffer.str("");
		buttonbuffer << load_codes;
		button_name = "chord_b" + buttonbuffer.str();
		if (!(config.readInto(chord_values[load_codes - 1], button_name)))
		{
			cerr << "Invalid code entered for " << button_name << " of: " << chord_values[load_codes] << endl;
		}
		if (debug)
			{
				if (1 == load_codes)
				{
					cout << "Chord Button Values: B" << buttonbuffer.str() << ": " << chord_values[load_codes];
				}else{
					cout << " B" << buttonbuffer.str() << ": " << chord_values[load_codes];
				}
				if (load_codes == total_chorded_buttons)
				{
					cout << endl;
				}
			}
		}
	
	if (debug)
	{
		cout << "Starting to load chorded config values from 0 to " << pow(2,total_chorded_buttons) << "(2^" << total_chorded_buttons << ")" << endl;
	}
	for (int mode_loop = 1; mode_loop <= total_modes; mode_loop++)
	{	
		ostringstream lbuffer;
		lbuffer << mode_loop;
		string current_mode_code = lbuffer.str() + "modecode";
		if (!(config.readInto(mode_code[mode_loop], current_mode_code )))
		{ // code used for changing modes
			cerr << "Invalid code for " << current_mode_code << endl;
		}
		if (debug)
		{
			cout << "Adding Mode " << mode_code[mode_loop] << " into " << current_mode_code << endl;
		}
		for (int key_loop = 1; key_loop < (pow(2,total_chorded_buttons)); key_loop++)
		{// position 0 isn't used on key loop
			ostringstream tbuffer;
			tbuffer << key_loop;
			string itemname = lbuffer.str() + "chord" + tbuffer.str();
			string readvalue = "";
			if (!(config.readInto(readvalue,itemname)))
			{
				cerr << "Invalid entry for value: " << itemname << endl;
			}
			__u16 ukeyvalue = chordmap.find(readvalue)->second;
			modes[mode_loop][key_loop] = ukeyvalue;
			if ((debug) && (readvalue != ""))
			{ // only read valid entries
				cout << "Adding " << readvalue << "[" << ukeyvalue << "]" << " to chorded[" << mode_loop << "][" << key_loop << "] " << endl;
			}
		}
	}
	if (debug)
	{
		cout << "Done Loading chorded config values" << endl;	
	}
}

