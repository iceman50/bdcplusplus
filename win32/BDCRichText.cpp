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

#include "stdafx.h"
#include "BDCRichText.h"
#include "BDCWinUtil.h"

#include <dcpp/BDCManager.h>
#include <dcpp/Bdcpp.h>
#include <dcpp/BDCText.h>
#include <dcpp/File.h>
#include <dcpp/Text.h>
#include <dcpp/Util.h>

#include <dwt/util/GDI.h>

#include "WinUtil.h"

void BDCRichText::addChatHtml(RichTextBox* aRichText, const string& aHtml) {
	BDCRichText brt(aRichText);
	try {
		SimpleXMLReader(&brt).parse(aHtml);
	}
	catch (const SimpleXMLException& e) {
		Text::toT(e.getError());
		return;
	}
	brt.write(false);
}

void BDCRichText::addChatPlain(RichTextBox* aRichText, const tstring& aLine) {
	addChatHtml(aRichText, BDCText(Text::fromT(aLine), aRichText->isPMChat).getHtmlText());
}

void BDCRichText::addSystemLog(RichTextBox* aRichText, const LogMessage& logMsg) {
	addChatHtml(aRichText, BDCText(logMsg).getHtmlText());
}

void BDCRichText::addExamples(RichTextBox* aRichText, const TextFormat& aFormat, int type) {
#define EXAMPLE(id, pm, person, self) \
	{ \
		TextFormat		tf		= aFormat; \
		StringPairList	spl; \
		ParamMap		params; \
		if(strlen(id) != 0) \
			tf.emplace(tf.begin(), id); \
		getExampleParams(params); \
		params["pm"]		= [] { return Util::toString(pm);		}; \
		params["3rdperson"]	= [] { return Util::toString(person);	}; \
		params["self"]		= [] { return Util::toString(self);		}; \
		addChatHtml(aRichText, BDCText(tf, params).getHtmlText()); \
		for(const auto& fii: aFormat) { \
			if(fii->supportsExample()) \
				fii->get_example(spl, params); \
		} \
		for(const auto& spi: spl) { \
			if(spi.first.empty()) \
				continue; \
			getExampleParams(params); \
			params["pm"]		= [] { return Util::toString(pm);		}; \
			params["3rdperson"]	= [] { return Util::toString(person);	}; \
			params["self"]		= [] { return Util::toString(self);		}; \
			params[spi.first]	= spi.second.empty() ? "testdata" : spi.second; \
			addChatHtml(aRichText, BDCText(tf, params).getHtmlText()); \
		} \
	}

	switch (type) {
	case 0:
	{
		EXAMPLE("user/1st/hub\t", 0, 0, 0);
		EXAMPLE("self/1st/hub\t", 0, 0, 1);
		EXAMPLE("user/3rd/hub\t", 0, 1, 0);
		EXAMPLE("self/3rd/hub\t", 0, 1, 1);
		EXAMPLE("user/1st/pm\t", 1, 0, 0);
		EXAMPLE("self/1st/pm\t", 1, 0, 1);
		EXAMPLE("user/3rd/pm\t", 1, 1, 0);
		EXAMPLE("self/3rd/pm\t", 1, 1, 1);
		break;
	}
	case 1:	EXAMPLE("hub\t", 0, 0, 0); EXAMPLE("pm\t", 1, 0, 0); break;
	case 2:	EXAMPLE("", 0, 0, 0); break;
	}
}

