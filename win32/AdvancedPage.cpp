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
#include "AdvancedPage.h"

#include <dwt/widgets/Grid.h>
#include <dcpp/SettingsManager.h>

#include "resource.h"
#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;

AdvancedPage::ListItem AdvancedPage::listItems[] = {
	{ SettingsManager::AUTO_FOLLOW, N_("Automatically follow redirects") },
	{ SettingsManager::CLEAR_SEARCH, N_("Clear search box after each search") },
	{ SettingsManager::LIST_DUPES, N_("Keep duplicate files in your file list") },
	{ SettingsManager::URL_HANDLER, N_("Register with Windows to handle dchub://, nmdcs:// adc:// and adcs:// URL links") },
	{ SettingsManager::MAGNET_REGISTER, N_("Register with Windows to handle magnet: URI links") },
	{ SettingsManager::DCEXT_REGISTER, N_("Register with Windows to handle .dcext files") },
	{ SettingsManager::KEEP_LISTS, N_("Don't delete file lists when exiting") },
	{ SettingsManager::AUTO_KICK, N_("Automatically disconnect users who leave the hub") },
	{ SettingsManager::SFV_CHECK, N_("Enable automatic SFV checking") },
	{ SettingsManager::NO_AWAYMSG_TO_BOTS, N_("Don't send the away message to bots") },
	{ SettingsManager::ADLS_BREAK_ON_FIRST, N_("Break on first ADLSearch match") },
	{ SettingsManager::COMPRESS_TRANSFERS, N_("Enable safe and compressed transfers") },
	{ SettingsManager::HUB_USER_COMMANDS, N_("Accept custom user commands from hub") },
	{ SettingsManager::SEND_UNKNOWN_COMMANDS, N_("Send unknown /commands to the hub") },
	{ SettingsManager::ADD_FINISHED_INSTANTLY, N_("Add finished files to share instantly (if shared)") },
	{ SettingsManager::USE_CTRL_FOR_LINE_HISTORY, N_("Use CTRL for line history") },
	{ SettingsManager::AUTO_KICK_NO_FAVS, N_("Don't automatically disconnect favorite users who leave the hub") },
	{ SettingsManager::OWNER_DRAWN_MENUS, N_("Use extended menus with icons and titles") },
	{ SettingsManager::USE_SYSTEM_ICONS, N_("Use system icons when browsing files (slows browsing down a bit)") },
	{ SettingsManager::CLICKABLE_CHAT_LINKS, N_("Clickable chat links (disable on Wine)") },
	{ SettingsManager::SEGMENTED_DL, N_("Enable segmented downloads") },
	{ SettingsManager::REGISTER_SYSTEM_STARTUP, N_("Start DC++ when Windows starts") },
	{ SettingsManager::ENABLE_NMDC_TLS, N_("Enable NMDC TLS C-C Connections") },
	{ SettingsManager::TESTING_STATUS, N_("Display testing nags"),
		[]() { return SETTING(TESTING_STATUS) != SettingsManager::TESTING_DISABLED; }, // custom read
		[](bool checked) { // custom write
			if(checked) {
				SettingsManager::getInstance()->unset(SettingsManager::TESTING_STATUS); // back to defaults
			} else {
				SettingsManager::getInstance()->set(SettingsManager::TESTING_STATUS, SettingsManager::TESTING_DISABLED);
			}
		}
	},
	{ 0, 0 }
};

AdvancedPage::AdvancedPage(dwt::Widget* parent) : PropPage(parent, 1, 1) {

	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;

	options = grid->addChild(WinUtil::Seeds::Dialog::optionsTable);
	PropPage::read(listItems, options);
}

AdvancedPage::~AdvancedPage() {
}

void AdvancedPage::write() {
	PropPage::write(options);
}
