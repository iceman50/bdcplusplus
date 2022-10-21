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
#include "ThemeInfoDlg.h"

#include <dcpp/LogManager.h>
#include <dcpp/PluginManager.h>

#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/Link.h>
#include <dwt/widgets/MessageBox.h>

using dwt::Grid;
using dwt::GridInfo;
using dwt::Label;
using dwt::Link;

ThemeInfoDlg::ThemeInfoDlg(dwt::Widget* parent, const string& path) :
	dwt::ModalDialog(parent),
	grid(0)
{
	onInitDialog([this, path] { return handleInitDialog(path); });
}

ThemeInfoDlg::~ThemeInfoDlg() {
}

int ThemeInfoDlg::run() {
	create(dwt::Point(400, 300));
	return show();
}


bool ThemeInfoDlg::handleInitDialog(const string& path) {
	grid = addChild(Grid::Seed(0, 2));
	grid->column(1).mode = GridInfo::FILL;
	grid->setSpacing(6);

	 const auto theme = WinUtil::extractTheme(path);
	try {
		WinUtil::addTheme(theme, false);
	} catch(const Exception& e) {
		resize(dwt::Rectangle());
		auto err = Text::toT(e.getError());
		callAsync([this, err, path] {
			error(err, Text::toT(Util::getFileName(path)));
			endDialog(IDCANCEL);
				  });
		return true;
	}

	enum Type { Name, UUID, Version, Description, Author, Website };

	auto addInfo = [this](tstring name, const string& value, Type type) {
		if(type == Description) {
			grid->addRow(GridInfo(0, GridInfo::FILL, GridInfo::STRETCH));
		} else {
			grid->addRow();
		}
		grid->addChild(Label::Seed(name));
		if(type == Website && !value.empty()) {
			grid->addChild(Link::Seed(Text::toT(value), true, WinUtil::parseLink));
		} else {
			grid->addChild(Label::Seed(value.empty() ?
									   T_("<Information unavailable>") : Text::toT(value)));
		}
	};

	addInfo(T_("Name: "), theme.name, Name);
	addInfo(T_("UUID: "), theme.uuid, UUID);
	addInfo(T_("Version: "), Util::toString(theme.version), Version);
	addInfo(T_("Description: "), theme.description, Description);
	addInfo(T_("Author: "), theme.author, Author);
	addInfo(T_("Website: "), theme.website, Website);

	{
		grid->addRow();
		auto cur = grid->addChild(Grid::Seed(1, 2));
		grid->setWidget(cur, grid->rowCount() - 1, 0, 1, 2);
		cur->column(0).mode = GridInfo::FILL;
		cur->column(0).align = GridInfo::BOTTOM_RIGHT;
		cur->setSpacing(grid->getSpacing());
		WinUtil::addDlgButtons(cur,
							   [this, theme] { handleOK(theme); },
							   [this] { endDialog(IDCANCEL); })
			.first->setText(T_("Install theme"));
	}

	setText(T_("Adding theme"));

	layout();
	centerWindow();

	return false;
}

void ThemeInfoDlg::handleOK(const WinUtil::Theme& theme) {
	try {
		WinUtil::handleTheme(theme);
		endDialog(IDOK);
	} catch(const Exception& e) {
		error(Text::toT(e.getError()), Text::toT(theme.name));
		endDialog(IDCANCEL);
	}
}

void ThemeInfoDlg::layout() {
	dwt::Point sz = getClientSize();
	grid->resize(dwt::Rectangle(3, 3, sz.x - 6, sz.y - 6));
}

void ThemeInfoDlg::error(const tstring& message, const tstring& title) {
	dwt::MessageBox(this).show(tstring(T_("Cannot install the theme:")) + _T("\r\n\r\n") + message, title,
							   dwt::MessageBox::BOX_OK, dwt::MessageBox::BOX_ICONSTOP);
}