void BDCRichText::getExampleParams(ParamMap& params) {
	params["hubDE"] = [] { return "testhubDE";		};
	params["hubDNS"] = [] { return "testhubDNS";		};
	params["hubGeoIP"] = [] { return "testhubGeoIP";	};
	params["hubI4"] = [] { return "testhubI4";		};
	params["hubI6"] = [] { return "testhubI6";		};
	params["hubNI"] = [] { return "testhubNI";		};
	params["hubURL"] = [] { return "testhubURL";		};

	params["myAW"] = [] { return "testmyAW";		};
	params["myDE"] = [] { return "testmyDE";		};
	params["myEM"] = [] { return "testmyEM";		};
	params["myGeoIP"] = [] { return "testmyGeoIP";	};
	params["myI4"] = [] { return "testmyI4";		};
	params["myI6"] = [] { return "testmyI6";		};
	params["myNI"] = [] { return "testmyNI";		};
	params["myOP"] = [] { return "testmyOP";		};
	params["myRG"] = [] { return "testmyRG";		};
	params["mySS"] = [] { return "testmySS";		};
	params["myU4"] = [] { return "testmyU4";		};
	params["myU6"] = [] { return "testmyU6";		};

	params["userAW"] = [] { return "testuserAW";		};
	params["userDE"] = [] { return "testuserDE";		};
	params["userEM"] = [] { return "testuserEM";		};
	params["userGeoIP"] = [] { return "testuserGeoIP";	};
	params["userI4"] = [] { return "testuserI4";		};
	params["userI6"] = [] { return "testuserI6";		};
	params["userNI"] = [] { return "testuserNI";		};
	params["userOP"] = [] { return "testuserOP";		};
	params["userRG"] = [] { return "testuserRG";		};
	params["userSS"] = [] { return "testuserSS";		};
	params["userU4"] = [] { return "testuserU4";		};
	params["userU6"] = [] { return "testuserU6";		};

	params["timestamp"] = [] { return Util::getShortTimeString();	};
	params["message"]	= [] { return "testmessage";				};
	params["id"]		= [] { return "testid";						};
	params["type"]		= [] { return "testtype";					};
	params["level"]		= [] { return "testlevel";					};
}

BDCRichText::BDCRichText(RichTextBox* aRichText) :
	richText(aRichText), hWnd(aRichText->handle()), limit(::SendMessage(aRichText->handle(), EM_GETLIMITTEXT, 0, 0)), adjustdpi(BDSETTING(ADJUST_CHAT_TO_DPI)),
	crstart(0), crend(0), scroll(true), edit(false),
	lengthhtml(0) {
	config.flags = ST_SELECTION;
	config.codepage = CP_ACP;
	addInitialContext();
}

void BDCRichText::startTag(const string& aName, StringPairList& attribs, bool simple) {
	auto name = boost::algorithm::trim_copy(aName);
	if (name.empty()) // empty tag names are forbidden
		return;

	Context context(name);

	if (name == "b") { context.setBold(true); }
	else if (name == "i") { context.setItalic(true); }
	else if (name == "u") { context.setUnderline(true); }
	else if (name == "s") { context.setStrikeout(true); }
	else if (name == "sub") { context.setSubscript(true); }
	else if (name == "sup") { context.setSuperscript(true); }

	if (!attribs.empty()) {
		const auto& st = getAttrib(attribs, "style", 0);
		State				state = STATE_DECLARATION;
		string::size_type	i = 0;
		string::size_type	j;

		while ((j = st.find_first_of(":;", i)) != string::npos) {
			string tmp(st.c_str() + i, st.c_str() + j);
			i = j + 1;
			boost::algorithm::trim(tmp);

			switch (state) {
			case STATE_DECLARATION:
			{
				if (tmp == "font") { state = STATE_FONT; }
				else if (tmp == "color") { state = STATE_TEXTCOLOR; }
				else if (tmp == "background-color") { state = STATE_BGCOLOR; }
				else if (tmp == "text-decoration") { state = STATE_TEXTDECORATION; }
				else if (tmp == "text-transform") { state = STATE_TEXTTRANSFORM; }
				else if (tmp == "border") { state = STATE_BORDER; }
				else if (tmp == "border-bottom") { state = STATE_BORDERBOTTOM; }
				else if (tmp == "border-bottom-style") { state = STATE_BORDERBOTTOMSTYLE; }
				else if (tmp == "border-bottom-width") { state = STATE_BORDERBOTTOMWIDTH; }
				else { state = STATE_UNKNOWN; }
				continue; // declaration set, continue with the loop
			}
			case STATE_FONT:				decodeHtmlFont(tmp, context);				break;
			case STATE_TEXTCOLOR:			decodeHtmlColor(tmp, false, context);		break;
			case STATE_BGCOLOR:				decodeHtmlColor(tmp, true, context);		break;
			case STATE_TEXTDECORATION:		decodeHtmlTextDecoration(tmp, context);		break;
			case STATE_TEXTTRANSFORM:		decodeHtmlTextTransform(tmp, context);		break;
			case STATE_BORDER:				decodeHtmlBorder(tmp, context);				break;
			case STATE_BORDERBOTTOM:		decodeHtmlBorderBottom(tmp, context);		break;
			case STATE_BORDERBOTTOMSTYLE:	decodeHtmlBorderBottomStyle(tmp, context);	break;
			case STATE_BORDERBOTTOMWIDTH:	decodeHtmlBorderBottomWidth(tmp, context);	break;
			case STATE_UNKNOWN:
			default: break;
			}

			state = STATE_DECLARATION;
		}

		if (name == "a") { context.setLink(getAttrib(attribs, "href", 0)); }
		else if (name == "img") { context.setImage(getAttrib(attribs, "src", 0)); }
	}

	if (!simple) // simple is < /> tag
		addContext(context); // incremental context on stack

	// add these AFTER adding context to the stack
	// newly created data parts should be using the latest context
	if (name == "br") { addData("\\line\n", true); }
	else if (name == "a") { newData(simple, context.getLink(), &context); }
	else if (name == "img") { const string& alt = getAttrib(attribs, "alt", 0); newData(simple, alt.empty() ? context.getImage() : alt, &context); }
}

