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
#include "BDCPage.h"

#include "BDCWinUtil.h"
#include "WinUtil.h"

#include <dcpp/BDCManager.h>
#include <dcpp/ClientManager.h>
#include <dcpp/LogManager.h>
#include <dcpp/SearchManager.h>
#include <dcpp/SettingsManager.h>
#include <dcpp/version.h>

#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/FolderDialog.h>

#include "BdcppSdExDialog.h"

using dwt::Grid;
using dwt::GridInfo;
using dwt::GroupBox;
using dwt::Label;
using dwt::FolderDialog;

PropPage::ListItem BDCPage::listItems[] = {
	{ BDCManager::ENABLE_ICON_THEMING,		 N_("Enable icon theming") },
	{ BDCManager::TABS_ON_BOTTOM,			 N_("Set tabs on bottom of chat window (restart required)") },
	{ BDCManager::FLASH_TASKBAR_ON_PM,		 N_("Flash taskbar on PM") },
	{ BDCManager::SHOW_HUB_IN_PM_CHATSTATUS, N_("Show origin hub in PM as a status message") },
};

BDCPage::BDCPage(dwt::Widget* parent) :	PropPage(parent, 1, 1),
options(0),
iconPath(0),
browseIcon(0),
actionDBLClickUserCb(0),
testLogger(0),
tabText(0),
sdexVersion(0)

{
	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;

	auto group = grid->addChild(GroupBox::Seed(T_(APPNAME " settings")));

	auto cur = group->addChild(Grid::Seed(4, 1));
	cur->column(0).mode = GridInfo::FILL;
	cur->row(0).mode = GridInfo::FILL;
	cur->row(0).align = GridInfo::STRETCH;

	options = cur->addChild(WinUtil::Seeds::Dialog::optionsTable);
	options->addColumn();
	//WinUtil::setColor(options);

	{
		auto grid = cur->addChild(GroupBox::Seed(T_("Icon theme folder")))->addChild(Grid::Seed(1, 2));
		grid->column(0).align = GridInfo::STRETCH;
		grid->column(0).mode = GridInfo::FILL;

		iconPath = grid->addChild(WinUtil::Seeds::Dialog::textBox);
		browseIcon = grid->addChild(WinUtil::Seeds::Dialog::button);
		browseIcon->setText(T_("..."));
		browseIcon->onClicked([this] { handleBrowseIcon(); });
	}


	{
		auto grid = cur->addChild(GroupBox::Seed(T_("Action for double-clicking users in the userlist")))->addChild(Grid::Seed(1, 1));
		grid->column(0).mode = GridInfo::FILL;
		grid->row(0).align = GridInfo::STRETCH;
		actionDBLClickUserCb = grid->addChild(WinUtil::Seeds::Dialog::comboBox);
		for(const auto& ai: Bdcpp::actions) {
			actionDBLClickUserCb->addValue(Text::toT(ai));
		}
	}

	{
		auto grid = cur->addChild(GroupBox::Seed(T_("Misc.")))->addChild(Grid::Seed(1, 3));
		grid->column(0).mode = GridInfo::FILL;
		grid->column(1).mode = GridInfo::FILL;
		grid->column(2).mode = GridInfo::FILL;
		grid->setSpacing(cur->getSpacing());

		testLogger = grid->addChild(WinUtil::Seeds::Dialog::button);
		testLogger->setText(T_("Test BDCLogger"));
		testLogger->onClicked([=] { testBDCLogger(); });

		tabText = grid->addChild(WinUtil::Seeds::Dialog::button);
		tabText->setText(T_("Custom hub tab text"));
		tabText->onClicked([=] { handleTabText(); });

		sdexVersion = grid->addChild(WinUtil::Seeds::Dialog::button);
		sdexVersion->setText(T_("SdEx version: ") + Text::toT(Util::toString(double(SDEX_VERSION))));
		sdexVersion->onClicked([=] { handleSdexVersion(); });
	}
	
	for(const auto& si: listItems)
		options->setChecked(options->insert(TStringList(1, T_(si.desc)), si.setting), BDCManager::getInstance()->get(BDCManager::BoolSetting(si.setting)));
	options->setColumnWidth(0, LVSCW_AUTOSIZE);

	actionDBLClickUserCb->setSelected(BDSETTING(ACTION_DOUBLECLICK_USER));

	iconPath->setText(Text::toT(BDSETTING(ICON_PATH)));
}

