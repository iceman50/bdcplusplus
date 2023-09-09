/*
* Copyright (C) 2001-2022 Jacek Sieka, arnetheduck on gmail point com
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

#include "resource.h"

#include "HoldRedraw.h"
#include "ThemePage.h"

#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/Link.h>

#include <dcpp/File.h>
#include <dcpp/format.h>
#include <dcpp/LogManager.h>
#include <dcpp/ScopedFunctor.h>
#include <dcpp/SimpleXML.h>
#include <dcpp/version.h>

#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;
using dwt::Label;
using dwt::Link;

enum { COLUMN_TEXT, COLUMN_LAST};
static const ColumnInfo tcol[COLUMN_LAST] = {
	{ "", 100, false }
};

ThemePage::ThemeData::ThemeData(tstring text, int textColorSetting, int bgColorSetting) :
	text(text),
	textColorSetting(textColorSetting),
	bgColorSetting(bgColorSetting)
{
}

ThemePage::ThemePage(dwt::Widget* parent) :
PropPage(parent, 1, 1),
table(0),
//installed(0),
themeInfo(0),
cbCtrlTheme(0),
expTheme(0),
modTheme(0)
{
	currentTheme();

	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;

	{
		auto group = grid->addChild(GroupBox::Seed(T_("Theme preview")));

		auto cur = group->addChild(Grid::Seed(2, 1));
		cur->column(0).mode = GridInfo::FILL;
		cur->row(0).mode = GridInfo::FILL;
		cur->row(0).align = GridInfo::STRETCH;
		cur->row(1).mode = GridInfo::FILL;
		cur->row(1).align = GridInfo::STRETCH;
		cur->setSpacing(grid->getSpacing());

			{
				auto seed = WinUtil::Seeds::Dialog::table;
				seed.style &= ~LVS_SHOWSELALWAYS;
				seed.style |= LVS_SINGLESEL | LVS_NOCOLUMNHEADER;
				table = cur->addChild(Table::Seed(seed));

				dwt::ImageListPtr images(new dwt::ImageList(dwt::Point(16, 16)));
				images->add(*WinUtil::createIcon(IDI_DCPP, 16));
				images->add(*WinUtil::createIcon(IDI_NET_STATS, 16)); /// @todo better icon for the "transfers" group?
				images->add(*WinUtil::createIcon(IDI_CHAT, 16));
				table->setGroupImageList(images);
			}

			auto row = cur->addChild(Grid::Seed(1, 2));
			row->column(0).mode = GridInfo::FILL;
			row->row(0).mode = GridInfo::FILL;
			row->row(0).align = GridInfo::STRETCH;
			row->column(1).mode = GridInfo::FILL;

			{
				//Table::Seed seed = WinUtil::Seeds::Dialog::combo;
				//seed.style |= LVS_SINGLESEL | LVS_NOCOLUMNHEADER;
				//installed = row->addChild(seed);
				cbCtrlTheme = row->addChild(WinUtil::Seeds::Dialog::comboBox);
				cbCtrlTheme->onSelectionChanged([this] { handleTheme(); refreshPreview(); /*table->Control::redraw(true);*/ });
			}

			{
				auto group = row->addChild(GroupBox::Seed(T_("Theme options"))); // Modify / Export
				themeInfo = group->addChild(Grid::Seed(2, 1));
				themeInfo->column(0).mode = GridInfo::FILL;

				expTheme = themeInfo->addChild(WinUtil::Seeds::Dialog::button);
				expTheme->setText(T_("E&xport theme"));
				expTheme->onClicked([] { /*exportTheme()*/ });

				modTheme = themeInfo->addChild(WinUtil::Seeds::Dialog::button);
				modTheme->setText(T_("M&odify theme"));
				modTheme->onClicked([] { /*modifyTheme()*/ });

			}
	}
	
	WinUtil::makeColumns(table, tcol, COLUMN_LAST);
	//WinUtil::makeColumns(installed, columns, COLUMN_COUNT);

	TStringList groups(GROUP_LAST);
	groups[GROUP_GENERAL] = T_("General");
	groups[GROUP_TRANSFERS] = T_("Transfers");
	groups[GROUP_CHAT] = T_("Chat");
	table->setGroups(groups);
	auto grouped = table->isGrouped();

	auto add = [this, grouped](tstring text, int group, int textColorSetting, int bgColorSetting) -> ThemeData* {
		auto data = new ThemeData(text, textColorSetting, bgColorSetting);
		this->table->insert(grouped ? group : -1, data);
		return data;
	};

	populateThemes();

	//Use default theme until I can get to this and fix 
	globalData = add(T_("Global application style"), GROUP_GENERAL, static_cast<int>(curTheme.textColor), static_cast<int>(curTheme.background));
	ulData = add(T_("Uploads"), GROUP_TRANSFERS, static_cast<int>(curTheme.uploadText), static_cast<int>(curTheme.uploadBg));
	dlData = add(T_("Downloads"), GROUP_TRANSFERS, static_cast<int>(curTheme.downloadText), static_cast<int>(curTheme.downloadBg));
	linkData = add(T_("Links"), GROUP_CHAT, static_cast<int>(curTheme.linkColor), -1);
	logData = add(T_("Logs"), GROUP_CHAT, static_cast<int>(curTheme.logColor), -1);

	refreshPreview();

	layout();
	centerWindow();
}