void BDCRichText::data(const string& aData) {
	addData(aData);
}

void BDCRichText::endTag(const string& aName) {
	auto name = boost::algorithm::trim_copy(aName);
	if (name.empty())
		return;

	removeContext(name);

	// create a new data block
	if (name == "a") { newData(); }
	else if (name == "img") { newData(); } // support faulty <img></img> tags, SimpleXMLReader doesn't know better
}

#define IMAGE_LIMIT 20
void BDCRichText::write(bool doclear /*= true*/) {
	if (lengthhtml == 0)
		return;

	editBegin(lengthhtml);

	tstring	table = Text::toT("{\\fonttbl" + Util::toString(Util::emptyString, fonts) + "}{\\colortbl" + Util::toString(Util::emptyString, colors) + "}");
	size_t	imagecount = 0;
	tstring	message;

	if (richText->length() != 0) {
		string line = "{\\urtf1\n\\line\n}\n";
		::SendMessage(hWnd, EM_SETTEXTEX, reinterpret_cast<WPARAM>(&config), reinterpret_cast<LPARAM>(line.c_str()));
	}

	for (const auto& pi : parts) {
		if (!pi.getImage().empty() && (imagecount < IMAGE_LIMIT)) {
			if (!message.empty()) { // first add the text we already built up
				string line = RichTextBox::escapeUnicode(_T("{\\urtf1\n") + table + message + _T("}\n"));
				::SendMessage(hWnd, EM_SETTEXTEX, reinterpret_cast<WPARAM>(&config), reinterpret_cast<LPARAM>(line.c_str()));
				message.clear();
			}
			bool inserted = insertImage(pi.getImage());
			imagecount += inserted;
			if (inserted)
				continue;
		}

		pi.getRtf(message);
//		LOG_DBG("part.getRTF() returned: " + Text::fromT(message));

		// richedit has the nasty habit of not allowing two links next to each other without any spaces between them
		// lets add an invisible space whenever we've added a link
		if (pi.isLink())
			message += _T("{\\v  }");
	}

	if (!message.empty()) {
		string line = RichTextBox::escapeUnicode(_T("{\\urtf1\n") + table + message + _T("}\n"));
		::SendMessage(hWnd, EM_SETTEXTEX, reinterpret_cast<WPARAM>(&config), reinterpret_cast<LPARAM>(line.c_str()));
	}

	if(doclear) {
		clear();
	}
	editEnd();
}

void BDCRichText::clear() {
	parts.clear();
	contexts.clear();
	fonts.clear();
	colors.clear();
	lengthhtml = 0;
	addInitialContext();
}

void BDCRichText::newData(bool simple /*= false*/, const string& aData /*= Util::emptyString*/, const Context* aContext /*= NULL*/) {
	Context context;
	if (aContext)
		context = *aContext;
	fillContext(context);
	parts.emplace_back(context);
	if (!simple)
		return;
	// new data part is simple, add aData to the data and finish with a new block
	addData(aData);
	newData();
}

