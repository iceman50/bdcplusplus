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
#include "BdcppTextFormattingPage.h"

#include <dcpp/BDCManager.h>
#include <dcpp/SdEx.h>
#include <dcpp/SettingsManager.h>
#include <dcpp/version.h>

#include <dwt/widgets/ColorDialog.h>
#include <dwt/widgets/FontDialog.h>
#include <dwt/widgets/Grid.h>
#include <dwt/widgets/LoadDialog.h>
#include <dwt/widgets/SaveDialog.h>

#include "BDCRichText.h"
#include "BdcppSdExDialog.h"
#include "HoldRedraw.h"
#include "resource.h"
#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;
using dwt::FolderDialog;
using dwt::FontDialog;

PropPage::ListItem BdcppTextFormattingPage::settingsItems[] = {
	{ BDCManager::ADJUST_CHAT_TO_DPI,				N_("Adjust the font size in chat according to windows DPI settings"),		},
	{ BDCManager::LINK_FORMATTING,					N_("Enable link formatting in chat"),										},
	{ BDCManager::FRIENDLY_LINKS,					N_("Make links user-friendly in chat"),										},
	{ BDCManager::USER_MATCHING_IN_CHAT,			N_("Use user matching nick coloring in chat"),								},
	{ BDCManager::TEXT_FORMAT_ADD_PARAMS,			N_("Add extra \"hub\", \"my\" and \"user\" parameters to text formatting"),	},
	{ BDCManager::BDCPP_TEXT_FORMATTING_EXTENDED,	N_("Enable extended " APPNAME " text formatting"),							},
};

BdcppTextFormattingPage::BdcppTextFormattingPage(dwt::Widget* parent) : PropPage(parent, 1, 1), formatsTbl(0), settingsTbl(0), importBtn(0), exportBtn(0), resetBtn(0) {

	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;

	auto group = grid->addChild(GroupBox::Seed(T_(APPNAME " text formatting")));

	auto cur = group->addChild(Grid::Seed(3, 1));
	cur->column(0).mode = dwt::GridInfo::FILL;
	cur->row(0).mode = dwt::GridInfo::FILL;
	cur->row(0).align = dwt::GridInfo::STRETCH;
	cur->row(2).mode = dwt::GridInfo::FILL;
	cur->row(2).align = dwt::GridInfo::STRETCH;

	formatsTbl = cur->addChild(WinUtil::Seeds::Dialog::table);
	formatsTbl->onDblClicked([this] { handleDoubleClick(); });

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

		resetBtn = grid->addChild(Button::Seed(T_("Reset to default")));
		resetBtn->onClicked([this] { handleResetClicked(); });
	}

	settingsTbl = cur->addChild(WinUtil::Seeds::Dialog::optionsTable);

	formatsTbl->addColumn(T_("Type"));
	formatsTbl->addColumn(T_("Format"));
	addFormat(T_("Chat message format (incoming, outgoing, PMs, /me)"),	BDCManager::FORMAT_CHAT, 0);
	addFormat(T_("Chat status message format"),							BDCManager::FORMAT_CHAT_STATUS, 1);
	addFormat(T_("System log formatting"),								BDCManager::FORMAT_SYSTEMLOG,	2);

	settingsTbl->addColumn();
	for(auto& si: settingsItems)
		settingsTbl->setChecked(settingsTbl->insert(TStringList(1, T_(si.desc)), si.setting), BDCManager::getInstance()->get(BDCManager::BoolSetting(si.setting)));
	settingsTbl->setColumnWidth(0, LVSCW_AUTOSIZE);
}

void BdcppTextFormattingPage::write() {
	int i = -1;
	while((i = settingsTbl->getNext(i, LVNI_ALL)) != -1)
		BDCManager::getInstance()->set(BDCManager::BoolSetting(settingsTbl->getData(i)), settingsTbl->isChecked(i));
	for(const auto& fi: formats) {
		BDCManager::FormatSetting key = BDCManager::getFormatSetting(fi.first);
		if(key == BDCManager::FORMAT_LAST)
			continue;
		BDCManager::getInstance()->set(key, fi.second);
	}
}

void BdcppTextFormattingPage::layout() {
	PropPage::layout();
	formatsTbl->setColumnWidth(0, (formatsTbl->getWindowSize().x - 30) * 2 / 5);
	formatsTbl->setColumnWidth(1, (formatsTbl->getWindowSize().x - 30) * 3 / 5);
}

