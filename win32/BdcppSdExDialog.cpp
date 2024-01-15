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

#include "stdafx.h"
#include "BdcppSdExDialog.h"

#include <dcpp/Bdcpp.h>
#include <dcpp/BDCManager.h>
#include <dcpp/SdEx.h>
#include <dcpp/version.h>

#include <dwt/widgets/Grid.h>
#include <dwt/widgets/MessageBox.h>

#include "ParamDlg.h"
#include "resource.h"
#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;
using dwt::Spinner;

BdcppSdExDialog::BdcppSdExDialog(dwt::Widget* parent, const tstring& aTitle, const string& aPattern, const StringList& aParams) :
dwt::ModalDialog(parent), grid(0), paramsTbl(0), functionsTbl(0), sdexBox(0), resultBox(0), addParamBtn(0), addTagBtn(0), sdex(aPattern), params(aParams) {
	onInitDialog([=] { return handleInitDialog(aTitle); });
}

bool BdcppSdExDialog::handleInitDialog(const tstring& aTitle) {

	grid = addChild(Grid::Seed(4, 1));
	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;
	grid->row(1).align = GridInfo::STRETCH;
	grid->row(2).align = GridInfo::STRETCH;
	grid->row(3).mode = GridInfo::FILL;
	grid->row(3).align = GridInfo::STRETCH;

	{
		auto cur = grid->addChild(Grid::Seed(1, 2));
		cur->column(0).mode = GridInfo::FILL;
		cur->row(0).align = GridInfo::STRETCH;

		auto seed = WinUtil::Seeds::Dialog::textBox;
		seed.lines = 3;
		seed.style &= ~ES_AUTOHSCROLL;
		seed.style |= WS_VSCROLL | WS_VSCROLL | ES_NOHIDESEL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL;
		sdexBox = cur->addChild(seed);
		sdexBox->onUpdated([this] { handleInputUpdated(); });

		WinUtil::addDlgButtons(cur->addChild(Grid::Seed(2, 1)), [this] { handleOKClicked(); }, [this] { endDialog(IDCANCEL); });
	}

	{
		auto seed = WinUtil::Seeds::Dialog::textBox;
		seed.lines = 2;
		seed.style &= ~ES_AUTOHSCROLL;
		seed.style |= ES_MULTILINE | WS_VSCROLL;
		resultBox = grid->addChild(seed);
		resultBox->setReadOnly(true);
		if(!sdex->supportsResult())
			resultBox->setVisible(false);
	}

	{
		auto cur = grid->addChild(Grid::Seed(1, 3));
		cur->column(0).mode = GridInfo::FILL;
		cur->column(0).align = GridInfo::STRETCH;
		cur->addChild(Label::Seed(T_("Create an SdEx parameter with or without SdEx tag or just add an SdEx tag to a parameter.")));

		addParamBtn = cur->addChild(Button::Seed(T_("Insert parameter")));
		addParamBtn->onClicked([this] { handleAddParamClicked(); });

		addTagBtn = cur->addChild(Button::Seed(T_("Insert tag")));
		addTagBtn->onClicked([this] { handleAddTagClicked(); });
		if(!sdex->supportsFunctions())
			addTagBtn->setVisible(false);
	}

	{
		auto cur = grid->addChild(Grid::Seed(1, 5));
		cur->row(0).mode = GridInfo::FILL;
		cur->row(0).align = GridInfo::STRETCH;
		cur->column(0).mode = GridInfo::FILL;
		cur->column(1).mode = GridInfo::FILL;
		cur->column(2).mode = GridInfo::FILL;
		cur->column(3).mode = GridInfo::FILL;
		cur->column(4).mode = GridInfo::FILL;

		auto seed = WinUtil::Seeds::Dialog::table;
		seed.style |= LVS_SHOWSELALWAYS | LVS_SINGLESEL;

		paramsTbl = cur->addChild(seed);
		paramsTbl->addColumn(T_("Parameter"));

		functionsTbl = cur->addChild(seed);
		cur->setWidget(functionsTbl, 0, 1, 1, 4);
		functionsTbl->addColumn(T_("Function"));
		functionsTbl->addColumn(T_("Description"));
		if(!sdex->supportsFunctions())
			functionsTbl->setVisible(false);
	}

	sdexBox->setText(Text::toT(sdex.getPattern()));

	paramsTbl->insert(TStringList(1, Util::emptyStringT));
	for(const auto& si: params)
		paramsTbl->insert(TStringList(1, Text::toT(si)));
	paramsTbl->setSelected(0);

	{
		TStringList empty;
		empty.push_back(Util::emptyStringT);
		empty.push_back(T_("Empty, no SdEx function will be added to the parameter"));
		functionsTbl->insert(empty);

		if(sdex->supportsFunctions()) {
			SdExFunctionBase::List sdexfunctions;
			sdex->get_functions(sdexfunctions);
			for(const auto& fi: sdexfunctions) {
				TStringList tsl;
				tsl.push_back(Text::toT(fi.cmd));
				tsl.push_back(Text::toT(fi.description));
				functionsTbl->insert(tsl, fi.components);
			}
		}

		functionsTbl->setSelected(0);
	}

	setText(aTitle);
	handleInputUpdated();
	layout();
	centerWindow();

	return false;
}

