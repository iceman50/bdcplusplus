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

#include <dcpp/format.h>
#include <dcpp/LogManager.h>
#include <dcpp/ScopedFunctor.h>
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

enum { COLUMN_THEME, COLUMN_UUID, COLUMN_COUNT }; // This is for installed(TablePtr)
static const ColumnInfo columns[COLUMN_COUNT] = {
	{ "", 100, false },
	{ "", 0, false } // hidden column to store the UUID
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
installed(0),
themeInfo(0)
{
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
				Table::Seed seed = WinUtil::Seeds::Dialog::table;
				seed.style |= LVS_SINGLESEL | LVS_NOCOLUMNHEADER;
				installed = row->addChild(seed);
			}

			{
				auto group = row->addChild(GroupBox::Seed(T_("Theme information")));
				themeInfo = group->addChild(Grid::Seed(1, 1));
				themeInfo->column(0).mode = GridInfo::FILL;
			}
	}
	
	WinUtil::makeColumns(table, tcol, COLUMN_LAST);
	WinUtil::makeColumns(installed, columns, COLUMN_COUNT);

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

	theme = WinUtil::loadedTheme;
	if (theme.uuid.empty()) { // Empty theme let's just load defaults
		defaultTheme(theme);
	}

	LogManager::getInstance()->message(theme.name + " " + theme.uuid + "WinUtil::loadedTheme \r\n");
	globalData = add(T_("Global application style"), GROUP_GENERAL, static_cast<int>(theme.textColor), static_cast<int>(theme.background));
	ulData = add(T_("Uploads"), GROUP_TRANSFERS, static_cast<int>(theme.uploadText), static_cast<int>(theme.uploadBg));
	dlData = add(T_("Downloads"), GROUP_TRANSFERS, static_cast<int>(theme.downloadText), static_cast<int>(theme.downloadBg));
	linkData = add(T_("Links"), GROUP_CHAT, static_cast<int>(theme.linkColor), -1);
	logData = add(T_("Logs"), GROUP_CHAT, static_cast<int>(theme.logColor), -1);

	update(globalData);
	update(ulData);
	update(dlData);
	update(linkData);
	update(logData);

	layout();
	centerWindow();

	refreshList();
	handleSelectionChange();

	installed->onSelectionChanged([this] { handleSelectionChange(); });
}

ThemePage::~ThemePage() {
}

void ThemePage::layout() {
	PropPage::layout();

	table->setColumnWidth(COLUMN_TEXT, table->getWindowSize().x - 30);
	installed->setColumnWidth(COLUMN_THEME, installed->getWindowSize().x - 30);
}

void ThemePage::write() {
	table->forEach(&ThemePage::ThemeData::write);
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
}

