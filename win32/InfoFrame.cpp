/*
 * Copyright (C) 2022 iceman50
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

#include "InfoFrame.h"

#include <dcpp/format.h>
#include <dcpp/Text.h>

#include <dwt/widgets/FontDialog.h>
#include <dwt/widgets/Grid.h>

#include "MainWindow.h"
#include "WinUtil.h"

using dwt::FontDialog;
using dwt::Grid;
using dwt::GridInfo;

const string InfoFrame::id = "Info";
const string& InfoFrame::getId() const { return id; }

struct FieldName {
	string field;
	tstring name;
	tstring(*convert)(const string& val);
};

static tstring formatBytes(const string& val) {
	return Text::toT(Util::formatBytes(val));
}

static tstring formatSpeed(const string& val) {
	return Text::toT(str(F_("%1%/s") % Util::formatBytes(val)));
}

static const FieldName fields[] =
{
	{ "NI", T_("Nick"), &Text::toT },
	{ "AW", T_("Away"), &Text::toT },
	{ "DE", T_("Description"), &Text::toT },
	{ "EM", T_("E-Mail"), &Text::toT },
	{ "SS", T_("Shared bytes"), &formatBytes },
	{ "SF", T_("Shared files"), &Text::toT },
	{ "US", T_("Upload speed"), &formatSpeed },
	{ "DS", T_("Download speed"), &formatSpeed },
	{ "SL", T_("Total slots"), &Text::toT },
	{ "FS", T_("Free slots"), &Text::toT },
	{ "HN", T_("Hubs (normal)"), &Text::toT },
	{ "HR", T_("Hubs (registered)"), &Text::toT },
	{ "HO", T_("Hubs (op)"), &Text::toT },
	{ "I4", T_("IP (v4)"), &Text::toT },
	{ "I6", T_("IP (v6)"), &Text::toT },
	{ "U4", T_("Search port (v4)"), &Text::toT },
	{ "U6", T_("Search port (v6)"), &Text::toT },
	{ "SU", T_("Features"), &Text::toT },
	{ "VE", T_("Application version"), &Text::toT },
	{ "AP", T_("Application"), &Text::toT },
	{ "ID", T_("CID"), &Text::toT },
	{ "KP", T_("TLS Keyprint"), &Text::toT },
	{ "CO", T_("Connection"), &Text::toT },
	{ "CT", T_("Client type"), &Text::toT },
	{ "TA", T_("Tag"), &Text::toT },
	//DiCe Addon
	{ "SP", T_("Supports"), &Text::toT },
	{ "LK", T_("Lock"), &Text::toT },
	{ "PK", T_("PK"), &Text::toT },
	{ "KY", T_("Key"), &Text::toT },

	{ "", _T(""), 0 }
};

static tstring displayInfo(const InfoFrame::InfoMap& userInfo) {
	auto info = move(userInfo);
	auto ret = tstring();

	for(auto f = fields; !f->field.empty(); ++f) {
		auto i = info.find(f->field);
		if(i != info.end()) {
			ret += f->name + _T(" ") + f->convert(i->second) + _T("\r\n");
			info.erase(i);
		}
	}
	if(ret.empty())
		return _T("CRITICAL ERROR: USER INFORMATION NOT AVAILABLE");

	return ret;
}

void InfoFrame::openWindow(TabViewPtr parent, const string& userName, const InfoFrame::InfoMap& userInfo, bool activate, bool temporary) {
	auto window = new InfoFrame(parent, userName, userInfo, temporary);
	if(activate) //TODO WindowParams?
		window->activate();
}

InfoFrame::InfoFrame(TabViewPtr parent, const string& userName, const InfoFrame::InfoMap& userInfo, bool temporary) :
BaseType(parent, Text::toT(userName), 0),//IDH_INFO_VIEWER
grid(0),
pad(0),
userName(userName),
userInfo(userInfo),
temporary(temporary)
{
	setIcon(WinUtil::userImages->getIcon(0));

	grid = addChild(Grid::Seed(2, 1));
	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;

	/*TODO Let's make this a customdrawn table */
	{
		TextBox::Seed cs = WinUtil::Seeds::textBox;
		cs.style |= WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY;
		if(!SettingsManager::getInstance()->isDefault(SettingsManager::INFO_VIEWER_FONT)) {
			// the user has configured a different font for text files; use it.
			LOGFONT lf;
			WinUtil::decodeFont(Text::toT(SETTING(INFO_VIEWER_FONT)), lf);
			cs.font = dwt::FontPtr(new dwt::Font(lf));
		}
		pad = grid->addChild(cs);
		addWidget(pad);
		WinUtil::handleDblClicks(pad);
	}

	{
		Button::Seed seed = WinUtil::Seeds::button;
		seed.caption = T_("Change the text viewer font");
		auto changeFont = grid->addChild(Grid::Seed(1, 1))->addChild(seed);
		changeFont->setHelpId(0); // Generic I'm too lazy to do this properly
		changeFont->onClicked([this] { handleFontChange(); });
		addWidget(changeFont);
	}

	initStatus();
	status->setText(STATUS_STATUS, str(TF_("Viewing User: %1%") % Text::toT(userName)));

	pad->setTextLimit(0);

	auto text = displayInfo(userInfo);
	pad->setText(text);

	layout();
}

void InfoFrame::layout() {
	dwt::Rectangle r{ getClientSize() };

	r.size.y -= status->refresh();

	r.size.y -= grid->getSpacing(); // add a bottom margin not to be too stuck to the status bar.
	grid->resize(r);
}

void InfoFrame::handleFontChange() {
	LOGFONT logFont;
	WinUtil::decodeFont(Text::toT(
		SettingsManager::getInstance()->isDefault(SettingsManager::INFO_VIEWER_FONT) ?
		SETTING(MAIN_FONT) : SETTING(INFO_VIEWER_FONT)), logFont);
	DWORD color = 0;
	FontDialog::Options options;
	options.strikeout = false;
	options.underline = false;
	options.color = false;
	if(FontDialog(this).open(logFont, color, &options)) {
		SettingsManager::getInstance()->set(SettingsManager::INFO_VIEWER_FONT, Text::fromT(WinUtil::encodeFont(logFont)));
		pad->setFont(new dwt::Font(logFont));
	}
}