void BdcppTextFormattingPage::addFormat(const tstring& line, BDCManager::FormatSetting id, int type) {
	const TextFormat& tf = BDCManager::getInstance()->get(id);
	TStringList tsl;
	tsl.push_back(line);
	tsl.push_back(getFormat(tf));
	formatsTbl->insert(tsl, type);
	formats.emplace_back(BDCManager::getSettingTag(id), tf);
}

tstring BdcppTextFormattingPage::getFormat(const TextFormat& aFormat) {
	string f;
	for(const auto& fi: aFormat)
		f += fi.getPattern();
	return Text::toT(f);
}

void BdcppTextFormattingPage::handleDoubleClick() {
	if(!formatsTbl->hasSelected())
		return;
	int i = -1;
	while((i = formatsTbl->getNext(i, LVNI_SELECTED)) != -1) {
		auto fli = (formats.begin() + i);
		BdcppTextFormatDialog dlg(this, Text::toT(fli->first), fli->second, formatsTbl->getData(i));
		if(dlg.run() == IDOK) {
			fli->second = dlg.getFormat();
			formatsTbl->setText(i, 1, getFormat(fli->second));
		}
	}
}

void BdcppTextFormattingPage::handleImportClicked() {
	tstring dir = Text::toT(Util::getPath(Util::PATH_USER_CONFIG));
	tstring target;
	if(!dwt::LoadDialog(this).addFilter(T_("XML files"), _T("*.xml")) .addFilter(T_("All files"), _T("*.*")) .setInitialDirectory(dir) .open(target))
		return;

	BDCManager::TextFormatList tfl;
	BDCManager::getInstance()->loadChatFormats(Text::fromT(target), tfl);

	for(const auto& tfi: tfl) {
		for(auto fi = formats.begin(); fi != formats.end(); ++fi) {
			if(fi->first == tfi.first) {
				fi->second = tfi.second;
				formatsTbl->setText(fi-formats.begin(), 1, getFormat(fi->second));
				break;
			}
		}
	}
}

void BdcppTextFormattingPage::handleExportClicked() {
	if(formatsTbl->getSelection().empty())
		return;

	tstring dir = Text::toT(Util::getPath(Util::PATH_USER_CONFIG));
	tstring target;
	if(!dwt::SaveDialog(this).addFilter(T_("XML files"), _T("*.xml")) .addFilter(T_("All files"), _T("*.*")) .setInitialDirectory(dir) .open(target))
		return;
	if(target.find(_T(".xml")) != target.size()-4)
		target += _T(".xml");

	BDCManager::TextFormatList tfl;
	{
		int i = -1;
		while((i = formatsTbl->getNext(i, LVNI_SELECTED)) != -1)
			tfl.emplace_back(*(formats.begin()+i));
	}

	BDCManager::getInstance()->saveChatFormats(Text::fromT(target), tfl);
}

void BdcppTextFormattingPage::handleResetClicked() {
	if(formatsTbl->getSelection().empty())
		return;
	int i = -1;
	while((i = formatsTbl->getNext(i, LVNI_SELECTED)) != -1) {
		BDCManager::FormatSetting key = BDCManager::getFormatSetting((formats.begin()+i)->first);
		if(key == BDCManager::FORMAT_LAST)
			continue;
		(formats.begin()+i)->second = BDCManager::getInstance()->resetDefault(key);
		formatsTbl->setText(i, 1, getFormat((formats.begin()+i)->second));
	}
}

/* BdcppTextFormatDialog */
BdcppTextFormatDialog::BdcppTextFormatDialog(dwt::Widget* parent, const tstring& aName, const TextFormat& aFormat, int aType) :
dwt::ModalDialog(parent), grid(0), formatTbl(0), exampleBox(0), addBtn(0), editBtn(0), upBtn(0), downBtn(0), removeBtn(0), format(aFormat), name(aName), type(aType) {	
	onInitDialog([=] { return handleInitDialog(); });
}

