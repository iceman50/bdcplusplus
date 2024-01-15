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

#include "stdinc.h"
#include "BDCText.h"

#include "Client.h"
#include "ClientManager.h"
#include "Magnet.h"
#include "OnlineUser.h"
#include "PluginManager.h"
#include "BDCManager.h"
#include "Tagger.h"
#include "UserMatchManager.h"

namespace dcpp {

void BDCText::addTag(Tagger* tags, string::size_type begin, string::size_type end, const TextElement* aTe, const string& aData) {
	string s = cssStyle((const Style&)(*aTe));
	string attribs = "id=\"element\"";
	string name;
	string xmlTmp;

	if (!s.empty())
		attribs += " style=\"" + SimpleXML::escape(s, xmlTmp, true) + "\"";

	if (!aTe->imageFile.empty()) {
		name = "img";
		attribs += " src=\"" + SimpleXML::escape(aTe->imageFile, xmlTmp, true) + "\"";
		attribs += " alt=\"" + SimpleXML::escape(aData, xmlTmp, true) + "\"";
	}
	else if (!aTe->link.empty()) {
		name = "a";
		attribs += " href=\"" + SimpleXML::escape(aTe->link, xmlTmp, true) + "\"";
	}
	else {
		name = "span";
	}

	tags->add(begin, end, name, attribs);
	if (aTe->textStyle & Bdcpp::STYLE_BOLD) { tags->add(begin, end, "b", ""); }
	if (aTe->textStyle & Bdcpp::STYLE_ITALIC) { tags->add(begin, end, "i", ""); }
	//	if(aTe->textStyle & Bdcpp::STYLE_UNDERLINE_ANY)	{ tags->add(begin, end, "u", "");	} // underline is handled via css
	if (aTe->textStyle & Bdcpp::STYLE_STRIKEOUT) { tags->add(begin, end, "s", ""); }
	if (aTe->textStyle & Bdcpp::STYLE_SUBSCRIPT) { tags->add(begin, end, "sub", ""); }
	if (aTe->textStyle & Bdcpp::STYLE_SUPERSCRIPT) { tags->add(begin, end, "sup", ""); }
}

string BDCText::cssStyle(const Style& aStyle) {
#define CSS_STYLE(type, add, bor) if((aStyle.textStyle & Bdcpp::type) || (bor)) { style += add; }
	string style;
	CSS_STYLE(STYLE_NONE, "font: " + Util::cssFont(aStyle.font) + ";", !aStyle.font.empty());
	CSS_STYLE(STYLE_NONE, "color: #" + Util::cssColor(aStyle.textColor) + ";", (!(aStyle.textStyle & Bdcpp::STYLE_AUTOCOLOR)) && (aStyle.textColor != -1));
	CSS_STYLE(STYLE_NONE, "background-color: #" + Util::cssColor(aStyle.bgColor) + ";", (!(aStyle.textStyle & Bdcpp::STYLE_AUTOBACKCOLOR)) && (aStyle.bgColor != -1));
	CSS_STYLE(STYLE_UNDERLINE_ANY, "text-decoration: underline;", false);
	//	CSS_STYLE(STYLE_STRIKEOUT,			"text-decoration: line-through",								false); // handled via <s> </s> tag
	CSS_STYLE(STYLE_ALLCAPS, "text-transform: uppercase;", false);
	CSS_STYLE(STYLE_UNDERLINE_DOT, "border-bottom-style: dotted;", false);
	CSS_STYLE(STYLE_UNDERLINE_DASH, "border-bottom-style: dashed;", false);
	CSS_STYLE(STYLE_UNDERLINE_THICK, "border-bottom-width: thick;", false);
	return style;
}

string BDCText::htmlSpan(const string& aData, const Style& aStyle /*= Style()*/, const string& aId /*= ""*/, const string& aLink /*= ""*/, const string& aImage /*= ""*/, bool htmlraw /*= false*/) {
	bool				hasLink = !aLink.empty();
	bool				hasImage = !htmlraw && !aImage.empty();
	string				style = cssStyle(aStyle);
	string				data;
	string				xmlTmp;
	std::stringstream	stream;

	if (hasLink) {
		stream << "<a href=\"";
		stream << SimpleXML::escape(aLink, xmlTmp, true);
		stream << "\"";
	}
	else {
		stream << "<span";
	}
	if (!aId.empty()) {
		stream << " id=\"";
		stream << SimpleXML::escape(aId, xmlTmp, true);
		stream << "\"";
	}
	if (!style.empty()) {
		stream << " style=\"";
		stream << SimpleXML::escape(style, xmlTmp, true);
		stream << "\"";
	}
	stream << ">";

	if (aStyle.textStyle & Bdcpp::STYLE_BOLD) { stream << "<b>"; }
	if (aStyle.textStyle & Bdcpp::STYLE_ITALIC) { stream << "<i>"; }
	//	if(aStyle.textStyle & Bdcpp::STYLE_UNDERLINE_ANY)	{ stream << "<u>";		} // underline is handled via css
	if (aStyle.textStyle & Bdcpp::STYLE_STRIKEOUT) { stream << "<s>"; }
	if (aStyle.textStyle & Bdcpp::STYLE_SUBSCRIPT) { stream << "<sub>"; }
	if (aStyle.textStyle & Bdcpp::STYLE_SUPERSCRIPT) { stream << "<sup>"; }

	if (hasImage) {
		stream << "<img src=\"";
		stream << aImage;
		stream << "\" alt=\"";
	}

	if (htmlraw) {
		data = aData;
		// fix <img></img> to <img/>
		string::size_type i;
		string::size_type j = 0;
		while ((i = Bdcpp::strfnd(data, "<img", j)) != string::npos) {
			// we found an <img opening
			j = Bdcpp::strfnd(data, ">", i + 4); // find the end of the tag
			if (j == string::npos) { j = i + 4;	continue; } // must be an exceptionally poor formatted html line
			if (data[j - 1] == '/') { j++;			continue; } // all is cool: <img />

			i = Bdcpp::strfnd(data, "</img>", j + 1);
			if (i == string::npos) { j++;			continue; } // probably another case of bad formatting

			i += 5;
			data.replace(j, i - j, " /", 2);
			j += 3;
		}
	}
	else {
		data = SimpleXML::escape(aData, xmlTmp, hasImage);
	}
	stream << data;

	if (hasImage)
		stream << "\" />";

	if (aStyle.textStyle & Bdcpp::STYLE_SUPERSCRIPT) { stream << "</sup>"; }
	if (aStyle.textStyle & Bdcpp::STYLE_SUBSCRIPT) { stream << "</sub>"; }
	if (aStyle.textStyle & Bdcpp::STYLE_STRIKEOUT) { stream << "</s>"; }
	//	if(aStyle.textStyle & Bdcpp::STYLE_UNDERLINE_ANY)	{ stream << "</u>";		} // underline is handled via css
	if (aStyle.textStyle & Bdcpp::STYLE_ITALIC) { stream << "</i>"; }
	if (aStyle.textStyle & Bdcpp::STYLE_BOLD) { stream << "</b>"; }

	if (hasLink) { stream << "</a>"; }
	else { stream << "</span>"; }

	return stream.str();
}

const string exDash = "  -\t";
const string exBullet = "  *\t";
const string exBullet3 = "  ***\t";

void BDCText::extendedFormatting(string& data, const string& aId) {
#define EXTEND_TEXT(f, r) \
{ \
	string::size_type i, j = 0; \
	while((i = Bdcpp::strfnd(data, f, j)) != string::npos) { \
		string::size_type k = Bdcpp::strfndlstnof(data.c_str()+j, i-j, " \t", 2); \
		i += strlen(f); \
		if(k != string::npos) { \
			j += k; \
			if(data[j] != '\n') { \
				j = i; \
				continue; \
			} \
			j++; \
		} \
		if((k = Bdcpp::strfndfstnof(&(*(data.begin()+i)), &(*data.end()), " \t", 2)) == string::npos) \
				{ i = data.size();	} \
		else	{ i += k;			} \
		data.replace(j, i-j, r); \
		j += r.size(); \
	} \
}

	if (aId == "message") { // only check the message for this
		string::size_type i = 0;
		// copied from ChatMessage.cpp, add "- " for what might be copy/pastes of chats
		// this check does not add "- " on the first line, which could end up in ugly formatting
		while ((i = data.find('\n', i)) != string::npos) {
			i++;
			if (i == data.size())
				break;
			if ((data[i] == '[') || (data[i] == '<')) {
				data.insert(i, "- ");
				i += 2;
			}
		}
	}

	EXTEND_TEXT("- ", exDash);
	EXTEND_TEXT("*** ", exBullet3);
	EXTEND_TEXT("* ", exBullet);
}

// chat message
BDCText::BDCText(const string& aText, OnlineUser* from, bool isPm, bool thirdPerson, const time_t& aMessageTimestamp) {
	loadSettings();

	bool		isUserMatch = BDSETTING(USER_MATCHING_IN_CHAT);
	bool		self = (from->getUser() == ClientManager::getInstance()->getMe());
	Style		userMatch = from->getIdentity().getStyle();
	ParamMap	params;

	params["message"] = [&aText] { return aText; };
	if(SETTING(TIME_STAMPS))
		params["timestamp"] = [] { return Util::getShortTimeString(); };

	if(bdcppformatting) {
		params["pm"] = [&isPm] { return Util::toString(isPm); };
		params["self"] = [&self] { return Util::toString(self); };
		params["3rdperson"] = [&thirdPerson] { return Util::toString(thirdPerson); };

		if(isUserMatch)
			isUserMatch = UserMatchManager::getInstance()->isMatch(*from, true);
		if(aMessageTimestamp)
			params["messageTimestamp"] = [&aMessageTimestamp] { return str(F_("Sent %1%") % Util::getShortTimeString(aMessageTimestamp)); };

		if(BDSETTING(TEXT_FORMAT_ADD_PARAMS)) {
			Bdcpp::getClientParams(&from->getClient(), params);
			from->getClient().getMyIdentity().getParams(params, "my", false);
			params["myGeoIP"] = from->getClient().getMyIdentity().getCountry();
			if (!self) {
				from->getIdentity().getParams(params, "user", false);
				params["userGeoIP"] = from->getIdentity().getCountry();
			}
		} else {
			params["myNI"] = from->getClient().getMyIdentity().getNick();
			if (!self)
				params["userNI"] = from->getIdentity().getNick();
		}
	} else {
		params["userNI"] = (thirdPerson ? "* " + from->getIdentity().getNick() : "<" + from->getIdentity().getNick() + ">");
		isUserMatch = true; // default DC++ formatting adds style to the nick
	}

	append(bdcppformatting ? BDSETTING(FORMAT_CHAT) : BDCManager::getInstance()->getDcppChat(), params, isUserMatch ? &userMatch : nullptr);
}

// chat status message
BDCText::BDCText(const string& aText, bool isPm) {
	loadSettings();
	ParamMap params;
	params["message"] = [&aText] { return aText; };
	params["pm"] = [&isPm] { return Util::toString(isPm); };
	if (SETTING(TIME_STAMPS))
		params["timestamp"] = [] { return Util::getShortTimeString(); };
	append(BDSETTING(BDCPP_TEXT_FORMATTING) ? BDSETTING(FORMAT_CHAT_STATUS) : BDCManager::getInstance()->getDcppDefault(), params);
}

// system log message
BDCText::BDCText(const LogMessage& logMsg) {
	loadSettings();
	ParamMap params;
	params["id"] = [&logMsg] { return Util::toString(logMsg.getId()); };
	params["message"] = [&logMsg] { return logMsg.getText(); };	
	params["timestamp"] = [&logMsg] { return Util::getShortTimeString(logMsg.getTime()); };
	params["type"] = [&logMsg] { return Bdcpp::logTypes[logMsg.getMessageType()]; };
	params["level"] = [&logMsg] { return Bdcpp::logLevels[logMsg.getLogLevel()]; };
	append(BDSETTING(BDCPP_TEXT_FORMATTING) ? BDSETTING(FORMAT_SYSTEMLOG) : BDCManager::getInstance()->getDcppDefault(), params);
}

BDCText::BDCText(const TextFormat& aFormat, ParamMap& params) {
	loadSettings();
	append(aFormat, params);
}

void BDCText::loadSettings() {
	bdcppformatting = BDSETTING(BDCPP_TEXT_FORMATTING);
	extendedformatting = BDSETTING(BDCPP_TEXT_FORMATTING_EXTENDED);
	linksupport = BDSETTING(LINK_FORMATTING);
	friendlylinks = BDSETTING(FRIENDLY_LINKS);
	textelements = BDSETTING(BDCPP_TEXT_ELEMENTS);
}

void BDCText::append(const TextFormat& aFormat, ParamMap& params, const Style* aUserMatchStyle /*= NULL*/, OnlineUser* from /*= NULL*/) {
	string xmlTmp;

	for(auto& fi : aFormat) {
		SdExString::List sdstrings;
		if(!fi->format(sdstrings, params, false, false, true))
			continue;

		for(const auto& sdstring : sdstrings) {
			if(!sdstring.isParam()) {
				plainText += (const string&)(sdstring);
				if (fi.formatWhole) { htmlText += htmlSpan((const string&)(sdstring), (const Style&)(fi), "", (fi.textStyle & Bdcpp::STYLE_LINK) ? (const string&)(sdstring) : ""); }
				else { htmlText += htmlSpan((const string&)(sdstring)); }
				continue;
			}

			const string& id = sdstring.getName();
			bool isNick = (id == "userNI") || (id == "myNI");

			if(isNick && aUserMatchStyle) {
				plainText += (const string&)(sdstring);
				htmlText += htmlSpan((const string&)(sdstring), *aUserMatchStyle, id, (aUserMatchStyle->textStyle & Bdcpp::STYLE_LINK) ? (const string&)(sdstring) : "");
				continue;
			}

			bool isMessage = (id == "message");
			string data = (const string&)(sdstring);		
			
			if (isMessage && extendedformatting) // apply extended formatting only on message
				extendedFormatting(data, id);
			plainText += data; // add data to plaintext before it's modified for html formatting	

			Tagger tags;
			//DiCe !!!! Switch to new Tagger dump and compare contents between old and new and see why there is a difference
			//FIXME
			if (linksupport) { links(&tags, data, (const Style&)(fi)); }
			if (textelements) { BDCManager::getInstance()->checkElementsTagger(&tags, data); }
			if (isMessage && from) { PluginManager::getInstance()->onChatTags(tags, from); } // do plugin stuff only on message
			htmlText += htmlSpan(tags.merge(data, xmlTmp), (const Style&)(fi), id.empty() ? "text" : id, "", "", true);
		}
	}
}

void BDCText::links(Tagger* tags, string& data, const Style& aDefaultStyle, const string::size_type pos /*= 0*/, int protid /* = 0*/) {
	if (protid == Bdcpp::PROTOCOL_LAST)
		return;
	if (pos >= data.size())
		return;

	string::size_type	j = pos;
	string::size_type	i;
	string				xmlTmp;

	while ((i = Bdcpp::strfnd(data, Bdcpp::protocols[protid], j)) != string::npos) {
		char opening = 0x00;
		{
			const auto start = j;
			if (protid == Bdcpp::PROTOCOL_EMAIL) { j = Bdcpp::strfndlstnof(data.substr(0, i), Bdcpp::charsemaillocal, j); }
			else { j = Bdcpp::strfndlstnof(data.substr(0, i), Bdcpp::lettersnumbers, j); }

			if (j == string::npos) { // no limit found
				if (start != 0) { // extra check for a limit before the starting position
					auto k = start - 1;
					if (protid == Bdcpp::PROTOCOL_EMAIL) { k = Bdcpp::strfnd(&(*Bdcpp::charsemaillocal.begin()), &(*Bdcpp::charsemaillocal.end()), data.c_str() + k, 1); }
					else { k = Bdcpp::strfnd(&(*Bdcpp::lettersnumbers.begin()), &(*Bdcpp::lettersnumbers.end()), data.c_str() + k, 1); }

					if (k != string::npos) { // the protocol is not limited before the starting position
						j = i + Bdcpp::protocols[protid].size();
						continue;
					}

					opening = data[start - 1];
				}
				j = start;
			}
			else {
				opening = data[j];
				j++;
			}
		}

		if (protid == Bdcpp::PROTOCOL_MAGNET) {
			if (i != j) { j = i + Bdcpp::protocols[protid].size(); continue; }
			i += Bdcpp::protocols[protid].size();
			auto k = Bdcpp::strfndfstof(data, " \t\r\n<>\"", i);
			if (k == string::npos) { k = data.size(); }
			if (k == i) { j = i; continue; }
			i = k;
		}
		else if (protid == Bdcpp::PROTOCOL_LINK) {
			if (i == j) { j = i + Bdcpp::protocols[protid].size(); continue; }
			i += Bdcpp::protocols[protid].size();
			auto k = Bdcpp::strfndfstnof(data, Bdcpp::charslink, i);
			if (k == string::npos) { k = data.size(); }
			if (k == i) { j = i; continue; }
			i = k;
		}
		else if (protid == Bdcpp::PROTOCOL_MAILTO) {
			if (i != j) { j = i + Bdcpp::protocols[protid].size(); continue; }
			i += Bdcpp::protocols[protid].size();
			auto k = Bdcpp::strfnd(data, "@", i);
			if (k == string::npos) { j = i; continue; }
			i = k + 1;
			k = Bdcpp::strfndfstnof(data, Bdcpp::charsemaildomain, i);
			if (k == string::npos) { k = data.size(); }
			if (k == i) { j = i; continue; }
			if (Bdcpp::strfnd(data.c_str() + i, data.c_str() + k, ".", 1) == string::npos) { j = i; continue; }
			i = k;
		}
		else if (protid == Bdcpp::PROTOCOL_EMAIL) {
			if (j == i) { j = i + Bdcpp::protocols[protid].size(); continue; }
			i += Bdcpp::protocols[protid].size();
			auto k = Bdcpp::strfndfstnof(data, Bdcpp::charsemaildomain, i);
			if (k == string::npos) { k = data.size(); }
			if (k == i) { j = i; continue; }
			if (Bdcpp::strfnd(data.c_str() + i, data.c_str() + k, ".", 1) == string::npos) { j = i; continue; }
			i = k;
		}
		else if (protid == Bdcpp::PROTOCOL_WWW) {
			if (i != j) { j = i + Bdcpp::protocols[protid].size(); continue; }
			i += Bdcpp::protocols[protid].size();
			auto k = Bdcpp::strfndfstnof(data, Bdcpp::charslink, i);
			if (k == string::npos) { k = data.size(); }
			if (k == i) { j = i; continue; }
			i = k;
		}
		else { // nothing valid we can check
			return;
		}

		if (opening != 0x00) {	// extra: if there was an opening bracket/parenthesis, skip the ending one as well
			char ending = 0x00;
			switch (opening) {
			case '(': ending = ')'; break;
			case '[': ending = ']'; break;
			case '{': ending = '}'; break;
			default: break;
			}
			if (ending != 0x00)
				i -= (data[i - 1] == ending);
		}

		string id;
		string link = data.substr(j, i - j);

		if (protid == Bdcpp::PROTOCOL_MAGNET) {
			string hash;
			string name;
			string key;
			if (!Magnet::parseUri(link, hash, name, key)) {
				j = i + 1;
				continue; // skip the whole wannabe magnet line, assume there aren't any other valid protocols in it
			}
			id = "magnet";
			if(friendlylinks) { // make magnet friendly here
				if(!name.empty()) {
					auto k = Bdcpp::strfnd(link, "&xl=");
					if (k == string::npos) {
						name += " (magnet)";
					} else {
						k += 4;
						auto l = Bdcpp::strfnd(link, "&", k);
						if (l == string::npos)
							l = link.size();
						name += " (" + Util::formatBytes(Util::toInt64(link.substr(k, l - k))) + ")";
					}
					// replace the data part with the new name
					data.replace(j, i - j, name);
					i = j + name.size();
				}
			}
		}
		else if (protid == Bdcpp::PROTOCOL_LINK) {
			id = "link";
		}
		else if (protid == Bdcpp::PROTOCOL_MAILTO) {
			id = "email";
			if (friendlylinks) { // erase "mailto:" from data
				data.erase(j, 7);
				i -= 7;
			}
		}
		else if (protid == Bdcpp::PROTOCOL_EMAIL) {
			id = "email";
			link = "mailto:" + link; // make a valid link out of it
		}
		else if (protid == Bdcpp::PROTOCOL_WWW) {
			id = "website";
			link = "http://" + link; // not needed actually
		}

		{
			string d = data.substr(0, j);
			links(tags, d, aDefaultStyle, pos, protid + 1); // check the first part for other protocols
			if (d.size() != j) {
				data.replace(0, j, d);
				i = d.size() + i - j;
				j = d.size();
			}
		}

		Style st = aDefaultStyle;
		st.textColor = SETTING(LINK_COLOR);
		st.textStyle &= ~Bdcpp::STYLE_AUTOCOLOR;
		st.textStyle |= Bdcpp::STYLE_UNDERLINE;

		tags->add(j, i, "a",
			"href=\"" + SimpleXML::escape(link, xmlTmp, true) +
			"\" style=\"" + SimpleXML::escape(cssStyle(st), xmlTmp, true) +
			"\" id=\"" + SimpleXML::escape(id, xmlTmp, true) + "\"");

		if (i == data.size())
			return;
		j = i + 1;
	}

	links(tags, data, aDefaultStyle, j, protid + 1);
}
} // namespace dcpp