void BDCRichText::addData(const string& aData, bool rtfcode /*= false*/) {
	if (aData.empty())
		return;
	if (parts.empty())
		newData();
	if (rtfcode) { parts.back().addData(aData + ' '); } // rtfcode needs a a whitespace at the end
	else { parts.back().addData(dwt::RichTextBox::rtfEscape(Text::toT(Text::toDOS(aData)))); lengthhtml += aData.size(); }
}

size_t BDCRichText::addFont(const string& aFont) {
	if (aFont.empty())
		return fonts.empty() ? 0 : (fonts.size() - 1);

	auto index = fonts.size();
	string font = "{\\f" + std::to_string(index) + aFont;

	{	// lets try to find the same font in the table, ignoring the already given index
		// assume the index is one digit, if not this doesn't save a lot anyway
		auto fi = fonts.begin();
		for (; fi != fonts.end(); ++fi) {
			if (Bdcpp::streql(&(*(fi->begin() + 4)), &(*fi->end()), &(*(font.begin() + 4)), &(*font.end())) == 0) {
				index = fi - fonts.begin();
				break;
			}
		}
		if (fi == fonts.end())
			fonts.emplace_back(font);
	}

	return index;
}

size_t BDCRichText::addColor(const string& aColor) {
	if (aColor.empty())
		return colors.empty() ? 0 : (colors.size() - 1);

	auto index = colors.size();
	auto ci = find(colors.begin(), colors.end(), aColor);

	if (ci == colors.end()) { colors.emplace_back(aColor); }
	else { index = ci - colors.begin(); }

	return index;
}

void BDCRichText::addInitialContext() {
	auto		lf = richText->getFont()->getLogFont();
	COLORREF	tc = richText->getTextColor();
	COLORREF	bc = richText->getBgColor();
	string		font = "\\fnil\\fcharset" + Util::toString(lf.lfCharSet) + " " + Text::fromT(lf.lfFaceName) + ";}";
	size_t		fontsize = size_t(abs(lf.lfHeight) * 72.0 / 96.0 * (adjustdpi ? dwt::util::dpiFactor() : 1) * 2.0);
	string		tcrtf = "\\red" + Util::toString(GetRValue(tc)) + "\\green" + Util::toString(GetGValue(tc)) + "\\blue" + Util::toString(GetBValue(tc)) + ";";
	string		bcrtf = "\\red" + Util::toString(GetRValue(bc)) + "\\green" + Util::toString(GetGValue(bc)) + "\\blue" + Util::toString(GetBValue(bc)) + ";";
	Context		context;

	context.setFontIndex(addFont(font));
	context.setFontSize(fontsize);
	context.setTextColorIndex(addColor(tcrtf));
	context.setBgColorIndex(addColor(bcrtf));
	context.setBold(lf.lfWeight >= FW_BOLD);
	context.setItalic(lf.lfItalic);

	contexts.emplace_back(context);
}

void BDCRichText::addContext(const BDCRichText::Context& aContext) {
	Context oldc;
	Context newc;

	fillContext(oldc);
	contexts.emplace_back(aContext);
	fillContext(newc);

	addData(newc.getRtf(oldc), true); // update rtf text accordingly to the difference between old en new context
}

void BDCRichText::fillContext(BDCRichText::Context& context) {
	auto ci = contexts.end();
	while (ci != contexts.begin()) {
		ci--;
		ci->fillContext(context);
	}
}

void BDCRichText::removeContext(const string& aName) {
	auto ci = contexts.end();
	while (ci != contexts.begin()) {
		ci--;
		if (aName == ci->getName()) {
			Context oldc;
			Context newc;

			fillContext(oldc);
			contexts.erase(ci);
			fillContext(newc);

			addData(newc.getRtf(oldc), true); // update rtf text accordingly to the difference between old en new context
			break;
		}
	}
}