ThemePage::~ThemePage() {
}

void ThemePage::layout() {
	PropPage::layout();

	table->setColumnWidth(COLUMN_TEXT, table->getWindowSize().x - 30);
}

const tstring& ThemePage::ThemeData::getText(int) const {
	return text;
}

int ThemePage::ThemeData::getStyle(HFONT& font, COLORREF& textColor, COLORREF& bgColor, int) const {

	LOGFONT lf;
	WinUtil::decodeFont(Text::toT(SETTING(MAIN_FONT)), lf);
	auto f = new dwt::Font(lf);
	font = f->handle();

	auto color = getTextColor();
	if(color >= 0) {
		textColor = color;
	}

	color = getBgColor();
	if(color >= 0) {
		bgColor = color;
	}

	return CDRF_NEWFONT;
}

void ThemePage::update(ThemeData* const data) {
	if(data == globalData) {
		table->setColor(globalData->getTextColor(), globalData->getBgColor());
		table->Control::redraw(true);
	} else {
		table->update(data);
	}

	table->getParent()->layout();
}

void ThemePage::refreshPreview() {
	currentTheme();

	globalData->txtColor = curTheme.textColor;
	globalData->bgColor = curTheme.background;
	update(globalData);

	ulData->txtColor = curTheme.uploadText;
	ulData->bgColor = curTheme.uploadBg;
	update(ulData);

	dlData->txtColor = curTheme.downloadText;
	dlData->bgColor = curTheme.downloadBg;
	update(dlData);

	linkData->txtColor = curTheme.linkColor;
	update(linkData);
	logData->txtColor = curTheme.logColor;
	update(logData);

	/*currentTheme();*/
}

void ThemePage::currentTheme() {
	auto get = [&](SettingsManager::IntSetting setting) -> COLORREF {
		auto sm = SettingsManager::getInstance();
		return sm->get(setting);
	};

	curTheme.name = SETTING(SettingsManager::LOADED_THEME);
	curTheme.textColor = get(SettingsManager::TEXT_COLOR);
	curTheme.background = get(SettingsManager::BACKGROUND_COLOR);
	curTheme.background = get(SettingsManager::BACKGROUND_COLOR);
	curTheme.uploadText = get(SettingsManager::UPLOAD_TEXT_COLOR);
	curTheme.uploadBg = get(SettingsManager::UPLOAD_BG_COLOR);
	curTheme.downloadText = get(SettingsManager::DOWNLOAD_TEXT_COLOR);
	curTheme.downloadBg = get(SettingsManager::DOWNLOAD_BG_COLOR);
	curTheme.linkColor = get(SettingsManager::LINK_COLOR);
	curTheme.logColor = get(SettingsManager::LOG_COLOR);
}

void ThemePage::defaultTheme() {
	auto getDefault = [&](SettingsManager::IntSetting setting) -> int {
		auto sm = SettingsManager::getInstance();
		return sm->getDefault(setting);
	};

	defTheme.name = "Default";
	//defTheme.uuid = "{00000000-0000-0000-0000-000000000000}";
	//defTheme.description = "Default theme";
	//defTheme.author = APPNAME;
	//defTheme.website = "https://dcplusplus.sourceforge.io/";
	defTheme.textColor = getDefault(SettingsManager::TEXT_COLOR);
	defTheme.background = getDefault(SettingsManager::BACKGROUND_COLOR);
	defTheme.uploadText = getDefault(SettingsManager::UPLOAD_TEXT_COLOR);
	defTheme.uploadBg = getDefault(SettingsManager::UPLOAD_BG_COLOR);
	defTheme.downloadText = getDefault(SettingsManager::DOWNLOAD_TEXT_COLOR);
	defTheme.downloadBg = getDefault(SettingsManager::DOWNLOAD_BG_COLOR);
	defTheme.linkColor = getDefault(SettingsManager::LINK_COLOR);
	defTheme.logColor = getDefault(SettingsManager::LOG_COLOR);
}

void ThemePage::handleTheme() {
	auto sel = Text::fromT(cbCtrlTheme->getText());
	themeMap::iterator i = themes.find(sel);
	if(i != themes.end()) {
		loadTheme(i->second);
	}
}

