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
#include "BdcppTextElementsPage.h"

#include <dcpp/File.h>
#include <dcpp/SdEx.h>
#include <dcpp/SimpleXML.h>
#include <dcpp/Text.h>
#include <dcpp/version.h>

#include <dwt/widgets/ColorDialog.h>
#include <dwt/widgets/FontDialog.h>
#include <dwt/widgets/Grid.h>
#include <dwt/widgets/LoadDialog.h>
#include <dwt/widgets/SaveDialog.h>

#include "HoldRedraw.h"
#include "resource.h"
#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;
using dwt::GroupBox;

BdcppTextElementsPage::BdcppTextElementsPage(dwt::Widget* parent) :
PropPage(parent, 1, 1), elementsTbl(0), iconSizeCb(0), addUserMatchingChk(0), addBtn(0), editBtn(0), upBtn(0), downBtn(0), removeBtn(0), importBtn(0), exportBtn(0) {

	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;

	auto cur = grid->addChild(GroupBox::Seed(T_(APPNAME " text elements")))->addChild(Grid::Seed(4, 1));
	cur->column(0).mode = GridInfo::FILL;
	cur->row(0).mode = GridInfo::FILL;
	cur->row(0).align = GridInfo::STRETCH;
	cur->setSpacing(grid->getSpacing());

	elementsTbl = cur->addChild(WinUtil::Seeds::Dialog::table);
	elementsTbl->onDblClicked([this] { handleDoubleClick(); });
	elementsTbl->onKeyDown([this](int c) { return handleKeyDown(c); });
	elementsTbl->onSelectionChanged([this] { handleSelectionChanged(); });
	elementsTbl->addColumn(T_("Name"));
	elementsTbl->addColumn(T_("Pattern"));
	elementsTbl->addColumn(T_("Image"));

	{
		auto grid = cur->addChild(Grid::Seed(1, 5));
		grid->column(0).mode = GridInfo::FILL;
		grid->column(1).mode = GridInfo::FILL;
		grid->column(2).mode = GridInfo::FILL;
		grid->column(3).mode = GridInfo::FILL;
		grid->column(4).mode = GridInfo::FILL;
		grid->setSpacing(cur->getSpacing());

		addBtn = grid->addChild(Button::Seed(T_("&Add")));
		addBtn->onClicked([this] { handleAddClicked(); });

		editBtn = grid->addChild(Button::Seed(T_("&Edit")));
		editBtn->onClicked([this] { handleEditClicked(); });

		upBtn = grid->addChild(Button::Seed(T_("Move &Up")));
		upBtn->onClicked([this] { handleUpClicked(); });

		downBtn = grid->addChild(Button::Seed(T_("Move &Down")));
		downBtn->onClicked([this] { handleDownClicked(); });

		removeBtn = grid->addChild(Button::Seed(T_("&Remove")));
		removeBtn->onClicked([this] { handleRemoveClicked(); });
	}

	{
		auto grid = cur->addChild(Grid::Seed(1, 5));
		grid->column(0).mode = GridInfo::FILL;
		grid->column(1).mode = GridInfo::FILL;
		grid->column(2).mode = GridInfo::FILL;
		grid->column(3).mode = GridInfo::FILL;
		grid->column(4).mode = GridInfo::FILL;
		grid->setSpacing(cur->getSpacing());

		importBtn = grid->addChild(Button::Seed(T_("Import")));
		importBtn->onClicked([this] { handleImportClicked(); });

		exportBtn = grid->addChild(Button::Seed(T_("Export")));
		exportBtn->onClicked([this] { handleExportClicked(); });

		auto label = grid->addChild(Label::Seed(T_("Icon size")));
		label->addRemoveStyle(ES_RIGHT, true);
		grid->setWidget(label, 0, 2, 1, 2);

		iconSizeCb = grid->addChild(WinUtil::Seeds::Dialog::comboBox);
		iconSizeCb->addValue(T_("no limit"));
		iconSizeCb->addValue(T_("16 pixels"));
		iconSizeCb->addValue(T_("24 pixels"));
		iconSizeCb->addValue(T_("32 pixels"));
		iconSizeCb->addValue(T_("48 pixels"));
	}

	{
		auto grid = cur->addChild(Grid::Seed(1, 2));
		grid->column(0).mode = GridInfo::FILL;
		grid->column(1).mode = GridInfo::FILL;
		grid->setSpacing(cur->getSpacing());

		addUserMatchingChk = grid->addChild(CheckBox::Seed(T_("Add user matching to text elements (only works with partial or exact match rules)")));
		grid->setWidget(addUserMatchingChk, 0, 0, 1, 2);
	}

	iconSizeCb->setSelected(max(0, min(BDSETTING(ICON_SIZE) / 8, 5) - 1));
	addUserMatchingChk->setChecked(BDSETTING(ADD_USER_MATCHING_TO_TEXT_ELEMENTS));

	for(const auto& ei: BDCManager::getInstance()->getElements())
		insert(ei);
	elementsTbl->ensureVisible(0);

	handleSelectionChanged();
}