// copied from HtmlToRtf.cpp
void BDCRichText::decodeHtmlFont(const string& aFont, Context& context) {
	StringTokenizer<string> tok(aFont, ' ');
	auto& l = tok.getTokens();

	l.erase(std::remove_if(l.begin(), l.end(), [](const string& s) { return s.empty(); }), l.end());

	auto params = l.size();
	if (params < 2) // the last 2 params (font size & font family) are compulsory.
		return;

	// the last param (font family) may contain spaces; merge if that is the case.
	while ((*(l.back().end() - 1) == '\'') && ((l.back().size() <= 1) || (*l.back().begin() != '\''))) {
		*(l.end() - 2) += ' ' + std::move(l.back());
		l.erase(l.end() - 1);
		if (l.size() < 2)
			return;
	}

	// parse the last param (font family).
	auto& family = l.back();
	family.erase(std::remove(family.begin(), family.end(), '\''), family.end());
	if (family.empty())
		return;

	context.setFontIndex(addFont("\\fnil " + family + ";}")); // need to add "{\\" + Util::toString(fonts.size()) on saving the font

	// parse the second to last param (font size).
	auto& size = *(l.end() - 2);
	if ((size.size() > 2) && (*(size.end() - 2) == 'p') && (*(size.end() - 1) == 'x')) { // 16px
		context.setFontSize(size_t(
			abs(Util::toFloat(size.substr(0, size.size() - 2))) * 72.0 / 96.0 // px -> font points
			* (BDSETTING(ADJUST_CHAT_TO_DPI) ? dwt::util::dpiFactor() : 1) // respect DPI settings
			* 2.0)); // RTF font sizes are expressed in half-points
	}

	if (params > 2) {
		context.setBold(Util::toInt(*(l.end() - 3)) >= FW_BOLD);
		context.setItalic(l[0] == "italic");
	}
}

// copied from HtmlToRtf.cpp
void BDCRichText::decodeHtmlColor(const string& aColor, bool isBg, Context& context) {
	auto sharp = aColor.find('#');
	if ((sharp != string::npos) && (aColor.size() > (sharp + 6))) {
		try {
			size_t pos = 0;
			auto c = std::stol(aColor.substr(sharp + 1, 6), &pos, 16);
			string color =
				"\\red" + Util::toString((c & 0xFF0000) >> 16) +
				"\\green" + Util::toString((c & 0xFF00) >> 8) +
				"\\blue" + Util::toString(c & 0xFF) + ";";
			if (isBg) { context.setBgColorIndex(addColor(color)); }
			else { context.setTextColorIndex(addColor(color)); }
		}
		catch (const std::exception& e) {
			dcdebug("color parsing exception: %s with str: %s\n", e.what(), aColor.c_str());
		}
	}
}

void BDCRichText::decodeHtmlTextDecoration(const string& aDecoration, Context& context) {
	StringTokenizer<string> st(aDecoration, ' ');
	const auto& sl = st.getTokens();
	for (const auto& si : sl) {
		if (si.empty())
			continue;
		if (aDecoration == "underline") { context.setUnderline(true); }
		else if (aDecoration == "line-through") { context.setStrikeout(true); }
		else if (aDecoration == "none") {
			context.setUnderline(false);
			context.setStrikeout(false);
		}
	}
}

void BDCRichText::decodeHtmlTextTransform(const string& aTransform, Context& context) {
	if (aTransform == "uppercase") { context.setAllcaps(true); }
	else if (aTransform == "none") {
		context.setAllcaps(false);
	}
}

void BDCRichText::decodeHtmlBorder(const string& aBorderInfo, Context& context) {
	decodeHtmlBorderBottom(aBorderInfo, context);
}

void BDCRichText::decodeHtmlBorderBottom(const string& aBorderInfo, Context& context) {
	StringTokenizer<string> st(aBorderInfo, ' ');
	const auto& sl = st.getTokens();
	for (const auto& si : sl) {
		if (si.empty())
			continue;
		decodeHtmlBorderBottomStyle(si, context);
		decodeHtmlBorderBottomWidth(si, context);
	}
}

void BDCRichText::decodeHtmlBorderBottomStyle(const string& aBorderStyle, Context& context) {
	if (aBorderStyle == "dotted") { context.setUnderlineDot(true); }
	else if (aBorderStyle == "dashed") { context.setUnderlineDash(true); }
	else if (aBorderStyle == "none") {
		context.setUnderlineDot(false);
		context.setUnderlineDash(false);
	}
}

