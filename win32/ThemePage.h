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

#ifndef DCPLUSPLUS_WIN32_THEME_DLG_H
#define DCPLUSPLUS_WIN32_THEME_DLG_H

#include <dcpp/forward.h>

#include "TypedTable.h"
#include "PropPage.h"
#include "WinUtil.h"

#include <dwt/include/dwt/widgets/Table.h>

class ThemePage : public PropPage
{
public:
	ThemePage(dwt::Widget* parent);
	virtual ~ThemePage();

	virtual void layout();
	virtual void write();
	
private:

	enum {
		GROUP_GENERAL,
		GROUP_TRANSFERS,
		GROUP_CHAT,

		GROUP_LAST
	};

	class ThemeData {
	public:
		ThemeData(tstring text, int textColorSetting, int bgColorSetting);
		virtual ~ThemeData() { }
		
		const tstring& getText(int) const;
		int getStyle(HFONT& font, COLORREF& textColor, COLORREF& bgColor, int) const;

		virtual int getTextColor() const { return textColorSetting; }
		virtual int getBgColor() const { return bgColorSetting; }

		const tstring text;

		virtual void update() { }
		virtual void write() { }

		COLORREF txtColor;
		COLORREF bgColor;

	private:
		const int textColorSetting;
		const int bgColorSetting;
	}; //ThemeData

	WinUtil::Theme theme {};

	ThemeData *globalData, *ulData, *dlData, *linkData, *logData;

	typedef TypedTable<ThemeData> Table;
	typedef Table* TTablePtr;
	TTablePtr table;

	TablePtr installed;
	GridPtr themeInfo;

	void update(ThemeData* const data);
	void handleSelectionChange();

	void refreshList();
	void addEntry(size_t idx, const string& uuid);
	string sel() const;
	void defaultTheme(WinUtil::Theme& theme);

};

#endif // !defined(DCPLUSPLUS_WIN32_THEME_DLG_H)