BDCPage::~BDCPage() {
}

void BDCPage::write() {
	int i = -1;
	while((i = options->getNext(i, LVNI_ALL)) != -1)
		BDCManager::getInstance()->set(BDCManager::BoolSetting(options->getData(i)), options->isChecked(i));

	BDCManager::getInstance()->set(BDCManager::ACTION_DOUBLECLICK_USER, actionDBLClickUserCb->getSelected());
	BDCManager::getInstance()->set(BDCManager::ICON_PATH, Text::fromT(iconPath->getText()));
}

void BDCPage::testBDCLogger() {
	auto logTest = [](const string& msg, LogMessage::Type type, LogMessage::Level level) {
		auto lm = LogManager::getInstance();
		lm->message(msg, type, level);
	};

	logTest("TYPE_DEBUG / LOG_SYSTEM", LogMessage::TYPE_DEBUG, LogMessage::LOG_SYSTEM);
	logTest("TYPE_GENERAL / LOG_SYSTEM", LogMessage::TYPE_GENERAL, LogMessage::LOG_SYSTEM);
	logTest("TYPE_WARNING / LOG_SYSTEM", LogMessage::TYPE_WARNING, LogMessage::LOG_SYSTEM);
	logTest("TYPE_ERROR / LOG_SYSTEM", LogMessage::TYPE_ERROR, LogMessage::LOG_SYSTEM);

	logTest("TYPE_DEBUG / LOG_HASHER", LogMessage::TYPE_DEBUG, LogMessage::LOG_SHARE);
	logTest("TYPE_DEBUG / LOG_PRIVATE", LogMessage::TYPE_DEBUG, LogMessage::LOG_PRIVATE);
	logTest("TYPE_DEBUG / LOG_SPAM", LogMessage::TYPE_DEBUG, LogMessage::LOG_SPAM);
	logTest("TYPE_DEBUG / LOG_SERVER", LogMessage::TYPE_DEBUG, LogMessage::LOG_SERVER);
	logTest("TYPE_DEBUG / LOG_PLUGIN", LogMessage::TYPE_DEBUG, LogMessage::LOG_PLUGIN);
	logTest("TYPE_DEBUG / LOG_SHARE", LogMessage::TYPE_DEBUG, LogMessage::LOG_SHARE);
}

void BDCPage::handleSdexVersion() {
	StringList params;
	params.emplace_back("testparam");
	params.emplace_back("testNI");
	params.emplace_back("testDE");
	params.emplace_back("test");
	BdcppSdExDialog dlg(this, T_("SdEx version: ") + Text::toT(Util::toString(double(SDEX_VERSION))), Util::emptyString, params);
	dlg.run();
}

void BDCPage::handleTabText() {
	BdcppSdExDialog dlg(this, tabText->getText(), BDSETTING(CUSTOM_TABTEXT_HUB_FORMAT), SdEx::getParamsTabText());
	if(dlg.run() == IDOK) {
		BDCManager::getInstance()->set(BDCManager::CUSTOM_TABTEXT_HUB_FORMAT, dlg.getSdEx().getPattern());
		auto lock = ClientManager::getInstance()->lock();
		for(auto ci: ClientManager::getInstance()->getClients())
			ci->fire(ClientListener::HubUpdated(), ci);
	}
}

void BDCPage::handleBrowseIcon() {
	tstring dir = iconPath->getText();
	if(dir.empty())
		dir = Text::toT(BDCManager::getInstance()->get(BDCManager::ICON_PATH));
	if(FolderDialog(this).open(dir)) {
		iconPath->setText(dir);
	}
}
