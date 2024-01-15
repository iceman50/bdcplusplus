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
#include "BDCManager.h"

#include "BDCText.h"
#include "SdEx.h"
#include "UserMatchManager.h"
#include "Tagger.h"

namespace dcpp {

const string BDCManager::settingTags[] = {
	// booleans
	"AddUserMatchingToTextElements",
	"AjustChatToDPi",
	"FriendlyLinks",
	"LinkFormatting",
	"BdcppTextElements",
	"BdcppTextFormatting",
	"BdcppTextFormattingExtended",
	"TabsOnBottom",
	"TextFormatAddParams",
	"UserMatchingInChat",
	"FlashTaskBarOnPM",
	"EnableSUDP",
	"EnableIconTheming",
	"ShowHubNameInPMChatStatus",
	// integers
	"ActionDoubleclickUser",
	"IconSize",
	// strings
	"CustomTabTextHubFormat",
	"IconPath",
	// formats
	"FormatChat",
	"FormatChatStatus"
};

vector<SdEx*> BDCManager::sdexhooks;
boost::shared_mutex BDCManager::mutexsdexhooks;

BDCManager::BDCManager() {
	for(int i = BOOL_FIRST; i != BOOL_LAST; ++i)
		boolSettings[i - BOOL_FIRST] = true;
	for(int i = INT_FIRST; i != INT_LAST; ++i)
		intSettings[i - INT_FIRST] = 0;

	set(ADD_USER_MATCHING_TO_TEXT_ELEMENTS,	true);
	set(ADJUST_CHAT_TO_DPI,					true);
	set(FRIENDLY_LINKS,						true);
	set(LINK_FORMATTING,					true);
	set(BDCPP_TEXT_ELEMENTS,				true);
	set(BDCPP_TEXT_FORMATTING,				true);
	set(BDCPP_TEXT_FORMATTING_EXTENDED,		true);
	set(TABS_ON_BOTTOM,						false);
	set(TEXT_FORMAT_ADD_PARAMS,				true);
	set(USER_MATCHING_IN_CHAT,				true);
	set(ACTION_DOUBLECLICK_USER,			Bdcpp::ACTION_GETLIST);
	set(ICON_SIZE,							24);
	set(CUSTOM_TABTEXT_HUB_FORMAT,			"%[tabtext] - %[hubDE] (%[hubGeoIP])");
	set(FLASH_TASKBAR_ON_PM,				true);
	set(ENABLE_SUDP,						true);
	set(ENABLE_ICON_THEMING, 				false);
	set(ICON_PATH,							Util::emptyString);
	set(SHOW_HUB_IN_PM_CHATSTATUS,			false);

	defaultFormatChat();
	defaultFormatChatStatus();
	defaultFormatSystemLog();

	dcppChat.emplace_back("\\[%[timestamp]\\] ");
	dcppChat.emplace_back("%[userNI] ");
	dcppChat.emplace_back("%[message]");

	dcppDefault.emplace_back("\\[%[timestamp]\\] ");
	dcppDefault.emplace_back("%[message]");
}

#define ADD_TEXTFORMATINFO(pattern, crtext, crbg, textstyle) { Style style(Util::emptyString, crtext, crbg, textstyle); tf.emplace_back(pattern, style); }

void BDCManager::defaultFormatChat() {
	TextFormat tf;
	ADD_TEXTFORMATINFO("%[timestamp] ",											RGB(0x00,0xFF,0x00), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR | Bdcpp::STYLE_ITALIC);
	ADD_TEXTFORMATINFO("\\[%[messageTimestamp]\\] ",							RGB(0x00,0xFF,0x00), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR | Bdcpp::STYLE_ITALIC);
	ADD_TEXTFORMATINFO("%[{validate(all)=self,0;pm,1}](%[hubDNS]) ",			RGB(0x00,0x80,0xFF), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR | Bdcpp::STYLE_BOLD);
	ADD_TEXTFORMATINFO("%[{validate(all)=self,1;pm,1}](%[hubDNS]) ",			RGB(0x40,0x40,0xFF), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR | Bdcpp::STYLE_BOLD);
	ADD_TEXTFORMATINFO("%[{validate(all)=self,0;3rdperson,0}]%[userNI] says: ",	RGB(0x00,0x80,0xFF), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR | Bdcpp::STYLE_BOLD);
	ADD_TEXTFORMATINFO("%[{validate(all)=self,0;3rdperson,1}]--> %[userNI] ",	RGB(0x00,0x80,0xFF), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR | Bdcpp::STYLE_BOLD);
	ADD_TEXTFORMATINFO("%[{validate(all)=self,1;3rdperson,0}]%[myNI] says: ",	RGB(0x40,0x40,0xFF), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR | Bdcpp::STYLE_BOLD);
	ADD_TEXTFORMATINFO("%[{validate(all)=self,1;3rdperson,1}]--> %[myNI] ",		RGB(0x40,0x40,0xFF), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR | Bdcpp::STYLE_BOLD);
	ADD_TEXTFORMATINFO("%[{validate(all)=self,0}]%[message]",					RGB(0x00,0x80,0xFF), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR);
	ADD_TEXTFORMATINFO("%[{validate(all)=self,1}]%[message]",					RGB(0x80,0x00,0xFF), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR);
	set(FORMAT_CHAT, tf);
}

void BDCManager::defaultFormatChatStatus() {
	TextFormat tf;
	ADD_TEXTFORMATINFO("%[timestamp] ",						RGB(0xFF,0x00,0x00), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR | Bdcpp::STYLE_ITALIC);
	ADD_TEXTFORMATINFO("%[{validate(all)=pm,0}]%[message]",	RGB(0xDD,0xA0,0xDD), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR);
	ADD_TEXTFORMATINFO("%[{validate(all)=pm,1}]%[message]",	RGB(0x00,0xFF,0x7F), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR);
	set(FORMAT_CHAT_STATUS, tf);
}

//TODO Do better default formatting for Log Messages
void BDCManager::defaultFormatSystemLog() {
	TextFormat tf;
	ADD_TEXTFORMATINFO("\\[%[timestamp]\\] ",	 RGB(0xFF,0x00,0x00), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR | Bdcpp::STYLE_BOLD);
	ADD_TEXTFORMATINFO("\\[Log ID: %[id]\\] ",	 RGB(0x00,0xFF,0x00), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR);
	ADD_TEXTFORMATINFO("\\[Type: %[type]\\] ",	 RGB(0x00,0xFF,0x00), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR);
	ADD_TEXTFORMATINFO("\\[Level: %[level]\\] ", RGB(0x00,0xFF,0x00), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR);
	ADD_TEXTFORMATINFO("%[message] ",			 RGB(0xFF,0x00,0xFF), RGB(0xFF,0xFF,0xFF), Bdcpp::STYLE_AUTOBACKCOLOR);
	set(FORMAT_SYSTEMLOG, tf);
}

void BDCManager::updateElements(const TextElement::List* aElements /*= NULL*/) {
	TextElement::List tel;
	if(aElements)	{ tel = *aElements;								}
	else			{ ReadLock lock(mutexelements); tel = elements;	} // copy the current list for editing

	// first, remove any previous usermatch element
	for(size_t i = 0; i != tel.size();) {
		if((tel.begin()+i)->isUserMatch) {
			tel.erase(tel.begin()+i);
			continue;
		}
		i++;
	}

	if(get(ADD_USER_MATCHING_TO_TEXT_ELEMENTS)) { // then add the new ones from the list
		const auto& umlist = UserMatchManager::getInstance()->getList();
		for(const auto& um: umlist) {
			for(const auto& r: um.rules) { // for each rule we add an element, since we can only check one rule (in text lines) at a time anyway
				if(r.pattern.empty())
					continue;

				switch(r.field) {
				case UserMatch::Rule::NICK:
				case UserMatch::Rule::CID:
				case UserMatch::Rule::IP: break;
				default: continue;
				}

				int flags = 0;
				switch(r.getMethod()) {
				case StringMatch::EXACT:	flags = Bdcpp::DELIMIT_EQUALS; break;
				case StringMatch::PARTIAL:	flags = Bdcpp::DELIMIT_CONTAINS; break;
				default: continue;
				}

				string sdex = "%[{delimit";
				if(r.getMethod() == StringMatch::PARTIAL) // also not case sensitive
					sdex += "(nocase)";
				sdex += "=";
				{
					string tmp = r.pattern;
					sdex += SdEx()->escape_data(tmp);
				}
				sdex += ',';
				sdex += Util::toString(flags);
				{
					string tmp = " \r\n\t";
					if(r.field == UserMatch::Rule::NICK)
						tmp += "<>";
					sdex += ',';
					sdex += SdEx()->escape_data(tmp);
				}
				sdex += "}]";
				tel.emplace_back("(user matching)", sdex, um.style, "", "", true);
			}
		}
	}

	// move elements back to the original list
	WriteLock lock(mutexelements);
	const_cast<TextElement::List&>(elements) = std::forward<TextElement::List>(tel);
}

void BDCManager::checkElementsTagger(Tagger* tags, const string& aData, string::size_type pos /*= 0*/, size_t epos /*= 0*/) const {
	if(pos >= aData.size())
		return;

	ReadLock lock(mutexelements); 

	if(epos >= elements.size()) // no elements to check
		return;

	auto				pbegin	= aData.c_str() + pos;
	auto				pend	= aData.c_str() + aData.size();
	auto				ei		= elements.begin() + epos; // should be valid
	auto				eiend	= elements.end();
	SdExString::List	sdstrings;

	// ipos in sdstrings will be relative to starting position pos
	while(!(*ei)->check(sdstrings, pbegin, pend)) {
		ei++;
		if(ei == eiend)
			return;
		sdstrings.clear();
	}

	epos = ei - elements.begin(); // update element position

	for(const auto& sdstring: sdstrings) {
		const auto& ipos = sdstring.getInitialPos();
		const auto& ilen = sdstring.getInitialLength();

		if(sdstring.isParam()) {
			BDCText::addTag(tags, pos+ipos, pos+ipos+ilen, &(*ei), (const string&)(sdstring));
			continue;
		}
		 // check all the non-elements for text elements
		checkElementsTagger(tags, aData.substr(0, pos+ipos+ilen), pos+ipos, epos + (ipos == 0)); // make sure the same element doesn't get checked on the same data
	}
}

void BDCManager::load() {
	if(loadFile(Bdcpp::getBdcppSettingsFile()))
		return;
}

void BDCManager::save() const {
	saveFile(Bdcpp::getBdcppSettingsFile());
}

bool BDCManager::loadFile(const string& aFile) {
	return loadBdcppFile(aFile,
		[this](SimpleXML& xml)
	{
		xml.resetCurrentChild();
		if(xml.findChild("Settings")) {
			xml.stepIn();

			for(int key = BOOL_FIRST; key != BOOL_LAST; ++key) {
				xml.resetCurrentChild();
				string tag = settingTags[key - SETTING_FIRST];
				if(xml.findChild(tag))
					set(BoolSetting(key), Util::toInt(xml.getChildData()) != 0);
			}

			for(int key = INT_FIRST; key != INT_LAST; ++key) {
				xml.resetCurrentChild();
				if(xml.findChild(settingTags[key - SETTING_FIRST]))
					set(IntSetting(key), Util::toInt(xml.getChildData()));
			}

			for(int key = STR_FIRST; key != STR_LAST; ++key) {
				xml.resetCurrentChild();
				if(xml.findChild(settingTags[key - SETTING_FIRST])) {
					switch(key) {
					default: set(StringSetting(key), xml.getChildData()); break;
					}
				}
			}

			xml.stepOut();
		}

		TextFormatList tfl;
		load(xml, tfl);
		for(auto& tfi: tfl) {
			int key = getFormatSetting(tfi.first);
			if(key == FORMAT_LAST) // the identifier is not known to us
				continue;
			set(FormatSetting(key), tfi.second);
		}

		TextElement::List tel;
		load(xml, tel);
		setElements(tel);
	} );
}

bool BDCManager::saveFile(const string& aFile) const {
	return saveBdcppFile(aFile,
		[this](SimpleXML& xml)
	{
		xml.addTag("Settings");
		xml.stepIn();
		{
			for(int key = BOOL_FIRST; key != BOOL_LAST; ++key)	{ xml.addTag(settingTags[key - SETTING_FIRST], get(BoolSetting(key)));		xml.addChildAttrib("Type", string("bool"));		}
			for(int key = INT_FIRST; key != INT_LAST; ++key)	{ xml.addTag(settingTags[key - SETTING_FIRST], get(IntSetting(key)));		xml.addChildAttrib("Type", string("int"));		}
			for(int key = STR_FIRST; key != STR_LAST; ++key)	{ xml.addTag(settingTags[key - SETTING_FIRST], get(StringSetting(key)));	xml.addChildAttrib("Type", string("string"));	}
		}
		xml.stepOut();

		TextFormatList tfl;
		for(int key = FORMAT_FIRST; key != FORMAT_LAST; ++key)
			tfl.emplace_back(settingTags[key - SETTING_FIRST], get(FormatSetting(key)));
		save(xml, tfl);

		save(xml, elements);
	} );
}

void BDCManager::load(SimpleXML& xml, BDCManager::TextFormatList& tfl) {
	xml.resetCurrentChild();

	const string f1 = "TextFormats";
	if(xml.findChild(f1)) {
		xml.stepIn();

		const string f2 = "TextFormat";
		while(xml.findChild(f2)) {
			string type = xml.getChildAttrib("Type");
			xml.stepIn();

			TextFormat tf;
			const string f3 = "TextFormatInfo";
			while(xml.findChild(f3)) {
				string sdex = getSdEx(xml.getChildData(), false);
				Style style(xml.getChildAttrib("Font"), xml.getIntChildAttrib("TextColor"), xml.getIntChildAttrib("BgColor"), xml.getIntChildAttrib("TextStyle"));
				tf.emplace_back(sdex, style, xml.getBoolChildAttrib("FormatWhole"));
			}
			tfl.emplace_back(type, tf);

			xml.stepOut();
		}

		xml.stepOut();
	}
}

void BDCManager::save(SimpleXML& xml, const BDCManager::TextFormatList& aTfl) {
	xml.addTag("TextFormats");
	xml.stepIn();
	{
		for(auto& tfi: aTfl) {
			xml.addTag("TextFormat");
			xml.addChildAttrib("Type", tfi.first);
			xml.stepIn();

			for(auto& fii: tfi.second) {
				xml.addTag("TextFormatInfo",		fii.getPattern());
				xml.addChildAttrib("Font",			fii.font);
				xml.addChildAttrib("TextColor",		fii.textColor);
				xml.addChildAttrib("BgColor",		fii.bgColor);
				xml.addChildAttrib("TextStyle",		fii.textStyle);
				xml.addChildAttrib("FormatWhole",	fii.formatWhole);
			}

			xml.stepOut();
		}
	}
	xml.stepOut();
}

void BDCManager::load(SimpleXML& xml, TextElement::List& tel) {
	xml.resetCurrentChild();
	if(xml.findChild("TextElements")) {
		xml.stepIn();

		while(xml.findChild("TextElement")) {
			string sdex = getSdEx(xml.getChildData(), false);
			Style style(xml.getChildAttrib("Font"), xml.getIntChildAttrib("TextColor"), xml.getIntChildAttrib("BgColor"), xml.getIntChildAttrib("TextStyle"));
			tel.emplace_back(xml.getChildAttrib("Name"), sdex, style, xml.getChildAttrib("ImageFile"), xml.getChildAttrib("Link"));
		}

		xml.stepOut();
	}
}

void BDCManager::save(SimpleXML& xml, const TextElement::List& aTel) {
	xml.addTag("TextElements");
	xml.stepIn();
	{
		for(auto& tei: aTel) {
			if(tei.isUserMatch)
				continue;
			xml.addTag("TextElement",		tei.getPattern());
			xml.addChildAttrib("Name",		tei.name);
			xml.addChildAttrib("Font",		tei.font);
			xml.addChildAttrib("TextColor",	tei.textColor);
			xml.addChildAttrib("BgColor",	tei.bgColor);
			xml.addChildAttrib("TextStyle",	tei.textStyle);
			xml.addChildAttrib("ImageFile",	tei.imageFile);
			xml.addChildAttrib("Link",		tei.link);
		}
	}
	xml.stepOut();
}

string BDCManager::getSdEx(const string& aPattern, bool fixEscapes) {
	string p = aPattern;
	if(fixEscapes) {
		string::size_type i = 0;
		while((i = Bdcpp::strfnd(p, "\\", i)) != string::npos) {
			p.insert(i, "\\");
			i += 2;
		}
	}
	return p;
}

} // namespace dcpp