void BdcppTextElementsPage::write() {
	// set ADD_USER_MATCHING_TO_TEXT_ELEMENTS first
	BDCManager::getInstance()->set(BDCManager::ADD_USER_MATCHING_TO_TEXT_ELEMENTS, addUserMatchingChk->getChecked());
	BDCManager::getInstance()->setElements(elements); // then update the text elements
	BDCManager::getInstance()->set(BDCManager::ICON_SIZE, ((iconSizeCb->getSelected() != 0) + iconSizeCb->getSelected() + (iconSizeCb->getSelected() == 4)) * 8);
}

void BdcppTextElementsPage::layout() {
	PropPage::layout();
	elementsTbl->setColumnWidth(0, (elementsTbl->getWindowSize().x - 30) * 1 / 5);
	elementsTbl->setColumnWidth(1, (elementsTbl->getWindowSize().x - 30) * 2 / 5);
	elementsTbl->setColumnWidth(2, (elementsTbl->getWindowSize().x - 30) * 2 / 5);
}

void BdcppTextElementsPage::insert(const TextElement& aElement, int index /*= -1*/) {
	TStringList tsl;
	tsl.push_back(Text::toT(aElement.name));
	tsl.push_back(Text::toT(aElement.getPattern()));
	tsl.push_back(Text::toT(aElement.imageFile));
	elementsTbl->insert(tsl, 0, index);
	if(index == -1)	{ elements.push_back(aElement); index = int(elements.size()-1);	}
	else			{ elements.insert(elements.begin() + index, aElement);			}
	elementsTbl->ensureVisible(index);
}

void BdcppTextElementsPage::erase(int index) {
	elementsTbl->erase(index);
	elements.erase(elements.begin() + index);
}

void BdcppTextElementsPage::handleDoubleClick() {
	if(elementsTbl->hasSelected())	{ handleEditClicked();	}
	else							{ handleAddClicked();	}
}

bool BdcppTextElementsPage::handleKeyDown(int c) {
	switch(c) {
	case VK_INSERT: handleAddClicked(); return true;
	case VK_DELETE: handleRemoveClicked(); return true;
	}
	return false;
}

void BdcppTextElementsPage::handleSelectionChanged() {
	auto sel = elementsTbl->countSelected();
	editBtn->setEnabled(sel != 0);
	upBtn->setEnabled(sel != 0);
	downBtn->setEnabled(sel != 0);
	removeBtn->setEnabled(sel != 0);
}

void BdcppTextElementsPage::handleAddClicked() {
	string sdex = "%[{ delimit() = ... ,1 , ";
	{
		string tmp = Bdcpp::delimiters;
		sdex += SdEx()->escape_data(tmp);
	}
	sdex += " }]";
	TextElement te("", sdex, Style("", 0, 0xFFFFFF, Bdcpp::STYLE_AUTOCOLOR | Bdcpp::STYLE_AUTOBACKCOLOR));
	BdcppTextElementDialog ted(this, te);
	if(ted.run() == IDOK)
		insert(ted.getElement());
}

