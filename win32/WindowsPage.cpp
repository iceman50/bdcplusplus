/*
 * Copyright (C) 2001-2023 Jacek Sieka, arnetheduck on gmail point com
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
#include "WindowsPage.h"

#include <dcpp/SettingsManager.h>

#include <dwt/widgets/Grid.h>

#include "resource.h"
#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;

WindowsPage::ListItem WindowsPage::optionItems[] = {
	{ SettingsManager::POPUP_PMS, N_("Open private messages in their own window") },
	{ SettingsManager::POPUP_HUB_PMS, N_("Open private messages from bots in their own window") },
	{ SettingsManager::POPUP_BOT_PMS, N_("Open private messages from the hub in their own window") },
	{ SettingsManager::POPUNDER_FILELIST, N_("Open new file list windows in the background") },
	{ SettingsManager::POPUNDER_PM, N_("Open new private message windows in the background") },
	{ SettingsManager::JOIN_OPEN_NEW_WINDOW, N_("Open new window when using /join") },
	{ SettingsManager::IGNORE_HUB_PMS, N_("Ignore private messages from the hub") },
	{ SettingsManager::IGNORE_BOT_PMS, N_("Ignore private messages from bots") },
	{ SettingsManager::TOGGLE_ACTIVE_WINDOW, N_("Toggle window when selecting an active tab") },
	{ SettingsManager::PROMPT_PASSWORD, N_("Popup box to input password for hubs") },
	{ 0, 0 }
};

WindowsPage::ListItem WindowsPage::confirmItems[] = {
	{ SettingsManager::CONFIRM_EXIT, N_("Confirm application exit") },
	{ SettingsManager::CONFIRM_HUB_CLOSING, N_("Confirm hub closing") },
	{ SettingsManager::CONFIRM_HUB_REMOVAL, N_("Confirm favorite hub removal") },
	{ SettingsManager::CONFIRM_USER_REMOVAL, N_("Confirm favorite user removal") },
	{ SettingsManager::CONFIRM_ITEM_REMOVAL, N_("Confirm item removal in download queue") },
	{ SettingsManager::CONFIRM_ADLS_REMOVAL, N_("Confirm ADL Search removal") },
	{ 0, 0 }
};

WindowsPage::WindowsPage(dwt::Widget* parent) :
PropPage(parent, 2, 1),
options(0),
confirm(0)
{
	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;
	grid->row(1).mode = GridInfo::FILL;
	grid->row(1).align = GridInfo::STRETCH;

	options = grid->addChild(GroupBox::Seed(T_("Window options")))->addChild(WinUtil::Seeds::Dialog::optionsTable);
	confirm = grid->addChild(GroupBox::Seed(T_("Confirm dialog options")))->addChild(WinUtil::Seeds::Dialog::optionsTable);

	PropPage::read(optionItems, options);
	PropPage::read(confirmItems, confirm);
}

WindowsPage::~WindowsPage() {
}

void WindowsPage::write() {
	PropPage::write(options);
	PropPage::write(confirm);
}
