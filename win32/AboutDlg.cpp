/*
 * Copyright (C) 2001-2023 Jacek Sieka, arnetheduck on gmail point com
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

#include "AboutDlg.h"

#include <dcpp/format.h>
#include <dcpp/HttpManager.h>
#include <dcpp/SettingsManager.h>
#include <dcpp/SimpleXML.h>
#include <dcpp/Streams.h>
#include <dcpp/version.h>

#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/Link.h>

#include "BDCWinUtil.h"

#include "resource.h"
#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;
using dwt::Label;
using dwt::Link;

static tstring info;

AboutDlg::AboutDlg(dwt::Widget* parent) :
dwt::ModalDialog(parent),
grid(0),
version(0),
c(nullptr)
{
	info.clear();
	BDCWinUtil::getSysInfo(info);
	BDCWinUtil::getNetStats(info);
	onInitDialog([this] { return handleInitDialog(); });
}

AboutDlg::~AboutDlg() {
}

int AboutDlg::run() {
	create(dwt::Point(400, 600));
	return show();
}

bool AboutDlg::handleInitDialog() {
	grid = addChild(Grid::Seed(6, 1));
	grid->column(0).mode = GridInfo::FILL;
	grid->row(1).mode = GridInfo::FILL;
	grid->row(1).align = GridInfo::STRETCH;

	// horizontally centered seeds
	GroupBox::Seed gs;
	gs.style |= BS_CENTER;
	gs.padding.y = 2;
	Label::Seed ls;
	ls.style |= SS_CENTER;

	{
		auto cur = grid->addChild(gs)->addChild(Grid::Seed(3, 1));
		cur->column(0).mode = GridInfo::FILL;
		cur->column(0).align = GridInfo::CENTER;

		cur->addChild(Label::Seed(WinUtil::createIcon(IDI_DCPP, 48)));

		ls.caption = Text::toT(dcpp::fullVersionString) + _T("\n(c) Copyright 2022-2023 iceman50\r\nThanks to the DC++ Team!");
		cur->addChild(ls);

		cur->addChild(Link::Seed(_T("https://github.com/iceman50/bdcplusplus"), true));

		auto ts = WinUtil::Seeds::Dialog::textBox;
		ts.style |= ES_READONLY;
		ts.exStyle &= ~WS_EX_CLIENTEDGE;

		gs.caption = T_("TTH");
		ts.caption = WinUtil::tth;
		cur->addChild(gs)->addChild(ts);
	}

	{
		gs.caption = T_("Client info");
		auto seed = WinUtil::Seeds::Dialog::textBox;
		seed.style &= ~ES_AUTOHSCROLL;
		seed.style |= ES_MULTILINE | WS_VSCROLL | ES_READONLY;
		seed.caption = info;
		grid->addChild(gs)->addChild(seed);
	}

	gs.caption = T_("Latest stable DC++ version");
	ls.caption = T_("Downloading...");
	version = grid->addChild(gs)->addChild(ls);

	auto buttons = WinUtil::addDlgButtons(grid,
		[this] { endDialog(IDOK); },
		[this] { endDialog(IDCANCEL); });
	buttons.first->setFocus();
	buttons.second->setVisible(false);

	setText(T_("About BDC++"));
	setSmallIcon(WinUtil::createIcon(IDI_DCPP, 16));
	setLargeIcon(WinUtil::createIcon(IDI_DCPP, 32));

	layout();
	centerWindow();

	HttpManager::getInstance()->addListener(this);
	onDestroy([this] { HttpManager::getInstance()->removeListener(this); });
	c = HttpManager::getInstance()->download("https://dcplusplus.sourceforge.io/version.xml");

	return false;
}

void AboutDlg::layout() {
	dwt::Point sz = getClientSize();
	grid->resize(dwt::Rectangle(3, 3, sz.x - 6, sz.y - 6));
}

void AboutDlg::completeDownload(bool success, const string& result) {
	tstring str;

	if(success && !result.empty()) {
		try {
			SimpleXML xml;
			xml.fromXML(result);
			if(xml.findChild("DCUpdate")) {
				xml.stepIn();
				if(xml.findChild("Version")) {
					const auto& ver = xml.getChildData();
					if(!ver.empty()) {
						str = Text::toT(ver);
					}
				}
			}
		} catch(const SimpleXMLException&) {
			str = T_("Error processing version information");
		}
	}

	version->setText(str.empty() ? Text::toT(result) : str);
}

void AboutDlg::on(HttpManagerListener::Failed, HttpConnection* c, const string& str) noexcept {
	if(c != this->c) { return; }
	c = nullptr;

	callAsync([str, this] { completeDownload(false, str); });
}

void AboutDlg::on(HttpManagerListener::Complete, HttpConnection* c, OutputStream* stream) noexcept {
	if(c != this->c) { return; }
	c = nullptr;

	auto str = static_cast<StringOutputStream*>(stream)->getString();
	callAsync([str, this] { completeDownload(true, str); });
}
