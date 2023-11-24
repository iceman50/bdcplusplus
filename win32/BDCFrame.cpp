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

#include "BDCFrame.h"

#include <dcpp/DirectoryListing.h>
#include <dcpp/File.h>
#include <dcpp/LogManager.h>
#include <dcpp/SettingsManager.h>
#include <dcpp/ShareManager.h>

#include <dwt/widgets/Grid.h>

#include "BDCWinUtil.h"
#include "DirectoryListingFrame.h"
#include "HoldRedraw.h"
#include "ShellMenu.h"
#include "TypedTable.h"
#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;

const string BDCFrame::id = "BDCLog";
const string& BDCFrame::getId() const { return id; }

dwt::ImageListPtr BDCFrame::logIcons;

static const ColumnInfo logColumns[] = {
	{ N_("Time"), 75, false },
	{ N_("ID#"), 65, false },
	{ N_("Type"), 75, false },
	{ N_("Level"), 75, false },
	{ N_("Message"), 750, false }
};

BDCFrame::BDCFrame(TabViewPtr parent) :
	BaseType(parent, T_("BDC Log"), IDI_DCPP, false),
	grid(0),
	logTable(0)
{
	grid = addChild(Grid::Seed(1, 1));
	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;

	if(!logIcons) {
		const dwt::Point size(16, 16);
		logIcons = dwt::ImageListPtr(new dwt::ImageList(size));

		auto addIcon = [=] (const int& icon, bool loadFile) { 
			loadFile ? logIcons->add(dwt::Icon(WinUtil::iconFilename(icon), size))
					 : logIcons->add(dwt::Icon(icon, size)); };

		if(SETTING(USE_THEME)) {
			try {
				//Types
				addIcon(IDI_WHATS_THIS, true);
				addIcon(IDI_DCPP, true);
				addIcon(IDI_DCPP_WARNING, true);
				addIcon(IDI_EXIT, true);
				//Levels
				addIcon(IDI_SETTINGS, true);
				addIcon(IDI_INDEXING, true);
				addIcon(IDI_CHAT, true);
				addIcon(IDI_HELP, true);
				addIcon(IDI_PUBLICHUBS, true);
				addIcon(IDI_PLUGINS, true);
			} catch(const dwt::DWTException&) {
				addIcon(IDI_WHATS_THIS, false);
				addIcon(IDI_DCPP, false);
				addIcon(IDI_DCPP_WARNING, false);
				addIcon(IDI_EXIT, false);

				addIcon(IDI_SETTINGS, false);
				addIcon(IDI_INDEXING, false);
				addIcon(IDI_CHAT, false);
				addIcon(IDI_HELP, false);
				addIcon(IDI_PUBLICHUBS, false);
				addIcon(IDI_PLUGINS, false);
			}
		} else {
			addIcon(IDI_WHATS_THIS, false);
			addIcon(IDI_DCPP, false);
			addIcon(IDI_DCPP_WARNING, false);
			addIcon(IDI_EXIT, false);

			addIcon(IDI_SETTINGS, false);
			addIcon(IDI_INDEXING, false);
			addIcon(IDI_CHAT, false);
			addIcon(IDI_HELP, false);
			addIcon(IDI_PUBLICHUBS, false);
			addIcon(IDI_PLUGINS, false);
		}
	}

	WidgetLogs::Seed cs(WinUtil::Seeds::table);
	cs.lvStyle |= LVS_EX_SUBITEMIMAGES;
	logTable = grid->addChild(cs);
	logTable->setSmallImageList(logIcons);
	logTable->onContextMenu([this](dwt::ScreenCoordinate pt) { return handleContextMenu(pt); });
	logTable->onLeftMouseDown([this](const dwt::MouseEvent& me) { return handleClick(me); });

	WinUtil::makeColumns(logTable, logColumns, COLUMN_LAST, SETTING(BDCFRAME_ORDER), SETTING(BDCFRAME_WIDTHS));
	WinUtil::setColor(logTable);

	initStatus();

	status->onDblClicked(STATUS_STATUS, [] {
		WinUtil::openFile(Text::toT(Util::validateFileName(LogManager::getInstance()->getPath(LogManager::SYSTEM))));
						 });

	layout();

	auto oldMessages = LogManager::getInstance()->getLastLogs();
	// Technically, we might miss a message or two here, but who cares...
	LogManager::getInstance()->addListener(this);

	for(const auto& i: oldMessages) {
		addLog(i);
	}
}

