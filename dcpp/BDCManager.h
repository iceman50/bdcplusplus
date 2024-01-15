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

#ifndef DCPLUSPLUS_BDCPP_MANAGER_H
#define DCPLUSPLUS_BDCPP_MANAGER_H

#include "File.h"
#include "LogManager.h"
#include "Bdcpp.h"
#include "SimpleXML.h"
#include "Singleton.h"
#include "typedefs.h"
#include "version.h"

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace dcpp {

class SdEx;
class Tagger;

class BDCManager : public Singleton<BDCManager> {
public:
	enum BoolSetting {
		SETTING_FIRST,
		BOOL_FIRST = SETTING_FIRST,
		ADD_USER_MATCHING_TO_TEXT_ELEMENTS = BOOL_FIRST,
		ADJUST_CHAT_TO_DPI,
		FRIENDLY_LINKS,
		LINK_FORMATTING,
		BDCPP_TEXT_ELEMENTS,
		BDCPP_TEXT_FORMATTING,
		BDCPP_TEXT_FORMATTING_EXTENDED,
		TABS_ON_BOTTOM,
		TEXT_FORMAT_ADD_PARAMS,
		USER_MATCHING_IN_CHAT,
		FLASH_TASKBAR_ON_PM,
		ENABLE_SUDP,
		ENABLE_ICON_THEMING,
		SHOW_HUB_IN_PM_CHATSTATUS,
		BOOL_LAST
	};

	enum IntSetting {
		INT_FIRST = BOOL_LAST,
		ACTION_DOUBLECLICK_USER = INT_FIRST,
		ICON_SIZE,
		INT_LAST
	};

	enum StringSetting {
		STR_FIRST = INT_LAST,
		CUSTOM_TABTEXT_HUB_FORMAT = STR_FIRST,
		ICON_PATH,
		STR_LAST,		
	};

	enum FormatSetting {
		FORMAT_FIRST = STR_LAST,
		FORMAT_CHAT = FORMAT_FIRST,
		FORMAT_CHAT_STATUS,
		FORMAT_SYSTEMLOG,
		FORMAT_LAST,
		SETTING_LAST = FORMAT_LAST
	};

	typedef vector<pair<string, TextFormat> > TextFormatList;
	typedef boost::shared_lock<boost::shared_mutex> ReadLock;
	typedef boost::unique_lock<boost::shared_mutex> WriteLock;

	static const string& getSettingTag(int key) { dcassert(key >= SETTING_FIRST); dcassert(key < SETTING_LAST); return settingTags[key - SETTING_FIRST]; }
	static FormatSetting getFormatSetting(const string& aTag) { for(int key = FORMAT_FIRST; key != FORMAT_LAST; ++key) { if(settingTags[key - SETTING_FIRST] == aTag) { return FormatSetting(key); } } return FORMAT_LAST; }

	static void addHook(SdEx* aSdEx) { WriteLock lock(mutexsdexhooks); sdexhooks.emplace_back(aSdEx); }
	static void removeHook(SdEx* aSdEx) { WriteLock lock(mutexsdexhooks); dcassert(find(sdexhooks.begin(), sdexhooks.end(), aSdEx) != sdexhooks.end()); sdexhooks.erase(find(sdexhooks.begin(), sdexhooks.end(), aSdEx)); }
	static void updateSdEx() { ReadLock lock(mutexsdexhooks); for(auto& si: sdexhooks) { si->update(); } }

	static void loadChatFormats(const string& aFile, TextFormatList& tfl) { loadBdcppFile(aFile, [&tfl](SimpleXML& xml) { load(xml, tfl); } ); };
	static void saveChatFormats(const string& aFile, const TextFormatList& aTfl) { saveBdcppFile(aFile, [&aTfl](SimpleXML& xml) { save(xml, aTfl); } ); };
	static void loadTextElements(const string& aFile, TextElement::List& tel) { loadBdcppFile(aFile, [&tel](SimpleXML& xml) { load(xml, tel); } ); };
	static void saveTextElements(const string& aFile, const TextElement::List& aTel) { saveBdcppFile(aFile, [&aTel](SimpleXML& xml) { save(xml, aTel); } ); };

	bool get(BoolSetting key) const { return boolSettings[key - BOOL_FIRST]; }
	int get(IntSetting key) const { return intSettings[key - INT_FIRST]; }
	const string& get(StringSetting key) const { return stringSettings[key - STR_FIRST]; }
	const TextFormat& get(FormatSetting key) const { return formatSettings[key - FORMAT_FIRST]; }
	const TextFormat& getDcppChat() const { return dcppChat; }
	const TextFormat& getDcppDefault() const { return dcppDefault; }

	void set(const BoolSetting& key, bool aData) { boolSettings[key - BOOL_FIRST] = aData; }
	void set(const IntSetting& key, int aData) { intSettings[key - INT_FIRST] = aData; }
	void set(const StringSetting& key, const string& aData) { stringSettings[key - STR_FIRST] = aData; }
	void set(const FormatSetting& key, const TextFormat& aData) { formatSettings[key - FORMAT_FIRST] = aData; }

	const TextFormat& resetDefault(const FormatSetting& key) {
		switch(key) {
		case FORMAT_CHAT:			defaultFormatChat(); break;
		case FORMAT_CHAT_STATUS:	defaultFormatChatStatus(); break;
		case FORMAT_SYSTEMLOG:		defaultFormatSystemLog(); break;
		case FORMAT_LAST:
		default: break;
		}
		return formatSettings[key - FORMAT_FIRST];
	}

	const TextElement::List& getElements() const { ReadLock lock(mutexelements); return elements; }
	void setElements(const TextElement::List& aElements) { updateElements(&aElements); }
	void updateElements(const TextElement::List* aElements = NULL);
	void checkElementsTagger(Tagger* tags, const string& aData, string::size_type pos = 0, size_t epos = 0) const;

	void load();
	void save() const;

	bool loadFile(const string& aFile);
	bool saveFile(const string& aFile) const;

private:

	friend class Singleton<BDCManager>;
	BDCManager();
	virtual ~BDCManager() { }

	static const string settingTags[SETTING_LAST - SETTING_FIRST];
	static vector<SdEx*> sdexhooks; // need a static list because SdEx objects get created before anything else
	static boost::shared_mutex mutexsdexhooks;

	bool boolSettings[BOOL_LAST - BOOL_FIRST];
	int intSettings[INT_LAST - INT_FIRST];
	string stringSettings[STR_LAST - STR_FIRST];
	TextFormat formatSettings[FORMAT_LAST - FORMAT_FIRST];
	TextElement::List elements;
	TextFormat dcppChat;
	TextFormat dcppDefault;
	mutable boost::shared_mutex mutexelements;

	bool isSet[SETTING_LAST];

	void defaultFormatChat();
	void defaultFormatChatStatus();
	void defaultFormatSystemLog();

	static void load(SimpleXML& xml, TextFormatList& tfl);
	static void save(SimpleXML& xml, const TextFormatList& aTfl);
	static void load(SimpleXML& xml, TextElement::List& tel);
	static void save(SimpleXML& xml, const TextElement::List& aTel);
	static string getSdEx(const string& aPattern, bool fixEscapes);

	template<class Func>
	static bool loadBdcppFile(const string& aFile, Func func) {
		try {
			SimpleXML xml;

			xml.fromXML(File(aFile, File::READ, File::OPEN).read());

			xml.resetCurrentChild();

			string versionstr = xml.getChildAttrib("Version");
			double version = Bdcpp::toDouble(versionstr);

			if (!xml.findChild("BDCPlusPlus"))
				throw Exception(_(APPNAME " error: not a valid " APPNAME " file: ") + aFile);

			double current = VERSIONFLOAT;

			if (version > current)
				LogManager::getInstance()->message(_(APPNAME " warning: xml version is newer than the " APPNAME " version: ") + aFile);

			xml.stepIn();

			func(xml);

			xml.stepOut();
		} catch(const Exception& e) {
			LogManager::getInstance()->message(e.getError());
			return false;
		}

		return true;
	}

	template<class Func>
	static bool saveBdcppFile(const string& aFile, Func func) {
		SimpleXML xml;
		xml.addTag("BDCPlusPlus");
		xml.addChildAttrib("Version", Util::toString(VERSIONFLOAT));

		xml.stepIn();
		{	// xml file created, save the settings given by the lambda function
			func(xml);
		}
		xml.stepOut();

		try {
			File out(aFile + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
			BufferedOutputStream<false> f(&out);
			f.write(SimpleXML::utf8Header);
			xml.toXML(&f);
			f.flush();
			out.close();
			File::deleteFile(aFile);
			File::renameFile(aFile + ".tmp", aFile);
		} catch(const FileException&) {
			return false;
		}

		return true;
	}
};

#define BDSETTING(x) BDCManager::getInstance()->get(BDCManager::x)

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_BDCPP_MANAGER_H)
