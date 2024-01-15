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

#ifndef DCPLUSPLUS_BDCPP_H
#define DCPLUSPLUS_BDCPP_H

#include "forward.h"
#include "LogMessage.h"
#include "SdEx.h"
#include "Style.h"
#include "typedefs.h"

namespace dcpp {

class Bdcpp {
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

	enum {
//		style ID					hex code		rtf code	CHARFORMAT2 effect	CHARFORMAT2 mask
		STYLE_BOLD					= 0x00000001, // \b			CFE_BOLD			CFM_BOLD
		STYLE_ITALIC				= 0x00000002, // \i			CFE_ITALIC			CFM_ITALIC
		STYLE_UNDERLINE				= 0x00000004, // \ul		CFE_UNDERLINE		CFM_UNDERLINE
		STYLE_STRIKEOUT				= 0x00000008, // \strike	CFE_STRIKEOUT		CFM_STRIKEOUT
//		STYLE_PROTECTED				= 0x00000010, //			CFE_PROTECTED		CFM_PROTECTED
		STYLE_LINK					= 0x00000020, //			CFE_LINK			CFM_LINK		{\field{\*\fldinst HYPERLINK "link"}{\fldrslt text}
//		STYLE_SMALLCAPS				= 0x00000040, // \scaps		CFE_SMALLCAPS		CFM_SMALLCAPS
		STYLE_ALLCAPS				= 0x00000080, // \caps		CFE_ALLCAPS			CFM_ALLCAPS
//		STYLE_HIDDEN				= 0x00000100, // \v			CFE_HIDDEN			CFM_HIDDEN
//		STYLE_OUTLINE				= 0x00000200, // \outl		CFE_OUTLINE			CFM_OUTLINE
//		STYLE_SHADOW				= 0x00000400, // \shad		CFE_SHADOW			CFM_SHADOW
//		STYLE_EMBOSS				= 0x00000800, // \embo		CFE_EMBOSS			CFM_EMBOSS
//		STYLE_IMPRINT				= 0x00001000, // \impr		CFE_IMPRINT			CFM_IMPRINT
//		STYLE_DISABLED				= 0x00002000, //			CFE_DISABLED		CFM_DISABLED
//		STYLE_REVISED				= 0x00004000, // \deleted	CFE_REVISED			CFM_REVISED
//									= 0x00008000, //								CFM_REVAUTHOR
		STYLE_SUBSCRIPT				= 0x00010000, // \sub		CFE_SUBSCRIPT		(CFM_SUBSCRIPT = CFE_SUBSCRIPT | CFE_SUPERSCRIPT)
		STYLE_SUPERSCRIPT			= 0x00020000, // \super		CFE_SUPERSCRIPT		CFM_SUPERSCRIPT
		STYLE_UNDERLINE_DOT			= 0x00040000, // \uld							CFM_ANIMATION
		STYLE_UNDERLINE_DASH		= 0x00080000, // \uldash						CFM_STYLE
//		STYLE_UNDERLINE_DOTDASH		= 0x00100000, // \uldashd						CFM_KERNING
//		STYLE_UNDERLINE_DOTDOTDASH	= 0x00200000, // \uldashdd						CFM_SPACING
		STYLE_UNDERLINE_THICK		= 0x00400000, // \ulth							CFM_WEIGHT
//		STYLE_UNDERLINE_WAVE		= 0x00800000, // \ulwave						CFM_UNDERLINETYPE
//									= 0x01000000, //								
//									= 0x02000000, //								CFM_LCID
		STYLE_AUTOBACKCOLOR			= 0x04000000, // \cb0		CFE_AUTOBACKCOLOR	CFM_BACKCOLOR
//									= 0x08000000, //								CFM_CHARSET
//									= 0x10000000, //								CFM_OFFSET
//									= 0x20000000, //								CFM_FACE
		STYLE_AUTOCOLOR				= 0x40000000, // \cf0		CFE_AUTOCOLOR		CFM_COLOR
		STYLE_AUTOFONT				= 0x80000000,  //								CFM_SIZE