void ThemePage::handleSelectionChange() {
	auto selected = installed->hasSelected();
	ScopedFunctor([&] { themeInfo->layout(); themeInfo->redraw(); });

	HoldRedraw hold { themeInfo };
	HoldRedraw hold2 { table };

	themeInfo->clearRows();
	auto children = themeInfo->getChildren<Control>();
	boost::for_each(std::vector<Control*>(children.first, children.second), [](Control* w) { w->close(); });

	themeInfo->addRow(GridInfo(0, GridInfo::FILL, GridInfo::STRETCH));

	if(installed->countSelected() != 1) {
		themeInfo->addChild(Label::Seed(T_("No theme has been selected")));
		return;
	}

	auto infoGrid = themeInfo->addChild(Grid::Seed(0, 2));
	infoGrid->column(1).mode = GridInfo::FILL;
	infoGrid->setSpacing(themeInfo->getSpacing());

	enum Type { Name, Version, Description, Author, Website };

	auto addInfo = [this, infoGrid](tstring name, const string& value, Type type) {
		if(type == Description) {
			infoGrid->addRow(GridInfo(0, GridInfo::FILL, GridInfo::STRETCH));
		} else {
			infoGrid->addRow();
		}
		infoGrid->addChild(Label::Seed(name));
		if(type == Website && !value.empty()) {
			infoGrid->addChild(Link::Seed(Text::toT(value), true, WinUtil::parseLink));
		} else {
			infoGrid->addChild(Label::Seed(value.empty() ?
									   T_("<Information unavailable>") : Text::toT(value)));
		}
	};
	//Test this
	auto theme = WinUtil::findTheme(sel());
	if(theme == nullptr) { 
		WinUtil::Theme defTheme = {  };
		defaultTheme(defTheme);
		theme = &defTheme;
	}

	addInfo(T_("Name: "), theme->name, Name);
	addInfo(T_("Version: "), Util::toString(theme->version), Version);
	addInfo(T_("Description: "), theme->description, Description);
	addInfo(T_("Author: "), theme->author, Author);
	addInfo(T_("Website: "), theme->website, Website);

//	auto t = table->getData(0);/*0..4*/
	{// Rethink this, it's broken currently
		globalData->txtColor = theme->textColor;
		globalData->bgColor = theme->background;
		ulData->txtColor = theme->uploadText;
		ulData->bgColor = theme->uploadBg;
		dlData->txtColor = theme->downloadText;
		dlData->bgColor = theme->downloadBg;
		linkData->txtColor = theme->linkColor;
		logData->txtColor = theme->logColor;
		update(globalData);
		update(ulData);
		update(dlData);
		update(linkData);
		update(logData);

		table->Control::redraw(true);
		table->setColor(theme->textColor, theme->background);
	}

}

void ThemePage::refreshList() {
	installed->clear();
	if(WinUtil::themeList.empty()) {
		TStringList row(COLUMN_COUNT);
		row[COLUMN_THEME] = Text::toT("No theme installed");
		row[COLUMN_UUID] = Text::toT("{00000000-0000-0000-0000-000000000000}");
		installed->insert(row, 0, 0);
		installed->setEnabled(false);
		table->setEnabled(false);
		return;
	}
	for(auto& themes : WinUtil::themeList) {
		addEntry(installed->size(), themes.uuid);
	}
	installed->setEnabled(true);
	table->setEnabled(true);
}

void ThemePage::addEntry(size_t idx, const string& uuid) {
	TStringList row(COLUMN_COUNT);
	auto theme = WinUtil::findTheme(uuid);
	row[COLUMN_THEME] = Text::toT(theme->name);
	row[COLUMN_UUID] = Text::toT(uuid);
	installed->insert(row, 0, idx);
}

string ThemePage::sel() const {
	return Text::fromT(installed->getText(installed->getSelected(), COLUMN_UUID));
}

void ThemePage::defaultTheme(WinUtil::Theme& theme) {
	auto getDefault = [&](SettingsManager::IntSetting setting) -> int {
		auto sm = SettingsManager::getInstance();
		return sm->getDefault(setting);
	};

	theme.name = "Default";
	theme.uuid = "{00000000-0000-0000-0000-000000000000}";
	theme.description = "Default theme";
	theme.author = "DC++";
	theme.website = "https://dcplusplus.sourceforge.io/";
	theme.version = VERSIONFLOAT;
	theme.textColor = getDefault(SettingsManager::TEXT_COLOR);
	theme.background = getDefault(SettingsManager::BACKGROUND_COLOR);
	theme.uploadText = getDefault(SettingsManager::UPLOAD_TEXT_COLOR);
	theme.uploadBg = getDefault(SettingsManager::UPLOAD_BG_COLOR);
	theme.downloadText = getDefault(SettingsManager::DOWNLOAD_TEXT_COLOR);	
	theme.downloadBg = getDefault(SettingsManager::DOWNLOAD_BG_COLOR);
	theme.linkColor = getDefault(SettingsManager::LINK_COLOR);
	theme.logColor = getDefault(SettingsManager::LOG_COLOR);
}
