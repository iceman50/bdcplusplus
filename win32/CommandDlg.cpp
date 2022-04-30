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
#include "CommandDlg.h"

#include <dcpp/UserCommand.h>
#include <dcpp/AdcCommand.h>
#include <dcpp/NmdcHub.h>
#include <dcpp/version.h>

#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/MessageBox.h>
#include <dwt/widgets/RadioButton.h>

#include "resource.h"
#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;
using dwt::Label;
using dwt::RadioButton;

CommandDlg::CommandDlg(dwt::Widget* parent, int type_, int ctx_, const tstring& name_, const tstring& command_,
					   const tstring& to_, const tstring& hub_) :
GridDialog(parent, 298, DS_CENTERMOUSE),
separator(0),
raw(0),
chat(0),
PM(0),
hubMenu(0),
userMenu(0),
searchMenu(0),
fileListMenu(0),
nameBox(0),
commandBox(0),
hubBox(0),
nick(0),
once(0),
type(type_),
ctx(ctx_),
name(name_),
command(command_),
to(to_),
hub(hub_)
{
	onInitDialog([this] { return handleInitDialog(); });
}

CommandDlg::~CommandDlg() {
}

bool CommandDlg::handleInitDialog() {
	grid = addChild(Grid::Seed(5, 3));
	grid->column(0).mode = GridInfo::FILL;
	grid->column(1).mode = GridInfo::FILL;
	grid->column(2).mode = GridInfo::FILL;

	{
		GroupBoxPtr group = grid->addChild(GroupBox::Seed(T_("Command Type")));
		grid->setWidget(group, 0, 0, 1, 3);

		GridPtr cur = group->addChild(Grid::Seed(2, 2));
		cur->column(1).mode = GridInfo::FILL;
		cur->column(1).align = GridInfo::BOTTOM_RIGHT;

		separator = cur->addChild(RadioButton::Seed(T_("Separator")));
		separator->onClicked([this] { handleTypeChanged(); });

		chat = cur->addChild(RadioButton::Seed(T_("Chat")));
		chat->onClicked([this] { handleTypeChanged(); });

		raw = cur->addChild(RadioButton::Seed(T_("Raw")));
		raw->onClicked([this] { handleTypeChanged(); });

		PM = cur->addChild(RadioButton::Seed(T_("PM")));
		PM->onClicked([this] { handleTypeChanged(); });
	}

	{
		GroupBoxPtr group = grid->addChild(GroupBox::Seed(T_("Context")));
		grid->setWidget(group, 1, 0, 1, 3);

		GridPtr cur = group->addChild(Grid::Seed(2, 2));
		cur->column(1).mode = GridInfo::FILL;
		cur->column(1).align = GridInfo::BOTTOM_RIGHT;

		hubMenu = cur->addChild(CheckBox::Seed(T_("Hub Menu")));
		searchMenu = cur->addChild(CheckBox::Seed(T_("Search Menu")));
		userMenu = cur->addChild(CheckBox::Seed(T_("User Menu")));
		fileListMenu = cur->addChild(CheckBox::Seed(T_("Filelist Menu")));
	}

	{
		GroupBoxPtr group = grid->addChild(GroupBox::Seed(T_("Parameters")));
		grid->setWidget(group, 2, 0, 1, 3);

		GridPtr cur = group->addChild(Grid::Seed(9, 1));
		cur->column(0).mode = GridInfo::FILL;

		cur->addChild(Label::Seed(T_("Name")));
		nameBox = cur->addChild(WinUtil::Seeds::Dialog::textBox);

		cur->addChild(Label::Seed(T_("Command")));
		TextBox::Seed seed = WinUtil::Seeds::Dialog::textBox;
		seed.style |= ES_MULTILINE | WS_VSCROLL | ES_WANTRETURN;
		commandBox = cur->addChild(seed);
		commandBox->onUpdated([this] { updateCommand(); });

		cur->addChild(Label::Seed(T_("Hub address (see help for usage)")));
		hubBox = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		hubBox->onUpdated([this] { updateHub(); });

		cur->addChild(Label::Seed(T_("To")));
		nick = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		nick->onUpdated([this] { updateCommand(); });

		once = cur->addChild(CheckBox::Seed(T_("Send once per nick")));
	}

	WinUtil::addDlgButtons(grid,
		[this] { handleOKClicked(); },
		[this] { endDialog(IDCANCEL); });

	int newType = -1;
	if(type == UserCommand::TYPE_SEPARATOR) {
		separator->setChecked(true);
		newType = 0;
	} else {
		commandBox->setText(Text::toDOS(command));
		if(type == UserCommand::TYPE_RAW || type == UserCommand::TYPE_RAW_ONCE) {
			raw->setChecked(true);
			newType = 1;
		} else if(type == UserCommand::TYPE_CHAT || type == UserCommand::TYPE_CHAT_ONCE) {
			if(to.empty()) {
				chat->setChecked(true);
				newType = 2;
			} else {
				PM->setChecked(true);
				nick->setText(to);
				newType = 3;
			}
		}
		if(type == UserCommand::TYPE_RAW_ONCE || type == UserCommand::TYPE_CHAT_ONCE) {
			once->setChecked(true);
		}
	}
	type = newType;

	hubBox->setText(hub);
	nameBox->setText(name);

	if(ctx & UserCommand::CONTEXT_HUB)
		hubMenu->setChecked(true);
	if(ctx & UserCommand::CONTEXT_USER)
		userMenu->setChecked(true);
	if(ctx & UserCommand::CONTEXT_SEARCH)
		searchMenu->setChecked(true);
	if(ctx & UserCommand::CONTEXT_FILELIST)
		fileListMenu->setChecked(true);

	updateControls();
	updateCommand();

	setText(T_("Create / Modify User Command"));

	layout();
	centerWindow();

	return false;
}