BDCFrame::~BDCFrame() {

}

bool BDCFrame::handleContextMenu(dwt::ScreenCoordinate pt) {
	auto sel = logTable->countSelected();
	if (sel > 0) {
		if (pt.x() == -1 && pt.y() == -1) {
			pt = logTable->getContextMenuPos();
		}

		auto menu = addChild(WinUtil::Seeds::menu);
		menu->setTitle((sel == 1) ? escapeMenu(T_("Copy log message")) : str(TF_("Copy %1% log messages") % sel));
		WinUtil::addCopyMenu(menu.get(), logTable);

		menu->open(pt);

		return true;
	}

	return false;
}

bool BDCFrame::handleClick(const dwt::MouseEvent& me) {
	auto item = logTable->hitTest(me.pos);
	if (item.first == -1 || item.second == -1) {
		return false;
	}

	auto ui = logTable->getData(item.first);
	switch (item.second) {
	case COLUMN_MESSAGE: {
		auto first = ui->message.find('<');
		auto last = ui->message.find('>');
		const string& link = ui->message.substr(first+1, last - (first+1));
		if (File::getSize(link) != -1) {
			openFile(link);
			return true;
		}
		break;
	}

	default:
		return false;
	}

	return true;
}

void BDCFrame::addLog(const LogMessagePtr& logMessage) {
	logTable->insert(new LogInfo(logMessage));
	setDirty(SettingsManager::BOLD_SYSTEM_LOG);
}

void BDCFrame::openFile(const string& path) const {
	// see if we are opening our own file list.
	if(path == ShareManager::getInstance()->getBZXmlFile()) {
		DirectoryListingFrame::openOwnList(getParent());
		return;
	}

	// see if we are opening a file list.
	auto u = DirectoryListing::getUserFromFilename(path);
	if(u) {
		DirectoryListingFrame::openWindow(getParent(), Text::toT(path), Util::emptyStringT,
										  HintedUser(u, Util::emptyString), 0, DirectoryListingFrame::FORCE_ACTIVE);
		return;
	}

	WinUtil::openFile(Text::toT(path));
}

void BDCFrame::layout() {
	dwt::Rectangle r(this->getClientSize());

	r.size.y -= status->refresh();

	grid->resize(r);
}

bool BDCFrame::preClosing() {
	LogManager::getInstance()->removeListener(this);
	return true;
}

void BDCFrame::postClosing() {
	SettingsManager::getInstance()->set(SettingsManager::BDCFRAME_WIDTHS, WinUtil::toString(logTable->getColumnWidths()));
	SettingsManager::getInstance()->set(SettingsManager::BDCFRAME_ORDER, WinUtil::toString(logTable->getColumnOrder()));
}

BDCFrame::LogInfo::LogInfo(const LogMessagePtr& logMessage) :
message(logMessage->getText()),
type(static_cast<uint8_t>(logMessage->getMessageType())),
level(static_cast<uint8_t>(logMessage->getLogLevel()))
{
	columns[COLUMN_TIME] = Text::toT(Util::getTimeString(logMessage->getTime()));
	columns[COLUMN_ID] = Text::toT(Util::toString(logMessage->getId()));
	columns[COLUMN_TYPE] = BDCWinUtil::logType[type];
	columns[COLUMN_LEVEL] = BDCWinUtil::logLevel[level];
	columns[COLUMN_MESSAGE] = Text::toT(message);
}

void BDCFrame::on(Message, const LogMessagePtr& logMsg) noexcept {
	callAsync([=] { addLog(logMsg); });
}