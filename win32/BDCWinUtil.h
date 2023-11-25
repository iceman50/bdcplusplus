/*
* Copyright (C) 2023 iceman50
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef DCPLUSPLUS_WIN32_BDC_UTIL_H
#define DCPLUSPLUS_WIN32_BDC_UTIL_H

#include <dcpp/LogMessage.h>
#include <dcpp/Util.h>

using namespace dcpp;

class BDCWinUtil {

public:
#define BDCPP_PCHARMETHOD_WITH_OVERLOADS(func) \
	static string::size_type func(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend); \
	static string::size_type func(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, size_t dlen) { return func(pbegin, pend, dbegin, dbegin+dlen); } \
	static string::size_type func(const string::value_type* pbegin, size_t plen, const string::value_type* dbegin, const string::value_type* dend) { return func(pbegin, pbegin+plen, dbegin, dend); } \
	static string::size_type func(const string::value_type* pbegin, size_t plen, const string::value_type* dbegin, size_t dlen) { return func(pbegin, pbegin+plen, dbegin, dbegin+dlen); } \
	static string::size_type func(const string& aLine, const string& aData, size_t aPos = 0) { \
		if((aPos == string::npos) || (aPos > aLine.size())) \
			return string::npos; \
		auto i = func(&(*(aLine.begin()+aPos)), &(*aLine.end()), &(*aData.begin()), &(*aData.end())); \
		return (i == string::npos) ? string::npos : (i + aPos); \
	}

	//Stats
	static bool getSysInfo(tstring& line);
	static tstring getCPUInfo();
	static tstring getOSInfo();
	static TStringList findVolumes();
	static tstring diskSpaceInfo(bool onlyTotal = false);
	static tstring diskInfoList();
	static bool getNetStats(tstring& line);
	static tstring formatTimeDifference(uint64_t diff, size_t levels = 3);
	static time_t getStartTime() { return startTime; }

	BDCPP_PCHARMETHOD_WITH_OVERLOADS(streql);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strcmp);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strfnd);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strfndfstof);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strfndfstnof);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strfndlstof);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strfndlstnof);

	//UI
	enum {
		ACTION_GETLIST = 0,
		ACTION_BROWSELIST,
		ACTION_MATCHQUEUE,
		ACTION_PRIVATEMESSAGE,
		ACTION_ADDFAVORITE,
		ACTION_GRANTSLOT,
		ACTION_REMOVEFROMQUEUE,
		ACTION_IGNORECHAT,
		ACTION_UNIGNORECHAT,
		ACTION_LAST
	};

	enum {
		PROTOCOL_MAGNET = 0,
		PROTOCOL_LINK,
		PROTOCOL_MAILTO,
		PROTOCOL_EMAIL,
		PROTOCOL_WWW,
		PROTOCOL_LAST
	};

	enum {
		//BDC++ style ID					hex code		rtf code	CHARFORMAT2 effect	CHARFORMAT2 mask
		STYLE_BOLD = 0x00000001,			// \b			CFE_BOLD			CFM_BOLD
		STYLE_ITALIC = 0x00000002,			// \i			CFE_ITALIC			CFM_ITALIC
		STYLE_UNDERLINE = 0x00000004,		 // \ul		CFE_UNDERLINE		CFM_UNDERLINE
		STYLE_STRIKEOUT = 0x00000008,		// \strike	CFE_STRIKEOUT		CFM_STRIKEOUT
		STYLE_LINK = 0x00000020, //							CFE_LINK			CFM_LINK		{\field{\*\fldinst HYPERLINK "link"}{\fldrslt text}
		STYLE_ALLCAPS = 0x00000080,			// \caps		CFE_ALLCAPS			CFM_ALLCAPS
		STYLE_SUBSCRIPT = 0x00010000, // \sub		CFE_SUBSCRIPT		(CFM_SUBSCRIPT = CFE_SUBSCRIPT | CFE_SUPERSCRIPT)
		STYLE_SUPERSCRIPT = 0x00020000, // \super		CFE_SUPERSCRIPT		CFM_SUPERSCRIPT
		STYLE_UNDERLINE_DOT = 0x00040000, // \uld							CFM_ANIMATION
		STYLE_UNDERLINE_DASH = 0x00080000, // \uldash						CFM_STYLE
		STYLE_UNDERLINE_THICK = 0x00400000, // \ulth							CFM_WEIGHT
		STYLE_AUTOBACKCOLOR = 0x04000000, // \cb0		CFE_AUTOBACKCOLOR	CFM_BACKCOLOR
		STYLE_AUTOCOLOR = 0x40000000, // \cf0		CFE_AUTOCOLOR		CFM_COLOR
		STYLE_AUTOFONT = 0x80000000,  //								CFM_SIZE

		STYLE_NONE = 0x00000000,
		STYLE_UNDERLINE_ANY = STYLE_UNDERLINE | STYLE_UNDERLINE_DOT | STYLE_UNDERLINE_DASH | STYLE_UNDERLINE_THICK,
	};

	static const tstring actions[ACTION_LAST];
	static const tstring logType[LogMessage::Type::TYPE_LAST];
	static const tstring logLevel[LogMessage::Level::LOG_LAST];
	static const string protocols[PROTOCOL_LAST];

	static const string charsemaillocal;
	static const string charsemaildomain;
	static const string charslink;
	static const string lettersnumbers;
	static const string digitshex;
	static const string digitsnumbers;
	static const string delimiters;

private:
	static time_t startTime;
};

#endif // DCPLUSPLUS_WIN32_BDC_UTIL_H