void ThemePage::populateThemes() {
	cbCtrlTheme->clear();
	themes.clear();

	if(themes.empty()) {
		string path = Util::getPath(Util::PATH_RESOURCES) + "Themes" + PATH_SEPARATOR_STR;
		for(FileFindIter i(path + "*.dcpptheme"), end; i != end; ++i) {
			LogManager::getInstance()->message("Path = " + path + " File: " + i->getFileName());
			string filepath = path + i->getFileName();
			themes.emplace(i->getFileName(), filepath);
		}
	}

	auto def = cbCtrlTheme->insertValue(0, T_("Select a theme"));
	for(themeMap::const_iterator t = themes.begin(); t != themes.end(); ++t) {
		cbCtrlTheme->addValue(Text::toT(t->first));
	}
	cbCtrlTheme->setSelected(def);
}

void ThemePage::loadTheme(const string& path) {
	auto sm = SettingsManager::getInstance();

	auto log = [](const string& message) {
		LogManager::getInstance()->message(message);
	};

	log("curTheme.name = " + curTheme.name);
	log("LoadTheme(" + path + ")");

	SimpleXML xml;
	try {
		xml.fromXML(File(path, File::READ, File::OPEN).read());
		if(xml.findChild("dcpptheme")) {
			xml.stepIn();

			auto parse = [&xml, sm, &log](string tag, string& out) {
				xml.resetCurrentChild();
				if(xml.findChild(tag)) {
					out = xml.getChildData();
					log(tag + " xml data = "+ out);
					sm->set(SettingsManager::LOADED_THEME, xml.getChildData());
				}
			};

			auto parseColor = [&xml, sm, &log](string tag, COLORREF& out, SettingsManager::IntSetting setting) {
				xml.resetCurrentChild();
				if (xml.findChild(tag)) {
					string color = xml.getChildData();
					log(tag + " xml color data = " + Util::toString(out));
					out = Util::toInt(color);
					sm->set(setting, static_cast<COLORREF>(Util::toInt(xml.getChildData())));
				}
			};


			//		Theme theme;
			parse("Name", curTheme.name);
			parseColor("TextColor", curTheme.textColor, SettingsManager::TEXT_COLOR);
			parseColor("BgColor", curTheme.background, SettingsManager::BACKGROUND_COLOR);
			parseColor("ULBarTextColor", curTheme.uploadText, SettingsManager::UPLOAD_TEXT_COLOR);
			parseColor("ULBarBgColor", curTheme.uploadBg, SettingsManager::UPLOAD_BG_COLOR);
			parseColor("DLBarTextColor", curTheme.downloadText, SettingsManager::DOWNLOAD_TEXT_COLOR);
			parseColor("DLBarBgColor", curTheme.downloadBg, SettingsManager::DOWNLOAD_BG_COLOR);
			parseColor("LinkColor", curTheme.linkColor, SettingsManager::LINK_COLOR);
			parseColor("LogColor", curTheme.logColor, SettingsManager::LOG_COLOR);

			//auto sm = SettingsManager::getInstance();
			//sm->set(SettingsManager::TEXT_COLOR, curTheme.textColor);
			//sm->set(SettingsManager::BACKGROUND_COLOR, curTheme.background);
			//sm->set(SettingsManager::LINK_COLOR, curTheme.linkColor);
			//sm->set(SettingsManager::LOG_COLOR, curTheme.logColor);
			//sm->save();

			xml.stepOut();
		}

		refreshPreview();
	} catch(const Exception& e) {
		dcdebug("ThemePage::loadTheme: %s\n", e.getError().c_str());
	}
	//sm->save();
}

void ThemePage::saveTheme(const string& path) {
	SimpleXML xml;
	xml.addTag("dcpptheme");
	xml.stepIn();

	xml.addChildAttrib("Name", curTheme.name);
	//xml.addChildAttrib("UUID", curTheme.uuid);
	//xml.addChildAttrib("Description", curTheme.description);
	//xml.addChildAttrib("Author", curTheme.author);
	//xml.addChildAttrib("Website", curTheme.website);
	//xml.addChildAttrib("Version", curTheme.version);
	xml.addChildAttrib("TextColor", curTheme.textColor);
	xml.addChildAttrib("BgColor", curTheme.background);
	xml.addChildAttrib("ULBarTextColor", curTheme.uploadText);
	xml.addChildAttrib("ULBarBgColor", curTheme.uploadBg);
	xml.addChildAttrib("DLBarTextColor", curTheme.downloadText);
	xml.addChildAttrib("DLBarBgColor", curTheme.downloadBg);
	xml.addChildAttrib("LinkColor", curTheme.linkColor);
	xml.addChildAttrib("LogColor", curTheme.logColor);

	xml.stepOut();

	try {
		File ff(path, File::WRITE, File::CREATE | File::TRUNCATE);
		BufferedOutputStream<false> f(&ff);
		f.write(SimpleXML::utf8Header);
		xml.toXML(&f);
	} catch(const FileException&) {
		// ...
	}
}