void BDCRichText::decodeHtmlBorderBottomWidth(const string& aBorderWidth, Context& context) {
	if (aBorderWidth == "thick") { context.setUnderlineThick(true); }
	else if (aBorderWidth == "none") {
		context.setUnderlineThick(false);
	}
	else if (aBorderWidth.size() > 2) {
		size_t size = aBorderWidth.size();
		if ((aBorderWidth[size - 1] == 'p') && (aBorderWidth[size - 1] == 'x')) {
			int width = Util::toInt(boost::algorithm::trim_copy(aBorderWidth.substr(0, size - 2)));
			// lets consider 3px and larger "thick"
			if (width > 2) { context.setUnderlineThick(true); }
			else { context.setUnderlineThick(false); }
		}
	}
}

void BDCRichText::editBegin(size_t addlen) {
	if (edit)
		return;
	edit = true;

	size_t curlen = richText->length();

	::SendMessage(hWnd, EM_GETSCROLLPOS, 0, reinterpret_cast<LPARAM>(&scrollPos));
	::SendMessage(hWnd, EM_GETSEL, reinterpret_cast<WPARAM>(&crstart), reinterpret_cast<LPARAM>(&crend));

	{
		SCROLLINFO scrollInfo = { sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE | SIF_POS };
		if (::GetScrollInfo(hWnd, SB_VERT, &scrollInfo))
		{
			scroll = (!scrollInfo.nPage || (scrollInfo.nPos >= static_cast<int>(scrollInfo.nMax - scrollInfo.nPage)));
		}
		else { scroll = false; }
	}

	if (!scroll)
		::SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);

	if ((curlen + addlen) > limit) {
		unsigned charsremoved = 0;
		POINTL pt;

		if (scroll)
			::SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);

		if (addlen >= limit) {
			charsremoved = unsigned(curlen);
		}
		else {
			int multiplier = 1;
			while (charsremoved < addlen)
				charsremoved = ::SendMessage(hWnd, EM_LINEINDEX, ::SendMessage(hWnd, EM_LINEFROMCHAR, multiplier++ * limit / 10, 0), 0);
		}

		::SendMessage(hWnd, EM_POSFROMCHAR, reinterpret_cast<WPARAM>(&pt), charsremoved);
		::SendMessage(hWnd, EM_SETSEL, 0, charsremoved);
		::SendMessage(hWnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(_T("")));

		if (scroll)
			::SendMessage(hWnd, WM_SETREDRAW, TRUE, 0);

		scrollPos.y -= pt.y;
		crstart -= charsremoved;
		crend -= charsremoved;
	}

	::SendMessage(hWnd, EM_SETSEL, WPARAM(-1), LPARAM(-1));
}

void BDCRichText::editEnd() {
	if(!edit)
		return;
	edit = false;

	if(scroll) {
		::SendMessage(hWnd, EM_SETSEL, WPARAM(-1), LPARAM(-1));
		::SendMessage(hWnd, EM_SCROLLCARET, 0, 0);
		::SendMessage(hWnd, EM_SETSEL, crstart, crend);
		::SendMessage(hWnd, WM_VSCROLL, SB_BOTTOM, 0);
		/*DiCe Test -- Scroll to bottom on login since currently it is broken ... 
			TODO: if(bool firsttime) { richText->scrollToBottom(); } 
		*/
		//richText->scrollToBottom();
	} else {
		::SendMessage(hWnd, EM_SETSEL, crstart, crend);
		::SendMessage(hWnd, EM_SETSCROLLPOS, 0, reinterpret_cast<LPARAM>(&scrollPos));
		::SendMessage(hWnd, WM_SETREDRAW, TRUE, 0);
	}

	::RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
}

