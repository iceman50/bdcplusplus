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

#ifndef BDCPLUSPLUS_WIN32_BDCPP_TEXTELEMENTS_PAGE_H
#define BDCPLUSPLUS_WIN32_BDCPP_TEXTELEMENTS_PAGE_H

#include <dcpp/BDCManager.h>
#include <dcpp/typedefs.h>

#include <dwt/widgets/ModalDialog.h>

#include "PropPage.h"

class BdcppTextElementsPage : public PropPage {
public:
	BdcppTextElementsPage(dwt::Widget* parent);
	virtual ~BdcppTextElementsPage() { }

	virtual void write();
	virtual void layout();

private:
	TablePtr elementsTbl;
	ComboBoxPtr iconSizeCb;
	CheckBoxPtr addUserMatchingChk;
	ButtonPtr addBtn;
	ButtonPtr editBtn;
	ButtonPtr upBtn;
	ButtonPtr downBtn;
	ButtonPtr removeBtn;
	ButtonPtr importBtn;
	ButtonPtr exportBtn;

	TextElement::List elements;

	void insert(const TextElement& aElement, int index = -1);
	void erase(int index);

	void handleDoubleClick();
	bool handleKeyDown(int c);
	void handleSelectionChanged();
	void handleAddClicked();
	void handleEditClicked();
	void handleUpClicked();
	void handleDownClicked();
	void handleRemoveClicked();
	void handleImportClicked();
	void handleExportClicked();
};

class BdcppTextElementDialog : public dwt::ModalDialog {
public:
	BdcppTextElementDialog(dwt::Widget* parent, const TextElement& aElement);
	virtual ~BdcppTextElementDialog() { }
	
	int run();

	const TextElement& getElement() const { return element; }

private:
	static PropPage::ListItem stylesItems[];

	GridPtr grid;
	TablePtr stylesTbl;
	TextBoxPtr nameBox;
	TextBoxPtr sdexBox;
	TextBoxPtr resultBox;
	TextBoxPtr imageBox;
	ButtonPtr patternBtn;
	ButtonPtr imageBtn;
	ButtonPtr fontBtn;
	ButtonPtr textColorBtn;
	ButtonPtr bgColorBtn;

	TextElement element;

	bool handleInitDialog();
	void layout();
	void handleOKClicked();

	void handleInputUpdated();
	void handlePatternClicked();
	void handleImageClicked();
	void handleFontClicked();
	void handleTextColorClicked();
	void handleBgColorClicked();
};

class BdcppTextElementEasyDialog : public dwt::ModalDialog {
public:
	BdcppTextElementEasyDialog(dwt::Widget* aParent, const string& aPattern);
	virtual ~BdcppTextElementEasyDialog() { }
	
	int run();

	const string& getPattern() const { return pattern; }

private:
	static PropPage::ListItem compareItems[];

	GridPtr grid;
	TablePtr compareTbl;
	TextBoxPtr elementBox;
	TextBoxPtr delimitBox;

	string pattern;

	bool handleInitDialog();
	void layout();
	void handleOKClicked();
};

#endif // !defined(BDCPLUSPLUS_WIN32_BDCPP_TEXTELEMENTS_PAGE_H)