bool BdcppTextFormatDialog::handleInitDialog() {

	grid = addChild(Grid::Seed(2, 1));
	grid->column(0).mode = GridInfo::FILL;
	grid->row(1).mode = GridInfo::FILL;
	grid->row(1).align = GridInfo::STRETCH;

	{
		auto seed = WinUtil::Seeds::richTextBox;
		seed.lines = 6;
		seed.style &= ~ES_AUTOHSCROLL;
		seed.style |= ES_MULTILINE | WS_VSCROLL;
		exampleBox = grid->addChild(seed);
		exampleBox->setReadOnly(true);
		exampleBox->setColor(SETTING(TEXT_COLOR), SETTING(BACKGROUND_COLOR));
	}
	
	auto cur = grid->addChild(Grid::Seed(1, 2));
	cur->row(0).mode = GridInfo::FILL;
	cur->row(0).align = GridInfo::STRETCH;
	cur->column(0).mode = GridInfo::FILL;

	formatTbl = cur->addChild(WinUtil::Seeds::Dialog::table);
	formatTbl->onDblClicked([this] { handleDoubleClick(); });
	formatTbl->onKeyDown([this](int c) { return handleKeyDown(c); });
	formatTbl->onSelectionChanged([this] { handleSelectionChanged(); });

	{
		auto grid = cur->addChild(Grid::Seed(7, 1));
		grid->row(5).mode = GridInfo::FILL;
		grid->row(5).align = GridInfo::BOTTOM_RIGHT;

		Button::Seed seed;

		seed.caption = T_("&Add");
		addBtn = grid->addChild(seed);
		addBtn->onClicked([this] { handleAddClicked(); });

		seed.caption = T_("Move &Up");
		upBtn = grid->addChild(seed);
		upBtn->onClicked([this] { handleMoveUpClicked(); });

		seed.caption = T_("Move &Down");
		downBtn = grid->addChild(seed);
		downBtn->onClicked([this] { handleMoveDownClicked(); });

		seed.caption = T_("&Edit");
		editBtn = grid->addChild(seed);
		editBtn->onClicked([this] { handleEditClicked(); });

		seed.caption = T_("&Remove");
		removeBtn = grid->addChild(seed);
		removeBtn->onClicked([this] { handleRemoveClicked(); });

		::SetWindowLongPtr(WinUtil::addDlgButtons(grid, [this] { handleOKClicked(); }, [this] { endDialog(IDCANCEL); }) .first->handle(), GWLP_ID, 0); // the def button is the "Add" button
	}

	formatTbl->addColumn(T_("Patterns"));
	formatTbl->addColumn(T_("Custom font"));
	for(const auto& fii: format) {
		TStringList tsl;
		tsl.push_back(Text::toT(fii.getPattern()));
		tsl.push_back(Text::toT((fii.textStyle & Bdcpp::STYLE_AUTOFONT) ? "" : getFontName(fii.font)));
		formatTbl->insert(tsl);
	}
	formatTbl->ensureVisible(0);

	setText(T_("Formatting of ") + name);
	handleSelectionChanged();
	updateExample();
	addBtn->setFocus();
	layout();
	centerWindow();

	return false;
}

void BdcppTextFormatDialog::layout() {
	dwt::Point sz = getClientSize();
	grid->resize(dwt::Rectangle(3, 3, sz.x - 6, sz.y - 6));
	formatTbl->setColumnWidth(0, (formatTbl->getWindowSize().x - 30) * 4 / 5);
	formatTbl->setColumnWidth(1, (formatTbl->getWindowSize().x - 30) * 1 / 5);
}

int BdcppTextFormatDialog::run() {
	create(Seed(dwt::Point(550, 400), DS_CENTERMOUSE));
	return show();
}

void BdcppTextFormatDialog::insert(const TextFormatInfo& aFormatInfo, int index /* = -1*/) {
	TStringList tsl;
	tsl.push_back(Text::toT(aFormatInfo.getPattern()));
	tsl.push_back(Text::toT((aFormatInfo.textStyle & Bdcpp::STYLE_AUTOFONT) ? "" : getFontName(aFormatInfo.font)));
	formatTbl->insert(tsl, 0, index);
	if(index == -1)	{ format.push_back(aFormatInfo); index = int(format.size()-1);	}
	else			{ format.insert(format.begin() + index, aFormatInfo);			}
	formatTbl->ensureVisible(index);
}

void BdcppTextFormatDialog::erase(int index)	{
	formatTbl->erase(index);
	format.erase(format.begin() + index);
}

void BdcppTextFormatDialog::updateExample() {
	exampleBox->setText(_T(""));
	BDCRichText::addExamples(exampleBox, format, type);
}