int BdcppSdExDialog::run() {
	create(Seed(dwt::Point(800, 400), DS_CENTERMOUSE));
	return show();
}

void BdcppSdExDialog::layout() {
	dwt::Point sz = getClientSize();
	grid->resize(dwt::Rectangle(3, 3, sz.x - 6, sz.y - 6));
	paramsTbl->setColumnWidth(0, (paramsTbl->getWindowSize().x - 30));
	functionsTbl->setColumnWidth(0, (functionsTbl->getWindowSize().x - 30) * 1 / 5);
	functionsTbl->setColumnWidth(1, (functionsTbl->getWindowSize().x - 30) * 4 / 5);
}

void BdcppSdExDialog::handleOKClicked() {
	sdex = SdEx(Text::fromT(sdexBox->getText()));
	endDialog(IDOK);
}

bool BdcppSdExDialog::getTag(tstring& param, const tstring& aCmd, const tstring& aDescription, int aOps, const tstring& aParameter /*= Util::emptyStringT*/) {
	if(aCmd.empty())
		return true; // valid use of getTag, just no function to process

	tstring		title	= (aParameter.empty() ? Util::emptyStringT : aParameter + _T(" --- ")) + aCmd + _T(" --- ");
	auto		i		= aDescription.find(_T(" - "));
	int			counter	= 1;
	TStringList	values;
	TStringList	fields;
	TStringList	action;
	tstring		mods;

	if(i == string::npos)	{ fields = StringTokenizer<tstring>(aDescription, L',').getTokens();				}
	else					{ fields = StringTokenizer<tstring>(aDescription.substr(0, i), L',').getTokens();	}
	
	action.push_back(_T("1 - ") + T_("Add value to the SdEx tag"));
	action.push_back(_T("2 - ") + T_("Finalize SdEx tag"));

	{
		BdcppSdExModifierDialog dlg(this, title + T_("modifiers"));
		if(dlg.run() != IDOK)
			return false; // not validated
		int m = dlg.getModifiers();

#define SDEX_MODIFIER(modtype, modname) if(m & SdExFunctionBase::modtype) { if(!mods.empty()) { mods += L','; } mods += _T(modname); m &= ~SdExFunctionBase::modtype; }

		SDEX_MODIFIER(OP_NAND,		"nand");
		SDEX_MODIFIER(OP_NOR,		"nor");
		SDEX_MODIFIER(OP_NXOR,		"nxor");
		SDEX_MODIFIER(OP_NXNOR,		"nxnor");
		SDEX_MODIFIER(OP_AND,		"and");
		SDEX_MODIFIER(OP_OR,		"or");
		SDEX_MODIFIER(OP_XOR,		"xor");
		SDEX_MODIFIER(OP_XNOR,		"xnor");
		SDEX_MODIFIER(OP_NOT,		"not");
		SDEX_MODIFIER(OP_NOCASE,	"nocase");
		SDEX_MODIFIER(OP_FORCECASE,	"forcecase");
	}

	while(true) {
		ParamDlg dlg(this, title + T_("value") + _T(" ") + Text::toT(Util::toString(counter)));

		if(!values.empty())
			dlg.addComboBox(T_("Values already added to the SdEx tag"), values, 0, false);

		int index = 0;
		for(const auto& tsi: fields) {
			int op = (aOps >> (16 + index * 4) & 0xF);
			if((op == SdExFunctionBase::CPNT_INT32) || (op == SdExFunctionBase::CPNT_INT64))
					{ dlg.addIntTextBox(T_("Component ") + tsi, _T("0"));	}
			else	{ dlg.addTextBox(T_("Component ") + tsi);				}
			index++;
		}

		dlg.addComboBox(T_("Action on OK"), action, 0, false);

		if(dlg.run() != IDOK)
			return false;

		const TStringList& components = dlg.getValues();
		if(components.back()[0] == '2') {
			if(!values.empty())
				break;
			dwt::MessageBox(this).show(T_("No values have been added to the SdEx tag yet!"), _T(APPNAME) _T(" ") _T(VERSIONSTRING), dwt::MessageBox::BOX_OK, dwt::MessageBox::BOX_ICONEXCLAMATION);
			continue;
		}

		tstring v;
		for(auto tsi = components.begin()+!values.empty(); tsi != (components.end()-1); ++tsi) {
			if(!v.empty())
				v += L',';
			if((aOps >> (16 + (tsi-components.begin()-!values.empty()) * 4) & 0xF) == SdExFunctionBase::CPNT_SDEX) {
				v += *tsi;
			} else {
				string c = Text::fromT(*tsi);
				v += Text::toT(sdex->escape_data(c));
			}
		}
		values.push_back(v);

		counter++;
	}

	tstring vs;
	for(const auto& v: values) {
		if(!vs.empty())
			vs += L';';
		vs += v;
	}

	param += _T("{") + aCmd;
	if(!mods.empty())
		param += _T("(") + mods + _T(")");
	param += (aOps & SdExFunctionBase::OP_SYSTEM) ? _T(":") : _T("=");
	param += vs + _T("}");
	return true; // everything added
}

