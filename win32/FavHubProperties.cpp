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
#include "FavHubProperties.h"

#include <dcpp/FavoriteManager.h>
#include <dcpp/HubEntry.h>
#include <dcpp/version.h>

#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/MessageBox.h>

#include "resource.h"
#include "FavHubGroupsDlg.h"
#include "HoldRedraw.h"
#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;
using dwt::Label;

std::map<UINT, std::wstring> FavHubProperties::encodings;

FavHubProperties::FavHubProperties(dwt::Widget* parent, FavoriteHubEntry *_entry) :
GridDialog(parent, 400, DS_CONTEXTHELP),
name(0),
address(0),
hubDescription(0),
nick(0),
password(0),
description(0),
email(0),
userIp(0),
userIp6(0),
encoding(0),
showJoins(0),
favShowJoins(0),
logMainChat(0),
groups(0),
entry(_entry)
{
	onInitDialog([this] { return handleInitDialog(); });
}

FavHubProperties::~FavHubProperties() {
}

bool FavHubProperties::handleInitDialog() {
	grid = addChild(Grid::Seed(5, 2));
	grid->column(0).mode = GridInfo::FILL;
	grid->column(1).mode = GridInfo::FILL;

	{
		auto group = grid->addChild(GroupBox::Seed(T_("Hub")));
		grid->setWidget(group, 0, 0, 1, 2);

		bool isAdcHub = Util::isAdcUrl(entry->getServer()) || Util::isAdcsUrl(entry->getServer());

		int rows = isAdcHub ? 3 : 4;

		auto cur = group->addChild(Grid::Seed(rows, 2));
		cur->column(0).align = GridInfo::BOTTOM_RIGHT;
		cur->column(1).mode = GridInfo::FILL;

		cur->addChild(Label::Seed(T_("Name")));
		name = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		name->setText(Text::toT(entry->getName()));

		cur->addChild(Label::Seed(T_("Address")));
		address = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		address->setText(Text::toT(entry->getServer()));
		WinUtil::preventSpaces(address);

		cur->addChild(Label::Seed(T_("Description")));
		hubDescription = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		hubDescription->setText(Text::toT(entry->getHubDescription()));

		if(!isAdcHub)
		{
			cur->addChild(Label::Seed(T_("Encoding")));
			encoding = cur->addChild(WinUtil::Seeds::Dialog::comboBox);

			fillEncodings();
		}
	}

	{
		auto group = grid->addChild(GroupBox::Seed(T_("Identification (leave blank for defaults)")));
		grid->setWidget(group, 1, 0, 1, 2);

		auto cur = group->addChild(Grid::Seed(6, 2));
		cur->column(0).align = GridInfo::BOTTOM_RIGHT;
		cur->column(1).mode = GridInfo::FILL;

		cur->addChild(Label::Seed(T_("Nick")));
		nick = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		nick->setText(Text::toT(entry->get(HubSettings::Nick)));
		WinUtil::preventSpaces(nick);

		cur->addChild(Label::Seed(T_("Password")));
		password = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		password->setPassword();
		password->setText(Text::toT(entry->getPassword()));
		WinUtil::preventSpaces(password);

		cur->addChild(Label::Seed(T_("Description")));
		description = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		description->setText(Text::toT(entry->get(HubSettings::Description)));

		cur->addChild(Label::Seed(T_("Email")));
		email = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		email->setText(Text::toT(entry->get(HubSettings::Email)));

		cur->addChild(Label::Seed(T_("IPv4")));
		userIp = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		userIp->setText(Text::toT(entry->get(HubSettings::UserIp)));
		WinUtil::preventSpaces(userIp);

		cur->addChild(Label::Seed(T_("IPv6")));
		userIp6 = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		userIp6->setText(Text::toT(entry->get(HubSettings::UserIp6)));
		WinUtil::preventSpaces(userIp6);
	}

	{
		auto cur = grid->addChild(Grid::Seed(3, 2));
		grid->setWidget(cur, 2, 0, 1, 2);
		cur->column(0).mode = GridInfo::FILL;
		cur->column(0).align = GridInfo::BOTTOM_RIGHT;

		cur->addChild(Label::Seed(T_("Show joins / parts in chat by default")));
		showJoins = cur->addChild(WinUtil::Seeds::Dialog::comboBox);
		WinUtil::fillTriboolCombo(showJoins);
		showJoins->setSelected(toInt(entry->get(HubSettings::ShowJoins)));

		cur->addChild(Label::Seed(T_("Only show joins / parts for favorite users")));
		favShowJoins = cur->addChild(WinUtil::Seeds::Dialog::comboBox);
		WinUtil::fillTriboolCombo(favShowJoins);
		favShowJoins->setSelected(toInt(entry->get(HubSettings::FavShowJoins)));

		cur->addChild(Label::Seed(T_("Log main chat")));
		logMainChat = cur->addChild(WinUtil::Seeds::Dialog::comboBox);
		WinUtil::fillTriboolCombo(logMainChat);
		logMainChat->setSelected(toInt(entry->get(HubSettings::LogMainChat)));
	}

	{
		auto group = grid->addChild(GroupBox::Seed(T_("Group")));
		grid->setWidget(group, 3, 0, 1, 2);

		auto cur = group->addChild(Grid::Seed(1, 2));
		cur->column(0).mode = GridInfo::FILL;

		auto seed = WinUtil::Seeds::Dialog::comboBox;
		seed.style |= CBS_SORT;
		groups = cur->addChild(seed);

		auto manage = cur->addChild(Button::Seed(T_("Manage &groups")));
		manage->onClicked([this] { handleGroups(); });
	}

	WinUtil::addDlgButtons(grid,
		[this] { handleOKClicked(); },
		[this] { endDialog(IDCANCEL); });

	fillGroups();

	setText(T_("Favorite Hub Properties"));

	layout();
	centerWindow();

	return false;
}