void BdcppTextElementsPage::handleEditClicked() {
	int i = -1;
	while((i = elementsTbl->getNext(i, LVNI_SELECTED)) != -1) {
		auto tei = elements.begin() + i;
		BdcppTextElementDialog ted(this, *tei);
		if((ted.run() == IDOK) && !tei->isUserMatch) {
			*tei = ted.getElement();
			elementsTbl->setText(i, 0, Text::toT(tei->name));
			elementsTbl->setText(i, 1, Text::toT(tei->getPattern()));
			elementsTbl->setText(i, 2, Text::toT(tei->imageFile));
		}
	}
}

void BdcppTextElementsPage::handleUpClicked() {
	HoldRedraw hold(elementsTbl);
	for(const auto& i: elementsTbl->getSelection()) {
		if(i == 0)
			continue;
		TextElement te = *(elements.begin() + i);
		erase(i);
		insert(te, i - 1);
		elementsTbl->select(i - 1);
	}
}

void BdcppTextElementsPage::handleDownClicked() {
	HoldRedraw hold(elementsTbl);
	auto selected = elementsTbl->getSelection();
	for(auto i = selected.rbegin(); i != selected.rend(); ++i) {
		if(*i == (elementsTbl->size() - 1))
			continue;
		TextElement te = *(elements.begin() + *i);
		erase(*i);
		insert(te, *i + 1);
		elementsTbl->select(*i + 1);
	}
}

void BdcppTextElementsPage::handleRemoveClicked() {
	int i = -1;
	while((i = elementsTbl->getNext(i, LVNI_SELECTED)) != -1) {
		if((elements.begin()+i)->isUserMatch)
			continue; // skip user matches
		erase(i);
		i--; // reset to previous item for getNext
	}
}

void BdcppTextElementsPage::handleImportClicked() {
	tstring dir = Text::toT(Util::getPath(Util::PATH_USER_CONFIG));
	tstring target;
	if(!dwt::LoadDialog(this).addFilter(T_("XML files"), _T("*.xml")) .addFilter(T_("All files"), _T("*.*")) .setInitialDirectory(dir) .open(target))
		return;

	TextElement::List tel;
	BDCManager::getInstance()->loadTextElements(Text::fromT(target), tel);

	for(auto& tei: tel)
		insert(tei);
}

void BdcppTextElementsPage::handleExportClicked() {
	if(elementsTbl->getSelection().empty())
		return;

	tstring dir = Text::toT(Util::getPath(Util::PATH_USER_CONFIG));
	tstring target;
	if(!dwt::SaveDialog(this).addFilter(T_("XML files"), _T("*.xml")) .addFilter(T_("All files"), _T("*.*")) .setInitialDirectory(dir) .open(target))
		return;
	if((target.size() < 4) || (Text::toLower(target).find(_T(".xml")) != (target.size() - 4)))
		target += _T(".xml");

	TextElement::List tel;
	int i = -1;
	while((i = elementsTbl->getNext(i, LVNI_SELECTED)) != -1) {
		if((elements.begin()+i)->isUserMatch)
			continue; // do not save user match generated elements
		tel.push_back(*(elements.begin()+i));
	}
	BDCManager::getInstance()->saveTextElements(Text::fromT(target), tel);
}

