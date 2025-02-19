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

#include "NotepadFrame.h"

#include <dcpp/File.h>
#include <dcpp/Text.h>
#include <dcpp/Magnet.h>

using dwt::Grid;
using dwt::GridInfo;

const string NotepadFrame::id = "Notepad";
const string& NotepadFrame::getId() const { return id; }

NotepadFrame::NotepadFrame(TabViewPtr parent) :
	BaseType(parent, T_("Notepad"), IDI_NOTEPAD),
	pad(0)
{
	{
		TextBox::Seed cs = WinUtil::Seeds::textBox;
		cs.style |= WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL | ES_WANTRETURN;
		pad = addChild(cs);
		addWidget(pad, AUTO_FOCUS, false);
		WinUtil::handleDblClicks(pad);

		pad->onContextMenu([this](const dwt::ScreenCoordinate &sc) { return handleMenu(sc); });
	}

	initStatus();

	pad->setTextLimit(0);
	try {
		pad->setText(Text::toT(File(Util::getNotepadFile(), File::READ, File::OPEN).read()));
	} catch(const FileException& e) {
		status->setText(STATUS_STATUS, str(TF_("Error loading the notepad file: %1%") % Text::toT(e.getError())));
	}

	pad->setModify(false);
	SettingsManager::getInstance()->addListener(this);

	layout();
}

NotepadFrame::~NotepadFrame() {
}

bool NotepadFrame::preClosing() {
	SettingsManager::getInstance()->removeListener(this);
	save();
	return true;
}

void NotepadFrame::layout() {
	dwt::Rectangle r { getClientSize() };

	r.size.y -= status->refresh();

	pad->resize(r);
}

void NotepadFrame::save() {
	if(pad->getModify()) {
		try {
			dcdebug("Writing notepad contents\n");
			File(Util::getNotepadFile(), File::WRITE, File::CREATE | File::TRUNCATE).write(Text::fromT(pad->getText()));
		} catch(const FileException& e) {
			dcdebug("Writing failed: %s\n", e.getError().c_str());
			///@todo Notify user
		}
	}
}

void NotepadFrame::on(SettingsManagerListener::Save, SimpleXML&) noexcept {
	callAsync([this] { save(); });
}

bool NotepadFrame::handleMenu(dwt::ScreenCoordinate pt) {
	if(pt.x() == -1 && pt.y() == -1) {
		pt = pad->getContextMenuPos();
	}

	makeMenu(pt)->open(pt);

	return true;
}

MenuPtr NotepadFrame::makeMenu(dwt::ScreenCoordinate pt) {
	auto menu = pad->getMenu();

	string name, hash, key;
	tstring searchText;
	WinUtil::getChatSelText(pad, searchText, pt);

	if (!searchText.empty()) {
		if(searchText.size() > 8 && Magnet::parseUri(Text::fromT(searchText), hash, name, key) && (!name.empty() || !key.empty())) {
			searchText = name.empty() ? Text::toT(key) : Text::toT(name);
		}

		WinUtil::addSearchMenu(menu.get(), searchText, hash);
	}

	return menu;
}