string BdcppTextFormatDialog::getFontName(const string& aFont) {
	string::size_type i = Bdcpp::strfnd(aFont, ",");
	return (i == string::npos) ? aFont : aFont.substr(0, i);
}

void BdcppTextFormatDialog::handleDoubleClick() {
	if(formatTbl->hasSelected())
		handleEditClicked();
}

bool BdcppTextFormatDialog::handleKeyDown(int c) {
	switch(c) {
	case VK_INSERT: handleAddClicked(); return true;
	case VK_DELETE: handleRemoveClicked(); return true;
	}
	return false;
}

void BdcppTextFormatDialog::handleSelectionChanged() {
	bool enable = formatTbl->hasSelected();
	upBtn->setEnabled(enable);
	downBtn->setEnabled(enable);
	editBtn->setEnabled(enable);
	removeBtn->setEnabled(enable);
}

void BdcppTextFormatDialog::handleAddClicked() {
	TextFormatInfo fi(Util::emptyString, Style("", 0, 0xFFFFFF, Bdcpp::STYLE_AUTOCOLOR | Bdcpp::STYLE_AUTOBACKCOLOR)); BdcppTextFormatInfoDialog fid(this, fi);
	if(fid.run() == IDOK) {
		fi = fid.getFormatInfo();
		insert(fi);
		updateExample();
	}
}

void BdcppTextFormatDialog::handleMoveUpClicked() {
	HoldRedraw hold(formatTbl);
	std::vector<unsigned> selected = formatTbl->getSelection();
	for(const auto& i: selected) {
		if(i > 0) {
			TextFormatInfo fi = *(format.begin() + i);
			erase(i);
			insert(fi, i - 1);
			formatTbl->select(i - 1);
			updateExample();
		}
	}
}

void BdcppTextFormatDialog::handleMoveDownClicked() {
	HoldRedraw hold(formatTbl);
	auto selected = formatTbl->getSelection();
	for(auto i = selected.rbegin(); i != selected.rend(); ++i) {
		if(*i < (formatTbl->size() - 1)) {
			TextFormatInfo fi = *(format.begin() + *i);
			erase(*i);
			insert(fi, *i + 1);
			formatTbl->select(*i + 1);
			updateExample();
		}
	}
}

void BdcppTextFormatDialog::handleEditClicked() {
	int i = -1;
	while((i = formatTbl->getNext(i, LVNI_SELECTED)) != -1) {
		auto fii = format.begin() + i;
		BdcppTextFormatInfoDialog fid(this, *fii);
		if(fid.run() == IDOK) {
			*fii = fid.getFormatInfo();
			formatTbl->setText(i, 0, Text::toT(fii->getPattern()));
			formatTbl->setText(i, 1, Text::toT((fii->textStyle & Bdcpp::STYLE_AUTOFONT) ? "" : getFontName(fii->font)));
			updateExample();
		}
	}
}

void BdcppTextFormatDialog::handleRemoveClicked() {
	int i;
	while((i = formatTbl->getNext(-1, LVNI_SELECTED)) != -1) {
		erase(i);
		updateExample();
	}
}

void BdcppTextFormatDialog::handleOKClicked() {
	endDialog(IDOK);
}