/* BdcppTextElementDialog */
PropPage::ListItem BdcppTextElementDialog::stylesItems[] = {
	{ int(Bdcpp::STYLE_AUTOBACKCOLOR),			N_("Default background color"),		},
	{ int(Bdcpp::STYLE_AUTOCOLOR),				N_("Default text color"),			},
	{ int(Bdcpp::STYLE_AUTOFONT),				N_("Default text font"),			},
	{ int(Bdcpp::STYLE_ALLCAPS),				N_("Capitalize text"),				},
	{ int(Bdcpp::STYLE_BOLD),					N_("Bold text"),					},
	{ int(Bdcpp::STYLE_ITALIC),					N_("Italic text"),					},
	{ int(Bdcpp::STYLE_LINK),					N_("Hyperlink text"),				},
	{ int(Bdcpp::STYLE_STRIKEOUT),				N_("Strikeout text"),				},
	{ int(Bdcpp::STYLE_SUBSCRIPT),				N_("Subscript text"),				},
	{ int(Bdcpp::STYLE_SUPERSCRIPT),			N_("Superscript text"),				},
	{ int(Bdcpp::STYLE_UNDERLINE),				N_("Underline text"),				},
	{ int(Bdcpp::STYLE_UNDERLINE_DOT),			N_("Underline text: dots"),			},
	{ int(Bdcpp::STYLE_UNDERLINE_DASH),			N_("Underline text: dashes"),		},
	{ int(Bdcpp::STYLE_UNDERLINE_THICK),		N_("Underline text: thick"),		},
};

BdcppTextElementDialog::BdcppTextElementDialog(dwt::Widget* parent, const TextElement& _element) :
dwt::ModalDialog(parent), grid(0), stylesTbl(0), nameBox(0), sdexBox(0), resultBox(0), imageBox(0), patternBtn(0), imageBtn(0), fontBtn(0), textColorBtn(0), bgColorBtn(0), element(_element) {
	onInitDialog([=] { return handleInitDialog(); });
}

bool BdcppTextElementDialog::handleInitDialog() {

	grid = addChild(Grid::Seed(5, 2));
	grid->column(1).mode = GridInfo::FILL;
	grid->column(1).align = GridInfo::STRETCH;
	grid->row(4).mode = GridInfo::FILL;
	grid->row(4).align = GridInfo::STRETCH;

	grid->addChild(Label::Seed(T_("Name")));
	nameBox = grid->addChild(WinUtil::Seeds::Dialog::textBox);

	{
		Button::Seed seed;
		seed.caption = T_("Pattern");
		patternBtn = grid->addChild(seed);
		patternBtn->onClicked([this] { handlePatternClicked(); });
	}

	{
		auto seed = WinUtil::Seeds::Dialog::textBox;
		seed.lines = 3;
		seed.style &= ~ES_AUTOHSCROLL;
		seed.style |= WS_VSCROLL | WS_VSCROLL | ES_NOHIDESEL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL;
		sdexBox = grid->addChild(seed);
		sdexBox->onUpdated([this] { handleInputUpdated(); });
	}

	{
		grid->addChild(Label::Seed(T_("Errors")));
		auto seed = WinUtil::Seeds::Dialog::textBox;
		seed.lines = 2;
		seed.style &= ~ES_AUTOHSCROLL;
		seed.style |= ES_MULTILINE | WS_VSCROLL;
		resultBox = grid->addChild(seed);
		resultBox->setReadOnly(true);
		if(!element->supportsResult())
			resultBox->setVisible(false);
	}

	{
		Button::Seed seed;
		seed.caption = T_("Image");
		imageBtn = grid->addChild(seed);
		imageBtn->onClicked([this] { handleImageClicked(); });
		imageBox = grid->addChild(WinUtil::Seeds::Dialog::textBox);
	}

	auto cur = grid->addChild(Grid::Seed(1, 2));
	grid->setWidget(cur, 4, 0, 1, 2);
	cur->row(0).mode = GridInfo::FILL;
	cur->row(0).align = GridInfo::STRETCH;
	cur->column(0).mode = GridInfo::FILL;

	stylesTbl = cur->addChild(WinUtil::Seeds::Dialog::optionsTable);
	stylesTbl->addColumn();

	{
		auto grid = cur->addChild(Grid::Seed(5, 1));
		grid->row(3).mode = GridInfo::FILL;
		grid->row(3).align = GridInfo::BOTTOM_RIGHT;

		Button::Seed seed;

		seed.caption = T_("Text font");
		fontBtn = grid->addChild(seed);
		fontBtn->onClicked([this] { handleFontClicked(); });

		seed.caption = T_("Text color");
		textColorBtn = grid->addChild(seed);
		textColorBtn->onClicked([this] { handleTextColorClicked(); });
		
		seed.caption = T_("background color");
		bgColorBtn = grid->addChild(seed);
		bgColorBtn->onClicked([this] { handleBgColorClicked(); });

		::SetWindowLongPtr(WinUtil::addDlgButtons(grid, [this] { handleOKClicked(); }, [this] { endDialog(IDCANCEL); }) .first->handle(), GWLP_ID, 0); // the def button is the "Add" button
	}

	nameBox->setText(Text::toT(element.name));
	sdexBox->setText(Text::toT(element.getPattern()));
	imageBox->setText(Text::toT(element.imageFile));

	for(const auto& si: stylesItems)
		stylesTbl->setChecked(stylesTbl->insert(TStringList(1, T_(si.desc)), si.setting), si.setting & element.textStyle);
	stylesTbl->setColumnWidth(0, LVSCW_AUTOSIZE);

	if(element.isUserMatch) {
		nameBox->setEnabled(false);
		sdexBox->setEnabled(false);
		resultBox->setEnabled(false);
		stylesTbl->setEnabled(false);
		fontBtn->setEnabled(false);
		textColorBtn->setEnabled(false);
		bgColorBtn->setEnabled(false);
		imageBtn->setEnabled(false);
		imageBox->setEnabled(false);
		patternBtn->setEnabled(false);
	}

	setText(T_("Text element"));
	handleInputUpdated();
	layout();
	centerWindow();

	return false;
}