void BdcppSdExDialog::handleInputUpdated() {
	SdEx sdex(Text::fromT(sdexBox->getText()));
	if(sdex->supportsResult())
		resultBox->setText(Text::toT(Text::toDOS(sdex->get_result())));
}

void BdcppSdExDialog::handleAddParamClicked() {
	tstring param;
	tstring name;

	if(paramsTbl->hasSelected())
		name = paramsTbl->getText(paramsTbl->getSelected(), 0);

	param += _T("%[");
	param += name;

	if(functionsTbl->hasSelected() && (functionsTbl->getSelected() != 0)) {
		if(!getTag(param, functionsTbl->getText(functionsTbl->getSelected(), 0), functionsTbl->getText(functionsTbl->getSelected(), 1), int(functionsTbl->getData(functionsTbl->getSelected())), name))
			return;
	}

	param += _T("]");

	auto	cr	= sdexBox->getCaretPosRange();
	tstring	old	= sdexBox->getText();
	tstring	txt	= old.substr(0, cr.first) + param + old.substr(cr.second);

	sdexBox->setText(txt);
	sdexBox->setSelection(cr.first, cr.first + param.size());
	sdexBox->setFocus();
}

void BdcppSdExDialog::handleAddTagClicked() {
	tstring tag;
	if(!getTag(tag, functionsTbl->getText(functionsTbl->getSelected(), 0), functionsTbl->getText(functionsTbl->getSelected(), 1), int(functionsTbl->getData(functionsTbl->getSelected()))))
		return;

	auto	cr	= sdexBox->getCaretPosRange();
	tstring	old	= sdexBox->getText();
	tstring	txt	= old.substr(0, cr.first) + tag + old.substr(cr.second);

	sdexBox->setText(txt);
	sdexBox->setSelection(cr.first, cr.first + tag.size());
	sdexBox->setFocus();
}

/* BdcppSdExModifierDialog */
PropPage::ListItem BdcppSdExModifierDialog::modifierItems[] = {
	{ int(SdExFunctionBase::OP_AND),		N_("AND - all values true"),						},
	{ int(SdExFunctionBase::OP_OR),			N_("OR - any value true"),							},
	{ int(SdExFunctionBase::OP_XOR),		N_("XOR - only one value true"),					},
	{ int(SdExFunctionBase::OP_XNOR),		N_("XNOR - all values true or all values false"),	},
	{ int(SdExFunctionBase::OP_NOT),		N_("NOT - invert true/false"),						},
	{ int(SdExFunctionBase::OP_NAND),		N_("NAND - all values false"),						},
	{ int(SdExFunctionBase::OP_NOR),		N_("NOR - any value false"),						},
	{ int(SdExFunctionBase::OP_NXOR),		N_("NXOR - only one value false"),					},
	{ int(SdExFunctionBase::OP_NXNOR),		N_("NXNOR - all values false or all values true"),	},
	{ int(SdExFunctionBase::OP_NOCASE),		N_("NOCASE - not case sensitive"),					},
	{ int(SdExFunctionBase::OP_FORCECASE),	N_("FORCECASE - always case sensitive"),			},
};

BdcppSdExModifierDialog::BdcppSdExModifierDialog(dwt::Widget* parent, const tstring& aTitle) : dwt::ModalDialog(parent), grid(0), modifierTbl(0), modifiers(0) {
	onInitDialog([=] { return handleInitDialog(aTitle); });
}

bool BdcppSdExModifierDialog::handleInitDialog(const tstring& aTitle) {

	grid = addChild(Grid::Seed(1, 2));
	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(1).align = GridInfo::STRETCH;
	grid->row(1).mode = GridInfo::FILL;
	
	modifierTbl = grid->addChild(WinUtil::Seeds::Dialog::optionsTable);
	modifierTbl->addColumn();

	WinUtil::addDlgButtons(grid->addChild(Grid::Seed(2, 1)), [this] { handleOKClicked(); }, [this] { endDialog(IDCANCEL); });
	
	for(auto& mi: modifierItems)
		modifierTbl->setChecked(modifierTbl->insert(TStringList(1, T_(mi.desc)), LPARAM(mi.setting)), 0);
	modifierTbl->setColumnWidth(0, LVSCW_AUTOSIZE);

	setText(aTitle);
	layout();
	centerWindow();

	return false;
}

int BdcppSdExModifierDialog::run() {
	create(Seed(dwt::Point(400, 200), DS_CENTERMOUSE));
	return show();
}

void BdcppSdExModifierDialog::layout() {
	dwt::Point sz = getClientSize();
	grid->resize(dwt::Rectangle(3, 3, sz.x - 6, sz.y - 6));
}

void BdcppSdExModifierDialog::handleOKClicked() {
	modifiers = 0;
	int i = -1;
	while((i = modifierTbl->getNext(i, LVNI_ALL)) != -1)
		modifiers |= modifierTbl->isChecked(i) ? int(modifierTbl->getData(i)) : 0;
	endDialog(IDOK);
}