BOOL BDCRichText::insertImage(const string& aFile) {
	BOOL			bHandled = TRUE;
	SCODE			sc = 0;
	HBITMAP			hBitmap = NULL;
	ImageObject*	pImageObject = NULL;
	LPDATAOBJECT	lpDataObject = NULL; // only used as a container for pImageObject
	IStorage*		pStorage = NULL;
	LPRICHEDITOLE	pRichEditOle = NULL;
	IOleClientSite* pOleClientSite = NULL;
	IOleObject*		pOleObject = NULL;
	CLSID			clsid = CLSID_NULL;
	tstring			file;

	if(File::isAbsolute(aFile)) {
		file = Text::toT(Util::validateFileName(aFile));
	} else { 
		file = Text::toT(Util::validateFileName(Util::getPath(Util::PATH_GLOBAL_CONFIG) + aFile));
	}

	if(file.substr(file.size() - 4) == _T(".ico")) {// icons
		hBitmap = getBitmapFromIcon(file, richText->getBgColor());
	} else {
		hBitmap = getMaskedBitmap(file, richText->getBgColor());
	}

	if (!hBitmap)
		return FALSE;

#define SCODE_TEST(res) if((sc = res) != S_OK) { throw sc; }

	try {
		pImageObject = new ImageObject();
		SCODE_TEST(pImageObject->QueryInterface(IID_IDataObject, (void**)(&lpDataObject)));
		SCODE_TEST(pImageObject->SetBitmap(hBitmap));
		SCODE_TEST(pImageObject->CreateStorage(&pStorage));
		SCODE_TEST(pImageObject->GetOle(hWnd, pStorage, &pRichEditOle, &pOleClientSite, &pOleObject));
		SCODE_TEST(pOleObject->GetUserClassID(&clsid));

		REOBJECT reobject;
		ZeroMemory(&reobject, sizeof(REOBJECT));
		reobject.cbStruct = sizeof(REOBJECT);
		reobject.clsid = clsid;
		reobject.cp = REO_CP_SELECTION;
		reobject.dvaspect = DVASPECT_CONTENT;
		reobject.dwFlags = REO_BELOWBASELINE; //| REO_DYNAMICSIZE | REO_RESIZABLE;
		reobject.poleobj = pOleObject;
		reobject.polesite = pOleClientSite;
		reobject.pstg = pStorage;

		pRichEditOle->InsertObject(&reobject);
	} catch (const Exception&) {
		bHandled = FALSE;
	}

	if (pOleObject) { pOleObject->Release(); }
	if (pOleClientSite) { pOleClientSite->Release(); }
	if (pStorage) { pStorage->Release(); }
	if (lpDataObject) { lpDataObject->Release(); }
	if (pRichEditOle) { pRichEditOle->Release(); }
	if (hBitmap) { DeleteObject(hBitmap); }

	if(!bHandled) {
		delete pImageObject;
	}

	return bHandled;
}

//TODO BDCRichText::insertImage(const string& aFile, const long size) {
//TODO RICHEDIT_IMAGE_PARAMETERS stores x and y as HIMETRIC so convert size to HIMETRIC
void BDCRichText::insertImage() {
	//dwt::Point pt { 16, 16 };
	//dwt::Point pt2;
	//LPSIZEL himetric = AtlPixelToHiMetric(pt, pt2);

	auto pixelToHimet = [](const SIZEL* lpPix, SIZEL* lpHiMetric) -> void {
		HDC dc = ::GetDC(NULL);
		lpHiMetric->cx = 100 * lpPix->cx / ::GetDeviceCaps( dc, LOGPIXELSX );
		lpHiMetric->cy = 100 * lpPix->cy / ::GetDeviceCaps( dc, LOGPIXELSY );
		::ReleaseDC(NULL, dc);
	};
	
	RICHEDIT_IMAGE_PARAMETERS rip;
	IStream *pStream = nullptr;
	DWORD grfMode = STGM_READ | STGM_SHARE_DENY_NONE;
	HRESULT hr = SHCreateStreamOnFileEx(_T("X:\\Projects\\icons\\test.png"), grfMode, FILE_ATTRIBUTE_NORMAL, FALSE, nullptr, &pStream);
	if(!SUCCEEDED(hr)) { LOG_DBG("SHCreateStreamOnFileEx failed"); }
	if(hr == S_OK) {
		ZeroMemory(&rip, sizeof(rip));
		rip.xWidth = 350;
		rip.yHeight = 350;
		rip.Type = TA_BASELINE;
		rip.pwszAlternateText = _T("test");
		rip.pIStream = pStream;
		hr = ::SendMessage(hWnd, EM_INSERTIMAGE, (WPARAM)&rip, 0);
		if(hr == S_OK) {
			::SendMessage(hWnd, EM_SETSEL, 0, 0);
		}
	}
}