int BdcppTextElementDialog::run() {
	create(Seed(dwt::Point(400, 400), DS_CENTERMOUSE));
	return show();
}

void BdcppTextElementDialog::layout() {
	dwt::Point sz = getClientSize();
	grid->resize(dwt::Rectangle(3, 3, sz.x - 6, sz.y - 6));
}

void BdcppTextElementDialog::handleOKClicked() {
	if(!element.isUserMatch) {
		element.name = Text::fromT(nameBox->getText());
		(SdEx&)(element) = SdEx(Text::fromT(sdexBox->getText()));
		element.textStyle = 0;
		int i = -1;
		while((i = stylesTbl->getNext(i, LVNI_ALL)) != -1)
			element.textStyle |= stylesTbl->isChecked(i) ? int(stylesTbl->getData(i)) : 0;
		element.imageFile = Text::fromT(imageBox->getText());
	}
	endDialog(IDOK);
}

void BdcppTextElementDialog::handleInputUpdated() {
	SdEx sdex(Text::fromT(sdexBox->getText()));
	if(sdex->supportsResult())
		resultBox->setText(Text::toT(Text::toDOS(sdex->get_result())));
}

void BdcppTextElementDialog::handlePatternClicked() {
	BdcppTextElementEasyDialog dlg(this, Text::fromT(sdexBox->getText()));
	if(dlg.run() == IDOK) {
		sdexBox->setText(Text::toT(dlg.getPattern()));
		handleInputUpdated();
	}
}

void BdcppTextElementDialog::handleImageClicked() {
	string			tmp		= Util::validateFileName(Text::fromT(imageBox->getText()));
	const tstring	local	= Text::toT(Text::toLower(Util::getPath(Util::PATH_USER_CONFIG)));
	tstring			dir;
	tstring			target;

	if(File::isAbsolute(tmp))	{ target = Text::toT(tmp); dir = Text::toT(Util::getFilePath(tmp)); }
	else if(!tmp.empty())		{ target = Text::toT(Util::validateFileName(Text::fromT(local) + tmp)); dir = local; }
	else						{ dir = local; }

	if(dwt::LoadDialog(this).addFilter(T_("Bitmaps and icons"), _T("*.bmp;*.ico")) .addFilter(T_("All files"), _T("*.*")) .setInitialDirectory(dir).open(target)) {
		if(Text::toLower(target).find(local) == 0)
			target.erase(0, local.size());
		imageBox->setText(target);
	}
}

