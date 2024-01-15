
/*	0017: SdExParameters.cpp
 *
 *	This file, with all its containing structures, has been
 *	created by Agent 0017, unless otherwise specified.
 *
 *	For all intents and purposes the normal DC++ license applies,
 *	as my addition to it is free for all to use. Also, there is
 *	no guarantee that my addition to the original program will be
 *	flawless or even useful. The reason for the additions is to
 *	enhance the original program and work out new ideas.
 *
 *	If you decide to use/modify my additions to DC++, just some
 *	credits to my original programming would do fine.
 *
 *		Agent 0017
 *		0017agent@gmail.com
 */

#include "stdinc.h"
#include "SdEx.h"

namespace dcpp {

StringList SdEx::getParamsTabText() {
	StringList sl;
	sl.push_back("tabtext");
	sl.push_back("hubDE");
	sl.push_back("hubDNS");
	sl.push_back("hubGeoIP");
	sl.push_back("hubI4");
	sl.push_back("hubNI");
	sl.push_back("hubURL");
	return sl;
}

StringList SdEx::getParamsTextFormat() {
	StringList sl;
	sl.push_back("timestamp");
	sl.push_back("message");
	sl.push_back("pm");
	sl.push_back("self");
	sl.push_back("3rdperson");
	sl.push_back("hubDE");
	sl.push_back("hubDNS");
	sl.push_back("hubGeoIP");
	sl.push_back("hubI4");
	sl.push_back("hubNI");
	sl.push_back("hubURL");
	sl.push_back("myAW");
	sl.push_back("myDE");
	sl.push_back("myEM");
	sl.push_back("myGeoIP");
	sl.push_back("myI4");
	sl.push_back("myNI");
	sl.push_back("myOP");
	sl.push_back("myRG");
	sl.push_back("mySS");
	sl.push_back("myU4");
	sl.push_back("userAW");
	sl.push_back("userBO");
	sl.push_back("userDE");
	sl.push_back("userEM");
	sl.push_back("userGeoIP");
	sl.push_back("userI4");
	sl.push_back("userNI");
	sl.push_back("userOP");
	sl.push_back("userRG");
	sl.push_back("userSS");
	sl.push_back("userU4");
	return sl;
}

StringList SdEx::getParamsWinamp() {
	StringList sl;
	sl.push_back("artist");
	sl.push_back("bitrate");
	sl.push_back("channels");
	sl.push_back("elapsed");
	sl.push_back("eqautoload");
	sl.push_back("eqband1");
	sl.push_back("eqband2");
	sl.push_back("eqband3");
	sl.push_back("eqband4");
	sl.push_back("eqband5");
	sl.push_back("eqband6");
	sl.push_back("eqband7");
	sl.push_back("eqband8");
	sl.push_back("eqband9");
	sl.push_back("eqband10");
	sl.push_back("eqpreamp");
	sl.push_back("equalizer");
	sl.push_back("length");
	sl.push_back("playlistlen");
	sl.push_back("playlistpos");
	sl.push_back("pos");
	sl.push_back("progress");
	sl.push_back("repeat");
	sl.push_back("sample");
	sl.push_back("shuffle");
	sl.push_back("status");
	sl.push_back("title");
	sl.push_back("track");
	sl.push_back("trackno");
	sl.push_back("version");
	return sl;
}
	
}
