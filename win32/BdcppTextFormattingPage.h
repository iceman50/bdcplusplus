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

#ifndef BDCPLUSPLUS_WIN32_BDCPP_TEXTFORMATTING_PAGE_H
#define BDCPLUSPLUS_WIN32_BDCPP_TEXTFORMATTING_PAGE_H

#include <dcpp/Bdcpp.h>
#include <dcpp/BDCManager.h>
#include <dcpp/typedefs.h>

#include <dwt/widgets/ModalDialog.h>

#include "forward.h"
#include "PropPage.h"

class BdcppTextFormattingPage : public PropPage {
public:
	BdcppTextFormattingPage(dwt::Widget* parent);
	virtual ~BdcppTextFormattingPage() { }
	
	virtual void layout();
	virtual void write();

private:
	static PropPage::ListItem settingsItems[];

	TablePtr formatsTbl;
	TablePtr settingsTbl;
	ButtonPtr importBtn;
	ButtonPtr exportBtn;
	ButtonPtr resetBtn;

	BDCManager::TextFormatList formats;

	void addFormat(const tstring& title, BDCManager::FormatSetting id, int type);
	tstring getFormat(const TextFormat& aFormat);

	void handleDoubleClick();
	void handleImportClicked();
	void handleExportClicked();
	void handleResetClicked();
};

class BdcppTextFormatDialog : public dwt::ModalDialog {
public:
	BdcppTextFormatDialog(dwt::Widget* parent, const tstring& aName, const TextFormat& aFormat, int aType);
	~BdcppTextFormatDialog() { }

	int run();

	const TextFormat& getFormat() const { return format; }

private:
	GridPtr grid;
	TablePtr formatTbl;
	RichTextBoxPtr exampleBox;
	ButtonPtr addBtn;
	ButtonPtr editBtn;
	ButtonPtr upBtn;
	ButtonPtr downBtn;
	ButtonPtr removeBtn;

	TextFormat format;
	const tstring name;
	int type;
	
	bool handleInitDialog();
	void layout();
	void handleOKClicked();
	
	void insert(const TextFormatInfo& aFormatInfo, int index = -1);
	void erase(int index);
	void updateExample();
	string getFontName(const string& aFont);

	void handleDoubleClick();
	bool handleKeyDown(int c);
	void handleSelectionChanged();
	void handleAddClicked();
	void handleMoveUpClicked();
	void handleMoveDownClicked();
	void handleEditClicked();
	void handleRemoveClicked();
};

class BdcppTextFormatInfoDialog : public dwt::ModalDialog {
public:
	BdcppTextFormatInfoDialog(dwt::Widget* parent, const TextFormatInfo& _formatInfo);
	virtual ~BdcppTextFormatInfoDialog() { }

	int run();

	const TextFormatInfo& getFormatInfo() { return formatInfo; }

private:
	static PropPage::ListItem stylesItems[];

	GridPtr grid;
	TablePtr stylesTbl;
	TextBoxPtr sdexBox;
	TextBoxPtr resultBox;
	CheckBoxPtr formatWholeChk;
	ButtonPtr patternBtn;
	ButtonPtr fontBtn;
	ButtonPtr textColorBtn;
	ButtonPtr bgColorBtn;

	TextFormatInfo formatInfo;

	bool handleInitDialog();
	void layout();
	void handleOKClicked();

	void handleInputUpdated();
	void handlePatternClicked();
	void handleFontClicked();
	void handleTextColorClicked();
	void handleBgColorClicked();
};

#endif // !defined(BDCPLUSPLUS_WIN32_BDCPP_TEXTFORMATTING_PAGE_H)