void BdcppTextElementDialog::handleFontClicked() {
	LOGFONT lf;
	WinUtil::decodeFont(Text::toT(element.font), lf);
	dwt::FontDialog::Options options;
	options.strikeout = false;
	options.underline = false;
	options.color = false;
	options.bgColor = element.bgColor;
	COLORREF cr = COLORREF(element.textColor);
	if(dwt::FontDialog(this).open(lf, cr, &options)) {
		element.font = Text::fromT(WinUtil::encodeFont(lf));
		element.textColor = int(cr);
	}
}

void BdcppTextElementDialog::handleTextColorClicked() {
	dwt::ColorDialog::ColorParams colorParams(element.textColor);
	if(dwt::ColorDialog(this).open(colorParams))
		element.textColor = colorParams.getColor();
}

void BdcppTextElementDialog::handleBgColorClicked() {
	dwt::ColorDialog::ColorParams colorParams(element.bgColor);
	if(dwt::ColorDialog(this).open(colorParams))
		element.bgColor = colorParams.getColor();
}

/* BdcppTextElementEasyDialog */
PropPage::ListItem BdcppTextElementEasyDialog::compareItems[] = {
	{ Bdcpp::DELIMIT_EQUALS,	N_("Text equals element"),						},
	{ Bdcpp::DELIMIT_CONTAINS,	N_("Text contains element"),					},
	{ Bdcpp::DELIMIT_ONLY,		N_("Just the element, disregard other text"),	},
	{ Bdcpp::DELIMIT_STARTS,	N_("Text starts with element"),					},
	{ Bdcpp::DELIMIT_ENDS,		N_("Text ends with element"),					},
	{ Bdcpp::DELIMIT_NOCASE,	N_("Element is not case-sensitive"),			},
};

BdcppTextElementEasyDialog::BdcppTextElementEasyDialog(dwt::Widget* aParent, const string& aPattern) : dwt::ModalDialog(aParent), grid(0), compareTbl(0), elementBox(0), delimitBox(0), pattern(aPattern) {
	onInitDialog([=] { return handleInitDialog(); });
}