void CommandDlg::handleTypeChanged() {
	updateType();
	updateCommand();
	updateControls();
}

void CommandDlg::handleOKClicked() {
	name = nameBox->getText();
	if((type != 0) && (name.empty() || commandBox->getText().empty())) {
		dwt::MessageBox(this).show(T_("Name and command must not be empty"), _T(APPNAME) _T(" ") _T(VERSIONSTRING), dwt::MessageBox::BOX_OK, dwt::MessageBox::BOX_ICONEXCLAMATION);
		return;
	}

	ctx = 0;
	if(hubMenu->getChecked())
		ctx |= UserCommand::CONTEXT_HUB;
	if(userMenu->getChecked())
		ctx |= UserCommand::CONTEXT_USER;
	if(searchMenu->getChecked())
		ctx |= UserCommand::CONTEXT_SEARCH;
	if(fileListMenu->getChecked())
		ctx |= UserCommand::CONTEXT_FILELIST;

	switch(type) {
	case 0:
		type = UserCommand::TYPE_SEPARATOR;
		break;
	case 1:
		type = once->getChecked() ? UserCommand::TYPE_RAW_ONCE : UserCommand::TYPE_RAW;
		break;
	case 2:
		type = UserCommand::TYPE_CHAT;
		break;
	case 3:
		type = once->getChecked() ? UserCommand::TYPE_CHAT_ONCE : UserCommand::TYPE_CHAT;
		to = nick->getText();
		break;
	}

	endDialog(IDOK);
}

void CommandDlg::updateType() {
	if(separator->getChecked()) {
		type = 0;
	} else if(raw->getChecked()) {
		type = 1;
	} else if(chat->getChecked()) {
		type = 2;
	} else if(PM->getChecked()) {
		type = 3;
	}
}

void CommandDlg::updateCommand() {
	if(type == 0) {
		command.clear();
	} else {
		command = commandBox->getText();
	}
	if(type == 1 && UserCommand::adc(Text::fromT(hub)) && !command.empty() && *(command.end() - 1) != '\n')
		command += '\n';
}

void CommandDlg::updateHub() {
	hub = hubBox->getText();
	updateCommand();
}

void CommandDlg::updateControls() {
	switch(type) {
		case 0:
			nameBox->setEnabled(false);
			commandBox->setEnabled(false);
			nick->setEnabled(false);
			break;
		case 1:
		case 2:
			nameBox->setEnabled(true);
			commandBox->setEnabled(true);
			nick->setEnabled(false);
			break;
		case 3:
			nameBox->setEnabled(true);
			commandBox->setEnabled(true);
			nick->setEnabled(true);
			break;
	}
}
