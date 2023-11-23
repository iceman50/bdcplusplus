/*
* Copyright (C) 2001-2023 Jacek Sieka, arnetheduck on gmail point com
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
	virtual void write() {};

	struct Theme {
		string name;
		COLORREF textColor;
		COLORREF background;
		COLORREF uploadText;
		COLORREF uploadBg;
		COLORREF downloadText;
		COLORREF downloadBg;
		COLORREF linkColor;
		COLORREF logColor;
	};

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

		COLORREF txtColor;
		COLORREF bgColor;

	private:
		const int textColorSetting;
		const int bgColorSetting;
	}; //ThemeData


	ThemeData *globalData, *ulData, *dlData, *linkData, *logData;

	typedef TypedTable<ThemeData> Table;
	typedef Table* TTablePtr;
	TTablePtr table;
	GridPtr themeInfo;
	ComboBoxPtr cbCtrlTheme;
	ButtonPtr expTheme, modTheme;

	void update(ThemeData* const data);
	void refreshPreview();

	void currentTheme();
	void defaultTheme();
	void handleTheme();
	void populateThemes();

	typedef std::map<string, string> themeMap;
	themeMap themes;
	Theme defTheme, curTheme;

public:
	void loadTheme(const string& path);
	void saveTheme(const string& path);
};

#endif // !defined(DCPLUSPLUS_WIN32_THEME_DLG_H)