void FavHubProperties::handleOKClicked() {
	tstring addressText = address->getText();
	if(addressText.empty()) {
		dwt::MessageBox(this).show(T_("Hub address cannot be empty"), _T(APPNAME) _T(" ") _T(VERSIONSTRING), dwt::MessageBox::BOX_OK, dwt::MessageBox::BOX_ICONEXCLAMATION);
		return;
	}
	entry->setServer(Text::fromT(addressText));
	entry->setName(Text::fromT(name->getText()));
	entry->setHubDescription(Text::fromT(hubDescription->getText()));
	if(encoding) { // the box is optional.
		entry->setEncoding(
			encoding->getSelected() == 0 ? string() : // "Default" item.
			Text::fromT(encoding->getText()));
	}
	entry->get(HubSettings::Nick) = Text::fromT(nick->getText());
	entry->setPassword(Text::fromT(password->getText()));
	entry->get(HubSettings::Description) = Text::fromT(description->getText());
	entry->get(HubSettings::Email) = Text::fromT(email->getText());
	entry->get(HubSettings::UserIp) = Text::fromT(userIp->getText());
	entry->get(HubSettings::UserIp6) = Text::fromT(userIp6->getText());
	entry->get(HubSettings::ShowJoins) = to3bool(showJoins->getSelected());
	entry->get(HubSettings::FavShowJoins) = to3bool(favShowJoins->getSelected());
	entry->get(HubSettings::LogMainChat) = to3bool(logMainChat->getSelected());
	entry->setGroup(Text::fromT(groups->getText()));
	FavoriteManager::getInstance()->save();
	endDialog(IDOK);
}

void FavHubProperties::handleGroups() {
	FavHubGroupsDlg(this, entry).run();

	HoldRedraw hold { groups };
	groups->clear();
	fillGroups();
}

void FavHubProperties::fillGroups() {
	const string& entryGroup = entry->getGroup();
	bool needSel = true;

	groups->addValue(_T(""));

	for(auto& i: FavoriteManager::getInstance()->getFavHubGroups()) {
		const string& name = i.first;
		auto pos = groups->addValue(Text::toT(name));
		if(needSel && name == entryGroup) {
			groups->setSelected(pos);
			needSel = false;
		}
	}

	if(needSel)
		groups->setSelected(0);
}

void FavHubProperties::fillEncodings()
{
	// Load all available code pages
	::EnumSystemCodePages(EnumCodePageProc, CP_INSTALLED);

	// Add a default code page
	encoding->addValue(T_("Default"));

	UINT currentCodePage = Text::getCodePage(entry->getEncoding());

	// Go through all code pages and add them to the view
	int selectedItem = 0, counter = 1;
	for(auto& e: encodings)
	{
		encoding->addValue(e.second);

		// This is so we keep track of which code page should be the one to be selected
		if(currentCodePage == e.first)
		{
			selectedItem = counter;
		}
		counter++;
	}

	encoding->setSelected(selectedItem);
}

BOOL CALLBACK FavHubProperties::EnumCodePageProc(LPTSTR lpCodePageString)
{
	if(wcslen(lpCodePageString) != 0)
	{
		UINT pageId = _ttoi(lpCodePageString);

		CPINFOEX cpInfoEx = { 0 };
		GetCPInfoEx(pageId, 0, &cpInfoEx);

		if(wcslen(cpInfoEx.CodePageName) != 0)
		{
			encodings[pageId] = cpInfoEx.CodePageName;
		}
	}

	return TRUE;
}