/* BdcppTextFormatInfoDialog */
PropPage::ListItem BdcppTextFormatInfoDialog::stylesItems[] = {
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

BdcppTextFormatInfoDialog::BdcppTextFormatInfoDialog(dwt::Widget* parent, const TextFormatInfo& _formatInfo) :
dwt::ModalDialog(parent), grid(0), stylesTbl(0), sdexBox(0), resultBox(0), formatWholeChk(0), patternBtn(0), fontBtn(0), textColorBtn(0), bgColorBtn(0), formatInfo(_formatInfo) {
	onInitDialog([=] { return handleInitDialog(); });
}

bool BdcppTextFormatInfoDialog::handleInitDialog() {

	grid = addChild(Grid::Seed(3, 2));
	grid->column(1).mode = GridInfo::FILL;
	grid->column(1).align = GridInfo::STRETCH;
	grid->row(2).mode = GridInfo::FILL;
	grid->row(2).align = GridInfo::STRETCH;

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

	grid->addChild(Label::Seed(T_("Errors")));
	{
		auto seed = WinUtil::Seeds::Dialog::textBox;
		seed.lines = 2;
		seed.style &= ~ES_AUTOHSCROLL;
		seed.style |= ES_MULTILINE | WS_VSCROLL;
		resultBox = grid->addChild(seed);
		resultBox->setReadOnly(true);
		if(!formatInfo->supportsResult())
			resultBox->setVisible(false);
	}

	auto cur = grid->addChild(Grid::Seed(1, 2));
	grid->setWidget(cur, 2, 0, 1, 2);
	cur->row(0).mode = GridInfo::FILL;
	cur->row(0).align = GridInfo::STRETCH;
	cur->column(0).mode = GridInfo::FILL;

	stylesTbl = cur->addChild(WinUtil::Seeds::Dialog::optionsTable);
	stylesTbl->addColumn();

	{
		auto grid = cur->addChild(Grid::Seed(6, 1));
		grid->row(4).mode = GridInfo::FILL;
		grid->row(4).align = GridInfo::BOTTOM_RIGHT;

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

		formatWholeChk = grid->addChild(CheckBox::Seed(T_("Format the whole line")));

		::SetWindowLongPtr(WinUtil::addDlgButtons(grid, [this] { handleOKClicked(); }, [this] { endDialog(IDCANCEL); }) .first->handle(), GWLP_ID, 0); // the def button is the "Add" button
	}

	sdexBox->setText(Text::toT(formatInfo.getPattern()));
	formatWholeChk->setChecked(formatInfo.formatWhole);

	for(const auto& si: stylesItems)
		stylesTbl->setChecked(stylesTbl->insert(TStringList(1, T_(si.desc)), si.setting), si.setting & formatInfo.textStyle);
	stylesTbl->setColumnWidth(0, LVSCW_AUTOSIZE);

	setText(T_("Format"));
	handleInputUpdated();
	layout();
	centerWindow();

	return false;
}

int BdcppTextFormatInfoDialog::run() {
	create(Seed(dwt::Point(475, 475), DS_CENTERMOUSE));
	return show();
}

void BdcppTextFormatInfoDialog::layout() {
	dwt::Point sz = getClientSize();
	grid->resize(dwt::Rectangle(3, 3, sz.x - 6, sz.y - 6));
}

void BdcppTextFormatInfoDialog::handleOKClicked() {
	(SdEx&)(formatInfo) = SdEx(Text::fromT(sdexBox->getText()));
	formatInfo.formatWhole = formatWholeChk->getChecked();
	formatInfo.textStyle = 0;
	int i = -1;
	while((i = stylesTbl->getNext(i, LVNI_ALL)) != -1)
		formatInfo.textStyle |= stylesTbl->isChecked(i) ? int(stylesTbl->getData(i)) : 0;
	endDialog(IDOK);
}

void BdcppTextFormatInfoDialog::handleInputUpdated() {
	SdEx sdex(Text::fromT(sdexBox->getText()));
	if(sdex->supportsResult())
		resultBox->setText(Text::toT(Text::toDOS(sdex->get_result())));
}

void BdcppTextFormatInfoDialog::handlePatternClicked() {
	BdcppSdExDialog dlg(this, T_("Text format pattern"), Text::fromT(sdexBox->getText()), SdEx::getParamsTextFormat());
	if(dlg.run() == IDOK)
		sdexBox->setText(Text::toT(dlg.getSdEx().getPattern()));
}

void BdcppTextFormatInfoDialog::handleFontClicked() {
	LOGFONT lf;
	WinUtil::decodeFont(Text::toT(formatInfo.font), lf);
	FontDialog::Options options;
	options.strikeout = false;
	options.underline = false;
	options.color = false;
	options.bgColor = formatInfo.bgColor;
	COLORREF cr = COLORREF(formatInfo.textColor);
	if(FontDialog(this).open(lf, cr, &options)) {
		formatInfo.font = Text::fromT(WinUtil::encodeFont(lf));
		formatInfo.textColor = int(cr);
	}
}

void BdcppTextFormatInfoDialog::handleTextColorClicked() {
	dwt::ColorDialog::ColorParams colorParams(formatInfo.textColor);
	if(dwt::ColorDialog(this).open(colorParams))
		formatInfo.textColor = colorParams.getColor();
}

void BdcppTextFormatInfoDialog::handleBgColorClicked() {
	dwt::ColorDialog::ColorParams colorParams(formatInfo.bgColor);
	if(dwt::ColorDialog(this).open(colorParams))
		formatInfo.bgColor = colorParams.getColor();
}
