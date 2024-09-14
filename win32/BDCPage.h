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

#ifndef DCPLUSPLUS_WIN32_BDC_PAGE_H
#define DCPLUSPLUS_WIN32_BDC_PAGE_H

#include "PropPage.h"

class BDCPage : public PropPage
{
public:
	BDCPage(dwt::Widget* parent);
	virtual ~BDCPage();

	virtual void write();

private:
	static ListItem listItems[];
	TablePtr options;

	ComboBoxPtr actionDBLClickUserCb;
	TextBoxPtr iconPath;
	ButtonPtr browseIcon, testLogger, tabText, sdexVersion;

	void testBDCLogger();
	void handleSdexVersion();
	void handleTabText();

	void handleBrowseIcon();
};

#endif // !defined(DCPLUSPLUS_WIN32_BDC_PAGE_H)