bool BdcppTextElementEasyDialog::handleInitDialog() {

	grid = addChild(Grid::Seed(3, 1));
	grid->column(0).mode = GridInfo::FILL;
	grid->row(1).mode = GridInfo::FILL;
	grid->row(1).align = GridInfo::STRETCH;

	{
		auto cur = grid->addChild(Grid::Seed(2, 2));
		cur->column(1).mode = GridInfo::FILL;
		cur->column(1).align = GridInfo::STRETCH;

		cur->addChild(Label::Seed(T_("Element")));
		elementBox = cur->addChild(WinUtil::Seeds::Dialog::textBox);

		cur->addChild(Label::Seed(T_("Delimiters")));
		delimitBox = cur->addChild(WinUtil::Seeds::Dialog::textBox);
	}

	compareTbl = grid->addChild(WinUtil::Seeds::Dialog::optionsTable);
	compareTbl->addColumn();

	{
		auto cur = grid->addChild(Grid::Seed(1, 2));
		cur->column(0).mode = GridInfo::FILL;
		cur->column(0).align = GridInfo::BOTTOM_RIGHT;

		::SetWindowLongPtr(WinUtil::addDlgButtons(cur, [this] { handleOKClicked(); }, [this] { endDialog(IDCANCEL); }) .first->handle(), GWLP_ID, 0); // the def button is the "Add" button
	}

	for(const auto& ci: compareItems)
		compareTbl->setChecked(compareTbl->insert(TStringList(1, T_(ci.desc)), ci.setting), 0);
	compareTbl->setColumnWidth(0, LVSCW_AUTOSIZE);

	if(!pattern.empty()) {	// assume standard delimit
		const string::value_type*	pbegin	= &(*pattern.begin());
		const string::value_type*	pend	= &(*pattern.end());
		string::size_type			i		= Bdcpp::strfndfstof(pbegin, pend, "=:", 2);

		if(i != string::npos) {
			if(Bdcpp::strfnd(pbegin, pbegin+i, "nocase", 6) != string::npos)	{ compareTbl->setChecked(5, true);	} // 5 = DELIMIT_NOCASE
			else if(Bdcpp::strfnd(pbegin, pbegin+i, "nc", 2) != string::npos)	{ compareTbl->setChecked(5, true);	}

			pbegin += i + 1;
			const string::value_type* phi = pbegin;
			int id = 0;
			string delim;

			while(true) {
				i = Bdcpp::strfndfstof(phi, pend, ",;}\\", 4);
				bool end = false;

				if(i == string::npos) {
					phi = pend;
				} else {
					phi += i;
					if(*phi == '\\') {
						phi ++;
						phi += (phi != pend);
						continue;
					}
					end = (*phi == ';') || (*phi == '}');
				}

				switch(id) {
				case 0:		elementBox->setText(Text::toT(SdEx()->get_data(pbegin, phi))); break; // element
				case 1:		// compare type
					{
						int cmp = Bdcpp::toInt32(SdEx()->get_data(pbegin, phi));
						if(cmp & Bdcpp::DELIMIT_EQUALS)		{ compareTbl->setChecked(0, true); }
						if(cmp & Bdcpp::DELIMIT_CONTAINS)	{ compareTbl->setChecked(1, true); }
						if(cmp & Bdcpp::DELIMIT_ONLY)		{ compareTbl->setChecked(2, true); }
						if(cmp & Bdcpp::DELIMIT_STARTS)		{ compareTbl->setChecked(3, true); }
						if(cmp & Bdcpp::DELIMIT_ENDS)		{ compareTbl->setChecked(4, true); }
						if(cmp & Bdcpp::DELIMIT_NOCASE)		{ compareTbl->setChecked(5, true); }
						break;
					}
				case 2:		// begin delimiters
				case 3:		// end delimiters
				default:	delim += SdEx()->get_data(pbegin, phi); break;
				}
				id++;

				if(end || (phi == pend))
					break;
				phi++;
				pbegin = phi;
			}

			delimitBox->setText(Text::toT(Bdcpp::escape(delim)));
		}
	} else { // no pattern, at least fill the delimitBox with defaults
		string tmp = Bdcpp::delimiters;
		delimitBox->setText(Text::toT(Bdcpp::escape(tmp)));
	}

	setText(T_("Easy text element"));
	layout();
	centerWindow();

	return false;
}

int BdcppTextElementEasyDialog::run() {
	create(Seed(dwt::Point(250, 300), DS_CENTERMOUSE));
	return show();
}

void BdcppTextElementEasyDialog::layout() {
	dwt::Point sz = getClientSize();
	grid->resize(dwt::Rectangle(3, 3, sz.x-6, sz.y-6));
}

void BdcppTextElementEasyDialog::handleOKClicked() {
	pattern.clear();

	int s = 0;
	int i = -1;
	while((i = compareTbl->getNext(i, LVNI_ALL)) != -1)
		s |= compareTbl->isChecked(i) ? compareTbl->getData(i) : 0;

	pattern = "%[{ delimit";
	if(s & Bdcpp::DELIMIT_NOCASE) {
		pattern += "(nocase)";
		s &= ~Bdcpp::DELIMIT_NOCASE;
	}
	pattern += " = ";

	{
		string tmp = Text::fromT(elementBox->getText());
		pattern += SdEx()->escape_data(Bdcpp::checkEscape(tmp));
	}
	pattern += ", ";
	pattern += Util::toString(s);

	if(!(s & Bdcpp::DELIMIT_ONLY)) {
		string tmp = Text::fromT(delimitBox->getText());
		pattern += ", ";
		pattern += SdEx()->escape_data(Bdcpp::checkEscape(tmp));
	}
	pattern += " }]";
	endDialog(IDOK);
}