		STYLE_NONE					= 0x00000000,
		STYLE_UNDERLINE_ANY			= STYLE_UNDERLINE | STYLE_UNDERLINE_DOT | STYLE_UNDERLINE_DASH | STYLE_UNDERLINE_THICK,
// others
//		STYLE_NOSUPERSUB			= .x........, // \nosupersub
//		STYLE_RESET					= .x........, // \plain
//		STYLE_UNDERLINE_NONE		= .x........, // \ulnone
// non-working styles:
//		STYLE_ANIMATION_LASVEGAS	= .x........, // \animtext1
//		STYLE_ANIMATION_BLINKINGBG	= .x........, // \animtext2
//		STYLE_ANIMATION_SPARKLE		= .x........, // \animtext3
//		STYLE_ANIMATION_BLACKANTS	= .x........, // \animtext4
//		STYLE_ANIMATION_REDANTS		= .x........, // \animtext5
//		STYLE_ANIMATION_SHIMMER		= .x........, // \animtext6
//		STYLE_STRIKEOUT_DOUBLE		= .x........, // \strikedl
//		STYLE_UNDERLINE_WORD		= .x........, // \ulw
//		STYLE_UNDERLINE_DOUBLE		= .x........, // \uldb
	};

	enum {
		DELIMIT_EQUALS		= 0x01,
		DELIMIT_CONTAINS	= 0x02,
		DELIMIT_ONLY		= 0x04,
		DELIMIT_STARTS		= 0x08,
		DELIMIT_ENDS		= 0x10,
		// legacy support
		DELIMIT_NOCASE		= 0x20
	};

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

	static const string actions[ACTION_LAST];
	static const string protocols[PROTOCOL_LAST];
	static const string logTypes[LogMessage::Type::TYPE_LAST];
	static const string logLevels[LogMessage::Level::LOG_LAST];

	static const string charsemaillocal;
	static const string charsemaildomain;
	static const string charslink;
	static const string lettersnumbers;
	static const string digitshex;
	static const string digitsnumbers;
	static const string delimiters;

	static string getBdcppSettingsFile();

	static void getClientParams(const Client* aClient, ParamMap& params);
	static double toDouble(const string& aDouble);
	static int32_t toInt32(const string& aInt32);
	static int64_t toInt64(const string& aInt64);

	static void stripWhitespace(const string::value_type** ppbegin, const string::value_type** ppend);
	static string& escape(string& line);
	static string& checkEscape(string& line);

	static char hexToChar(const string::value_type* pbegin, const string::value_type* pend);
	static void charToHex(const char& c, string& hex);
	
	static const string& genUuid();
	
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(streql);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strcmp);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strfnd);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strfndfstof);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strfndfstnof);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strfndlstof);
	BDCPP_PCHARMETHOD_WITH_OVERLOADS(strfndlstnof);
};

class TextFormatInfo : public SdEx, public Style {
public:
	TextFormatInfo(const string& aFormat, const Style& aStyle = Style(), bool aFormatWhole = false) : SdEx(aFormat), Style(aStyle), formatWhole(aFormatWhole) { }
	TextFormatInfo(const TextFormatInfo& rhs) : SdEx(rhs), Style(rhs), formatWhole(rhs.formatWhole) { }
	~TextFormatInfo() { }

	bool formatWhole;
};

typedef vector<TextFormatInfo> TextFormat;

class TextElement : public SdEx, public Style {
public:
	typedef vector<TextElement> List;

	TextElement(const string& aName, const string& aPattern, const Style& aStyle = Style(), const string& aImageFile = "", const string& aLink = "", bool aIsUserMatch = false) :
		SdEx(aPattern), Style(aStyle), name(aName), imageFile(aImageFile), link(aLink), isUserMatch(aIsUserMatch) { }
	TextElement(const TextElement& rhs) : SdEx(rhs), Style(rhs), name(rhs.name), imageFile(rhs.imageFile), link(rhs.link), isUserMatch(rhs.isUserMatch) { }
	~TextElement() { }

	string name;
	string imageFile;
	string link;
	bool isUserMatch;
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_BDCPP_H)