HBITMAP BDCRichText::getBitmapFromIcon(const tstring& aFile, COLORREF crBgColor) {
	int size = BDSETTING(ICON_SIZE);

	HICON hIcon = HICON(::LoadImage(NULL, aFile.c_str(), IMAGE_ICON, size, size, LR_LOADFROMFILE));
	if (!hIcon)
		return NULL;

	HDC		crtdc = GetDC(NULL);
	HDC		memdc = CreateCompatibleDC(crtdc);
	HBITMAP	hBitmap = CreateCompatibleBitmap(crtdc, size, size);
	HBITMAP	hOldBitmap = SelectBitmap(memdc, hBitmap);
	HBRUSH	hBrush = CreateSolidBrush(crBgColor);
	RECT	rect = { 0, 0, size, size };

	FillRect(memdc, &rect, hBrush);
	DrawIconEx(memdc, 0, 0, hIcon, size, size, 0, NULL, DI_NORMAL); // DI_NORMAL will automatically alpha-blend the icon over the memdc

	SelectBitmap(memdc, hOldBitmap);
	ReleaseDC(NULL, crtdc); // GetDC --> ReleaseDC
	DeleteDC(memdc); // CreateCompatibleDC --> DeleteDC
	DestroyIcon(hIcon); // HICON --> DestroyIcon
	DeleteObject(hBrush);

	return hBitmap;
}

HBITMAP BDCRichText::getMaskedBitmap(const tstring& aFile, COLORREF crBgColor) {
	HBITMAP	hBitmap = HBITMAP(::LoadImage(NULL, aFile.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	if(!hBitmap) {
		return NULL;
	}

	HDC	srcdc = CreateCompatibleDC(NULL);
	HDC	mskdc = CreateCompatibleDC(NULL);
	HBITMAP	hbmMask = createMask(hBitmap);
	HBITMAP oldHbmSrc = SelectBitmap(srcdc, hBitmap);
	HBITMAP oldHbmMsk = SelectBitmap(mskdc, hbmMask);
	BITMAP bm;
	GetObject(hBitmap, sizeof(bm), &bm);

	SetBkColor(srcdc, crBgColor);
	SetTextColor(srcdc, RGB(0, 0, 0));
	BitBlt(srcdc, 0, 0, bm.bmWidth, bm.bmHeight, mskdc, 0, 0, SRCPAINT);

	SelectBitmap(srcdc, oldHbmSrc);
	SelectBitmap(mskdc, oldHbmMsk);
	DeleteDC(srcdc);
	DeleteDC(mskdc);
	DeleteObject(hbmMask);

	return hBitmap;
}

HBITMAP BDCRichText::createMask(HBITMAP hBitmap) {
	HDC	srcdc = CreateCompatibleDC(NULL);
	HDC	tgtdc = CreateCompatibleDC(NULL);
	BITMAP bm;
	GetObject(hBitmap, sizeof(bm), &bm);
	HBITMAP hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);
	HBITMAP oldHbmSrc = SelectBitmap(srcdc, hBitmap);
	HBITMAP	oldHbmTgt = SelectBitmap(tgtdc, hbmMask);
	COLORREF crTrans = GetPixel(srcdc, 0, 0);

	SetBkColor(srcdc, crTrans);
	BitBlt(tgtdc, 0, 0, bm.bmWidth, bm.bmHeight, srcdc, 0, 0, SRCCOPY);
	SetBkColor(srcdc, RGB(0, 0, 0));
	SetTextColor(srcdc, RGB(255, 255, 255));
	BitBlt(srcdc, 0, 0, bm.bmWidth, bm.bmHeight, tgtdc, 0, 0, SRCAND);

	SelectBitmap(srcdc, oldHbmSrc);
	SelectBitmap(tgtdc, oldHbmTgt);
	DeleteDC(srcdc);
	DeleteDC(tgtdc);

	return hbmMask;
}
