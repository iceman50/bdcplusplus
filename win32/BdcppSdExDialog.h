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

#ifndef DCPLUSPLUS_WIN32_SDEX_DIALOG_H
#define DCPLUSPLUS_WIN32_SDEX_DIALOG_H

#include <dcpp/SdEx.h>
#include <dcpp/Util.h>

#include <dwt/widgets/ModalDialog.h>

#include "forward.h"
#include "PropPage.h"

class BdcppSdExDialog : public dwt::ModalDialog {
public:
	BdcppSdExDialog(dwt::Widget* parent, const tstring& aTitle, const string& aPattern, const StringList& aParams);
	virtual ~BdcppSdExDialog() { }
	
	int run();

	const SdEx& getSdEx() const { return sdex; }

private:
	GridPtr grid;
	TablePtr paramsTbl;
	TablePtr functionsTbl;
	TextBoxPtr sdexBox;
	TextBoxPtr resultBox;
	ButtonPtr addParamBtn;
	ButtonPtr addTagBtn;

	SdEx sdex;
	StringList params;

	bool handleInitDialog(const tstring& aTitle);
	void layout();
	void handleOKClicked();

	bool getTag(tstring& param, const tstring& aCmd, const tstring& aDescription, int aOps, const tstring& aParameter = Util::emptyStringT);

	void handleInputUpdated();
	void handleAddParamClicked();
	void handleAddTagClicked();
};

class BdcppSdExModifierDialog : public dwt::ModalDialog {
public:
	BdcppSdExModifierDialog(dwt::Widget* parent, const tstring& aTitle);
	virtual ~BdcppSdExModifierDialog() { }

	int run();

	int getModifiers() const { return modifiers; }

private:
	static PropPage::ListItem modifierItems[];

	GridPtr grid;
	TablePtr modifierTbl;

	int modifiers;

	bool handleInitDialog(const tstring& aTitle);
	void layout();
	void handleOKClicked();
};

#endif // !defined(DCPLUSPLUS_WIN32_SDEX_DIALOG_H)
