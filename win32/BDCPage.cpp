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

#include "BDCUtil.h"
#include "WinUtil.h"

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
		grid->column(0).mode = GridInfo::STATIC;
		grid->column(0).align = GridInfo::TOP_LEFT;
		testSUDP = grid->addChild(WinUtil::Seeds::Dialog::button);
		testSUDP->setText(T_("Test SUDP"));
		testSUDP->onClicked([] { SearchManager::getInstance()->testSUDP(); });

	}

	{
		auto grid = cur->addChild(GroupBox::Seed(T_("Action for double-clicking users in the userlist")))->addChild(Grid::Seed(1, 1));
		grid->column(0).mode = GridInfo::FILL;
		grid->row(0).align = GridInfo::STRETCH;
		actionDBLClickUserCb = grid->addChild(WinUtil::Seeds::Dialog::comboBox);
		for(const auto& ai: BDCUtil::actions) {
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

