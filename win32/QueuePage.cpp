/*
 * Copyright (C) 2001-2021 Jacek Sieka, arnetheduck on gmail point com
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
#include "QueuePage.h"

#include <dcpp/SettingsManager.h>

#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>

#include "resource.h"
#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;
using dwt::Label;

PropPage::ListItem QueuePage::optionItems[] = {
	{ SettingsManager::PRIO_LOWEST, N_("Set lowest prio for newly added files larger than Low prio size") },
	{ SettingsManager::AUTODROP_ALL, N_("Autodrop slow sources for all queue items (except filelists)") },
	{ SettingsManager::AUTODROP_FILELISTS, N_("Remove slow filelists") },
	{ SettingsManager::AUTODROP_DISCONNECT, N_("Don't remove the source when autodropping, only disconnect") },
	{ SettingsManager::AUTO_SEARCH, N_("Automatically search for alternative download locations") },
	{ SettingsManager::AUTO_SEARCH_AUTO_MATCH, N_("Automatically match queue for search hits") },
	{ SettingsManager::SKIP_ZERO_BYTE, N_("Skip zero-byte files") },
	{ SettingsManager::DONT_DL_ALREADY_SHARED, N_("Don't download files already in share") },
	{ SettingsManager::DONT_DL_ALREADY_QUEUED, N_("Don't download files already in the queue") },
	{ SettingsManager::KEEP_FINISHED_FILES, N_("Keep finished files in the queue") },
	{ 0, 0 }
};

QueuePage::QueuePage(dwt::Widget* parent) :
PropPage(parent, 3, 1),
otherOptions(0)
{
	grid->column(0).mode = GridInfo::FILL;
	grid->row(2).mode = GridInfo::FILL;
	grid->row(2).align = GridInfo::STRETCH;

	{
		auto cur = grid->addChild(GroupBox::Seed(T_("Auto priority settings")))->addChild(Grid::Seed(2, 6));
		cur->column(0).align = GridInfo::BOTTOM_RIGHT;
		cur->column(1).size = 40;
		cur->column(1).mode = GridInfo::STATIC;
		cur->column(2).mode = GridInfo::FILL;
		cur->column(3).align = GridInfo::BOTTOM_RIGHT;
		cur->column(4).size = 40;
		cur->column(4).mode = GridInfo::STATIC;

		cur->addChild(Label::Seed(T_("Highest prio max size")));
		auto box = cur->addChild(WinUtil::Seeds::Dialog::intTextBox);
		items.emplace_back(box, SettingsManager::PRIO_HIGHEST_SIZE, PropPage::T_INT);
		cur->addChild(Label::Seed(T_("KiB")));

		cur->addChild(Label::Seed(T_("High prio max size")));
		box = cur->addChild(WinUtil::Seeds::Dialog::intTextBox);
		items.emplace_back(box, SettingsManager::PRIO_HIGH_SIZE, PropPage::T_INT);
		cur->addChild(Label::Seed(T_("KiB")));

		cur->addChild(Label::Seed(T_("Normal prio max size")));
		box = cur->addChild(WinUtil::Seeds::Dialog::intTextBox);
		items.emplace_back(box, SettingsManager::PRIO_NORMAL_SIZE, PropPage::T_INT);
		cur->addChild(Label::Seed(T_("KiB")));

		cur->addChild(Label::Seed(T_("Low prio max size")));
		box = cur->addChild(WinUtil::Seeds::Dialog::intTextBox);
		items.emplace_back(box, SettingsManager::PRIO_LOW_SIZE, PropPage::T_INT);
		cur->addChild(Label::Seed(T_("KiB")));
	}

	{
		GridPtr cur = grid->addChild(GroupBox::Seed(T_("Autodrop settings")))->addChild(Grid::Seed(3, 6));
		cur->column(0).align = GridInfo::BOTTOM_RIGHT;
		cur->column(1).size = 40;
		cur->column(1).mode = GridInfo::STATIC;
		cur->column(2).mode = GridInfo::FILL;
		cur->column(3).align = GridInfo::BOTTOM_RIGHT;
		cur->column(4).size = 40;
		cur->column(4).mode = GridInfo::STATIC;

		cur->addChild(Label::Seed(T_("Drop sources below")));
		TextBoxPtr box = cur->addChild(WinUtil::Seeds::Dialog::intTextBox);
		items.emplace_back(box, SettingsManager::AUTODROP_SPEED, PropPage::T_INT);
		cur->addChild(Label::Seed(T_("B/s")));

		cur->addChild(Label::Seed(T_("Check every")));
		box = cur->addChild(WinUtil::Seeds::Dialog::intTextBox);
		items.emplace_back(box, SettingsManager::AUTODROP_INTERVAL, PropPage::T_INT);
		cur->addChild(Label::Seed(T_("s")));

		cur->addChild(Label::Seed(T_("Min elapsed")));
		box = cur->addChild(WinUtil::Seeds::Dialog::intTextBox);
		items.emplace_back(box, SettingsManager::AUTODROP_ELAPSED, PropPage::T_INT);
		cur->addChild(Label::Seed(T_("s")));

		cur->addChild(Label::Seed(T_("Max inactivity")));
		box = cur->addChild(WinUtil::Seeds::Dialog::intTextBox);
		items.emplace_back(box, SettingsManager::AUTODROP_INACTIVITY, PropPage::T_INT);
		cur->addChild(Label::Seed(T_("s")));

		cur->addChild(Label::Seed(T_("Min sources online")));
		box = cur->addChild(WinUtil::Seeds::Dialog::intTextBox);
		items.emplace_back(box, SettingsManager::AUTODROP_MINSOURCES, PropPage::T_INT);
		cur->addChild(Label::Seed(tstring()));

		cur->addChild(Label::Seed(T_("Min filesize")));
		box = cur->addChild(WinUtil::Seeds::Dialog::intTextBox);
		items.emplace_back(box, SettingsManager::AUTODROP_FILESIZE, PropPage::T_INT);
		cur->addChild(Label::Seed(T_("KiB")));
	}

	otherOptions = grid->addChild(GroupBox::Seed(T_("Other queue options")))->addChild(WinUtil::Seeds::Dialog::optionsTable);

	PropPage::read(items);
	PropPage::read(optionItems, otherOptions);
}

QueuePage::~QueuePage() {
}

void QueuePage::write() {
	PropPage::write(items);
	PropPage::write(otherOptions);

	SettingsManager* settings = SettingsManager::getInstance();
	if(SETTING(AUTODROP_INTERVAL) < 1)
		settings->set(SettingsManager::AUTODROP_INTERVAL, 1);
	if(SETTING(AUTODROP_ELAPSED) < 1)
		settings->set(SettingsManager::AUTODROP_ELAPSED, 1);
}
