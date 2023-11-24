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

#include <dcpp/LogManager.h>
#include <dcpp/SearchManager.h>
#include <dcpp/SettingsManager.h>
#include <dcpp/version.h>

#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>

using dwt::Grid;
using dwt::GridInfo;
using dwt::GroupBox;
using dwt::Label;

PropPage::ListItem BDCPage::listItems[] = {
	{ SettingsManager::USE_THEME, N_("Enable theming (restart required)") },
//	{ SettingsManager::THEME_REGISTER, N_("Register with Windows to handle .dcpptheme files") },
	{ SettingsManager::TABS_ON_BOTTOM, N_("Set tabs on bottom of chat window (restart required)") },
	{ SettingsManager::ENABLE_NMDC_TLS, N_("Enable NMDC TLS C-C Connections") },
	{ SettingsManager::ENABLE_SUDP, N_("Enable SUDP (ADCS Only)") },
	{ 0, 0 }
};

BDCPage::BDCPage(dwt::Widget* parent) :	PropPage(parent, 1, 1),
options(0),
testSUDP(0),
testLogger(0),
actionDBLClickUserCb(0)
{
	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;

	auto group = grid->addChild(GroupBox::Seed(T_(MODNAME " settings")));

	auto cur = group->addChild(Grid::Seed(3, 1));
	cur->column(0).mode = GridInfo::FILL;
	cur->row(0).mode = GridInfo::FILL;
	cur->row(0).align = GridInfo::STRETCH;

	options = cur->addChild(WinUtil::Seeds::Dialog::optionsTable);
	WinUtil::setColor(options);

	{
		auto grid = cur->addChild(GroupBox::Seed(T_("Experimental")))->addChild(Grid::Seed(1, 3));
		//grid->column(0).mode = GridInfo::STATIC;
		//grid->column(0).align = GridInfo::TOP_LEFT;
		//grid->column(1).mode = GridInfo::STATIC;
		//grid->column(1).align = GridInfo::TOP_LEFT;

		testSUDP = grid->addChild(WinUtil::Seeds::Dialog::button);
		testSUDP->setText(T_("Test SUDP"));
		testSUDP->onClicked([] { SearchManager::getInstance()->testSUDP(); });

		testLogger = grid->addChild(WinUtil::Seeds::Dialog::button);
		testLogger->setText(T_("Test BDCLogger"));
		testLogger->onClicked([=] { testBDCLogger(); });
	}

	{
		auto grid = cur->addChild(GroupBox::Seed(T_("Action for double-clicking users in the userlist")))->addChild(Grid::Seed(1, 1));
		grid->column(0).mode = GridInfo::FILL;
		grid->row(0).align = GridInfo::STRETCH;
		actionDBLClickUserCb = grid->addChild(WinUtil::Seeds::Dialog::comboBox);
		for(const auto& ai: BDCWinUtil::actions) {
			actionDBLClickUserCb->addValue(ai);
		}
	}

	PropPage::read(items);
	PropPage::read(listItems, options);

	actionDBLClickUserCb->setSelected(SETTING(ACTION_DOUBLECLICK_USER));
}

BDCPage::~BDCPage() {
}

void BDCPage::write()
{
	PropPage::write(items);
	PropPage::write(options);

	SettingsManager::getInstance()->set(SettingsManager::ACTION_DOUBLECLICK_USER, actionDBLClickUserCb->getSelected());
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

