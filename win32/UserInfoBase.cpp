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

#include "UserInfoBase.h"

#include <dcpp/ClientManager.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/LogManager.h>
#include <dcpp/QueueManager.h>
#include <dcpp/UploadManager.h>
#include <dcpp/User.h>
#include <dcpp/UserMatchManager.h>

#include <dwt/util/StringUtils.h>

#include "PrivateFrame.h"
#include "HubFrame.h"
#include "DirectoryListingFrame.h"

#include "InfoFrame.h"

void UserInfoBase::matchQueue() {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError(), LogMessage::TYPE_ERROR, LogMessage::LOG_SHARE);
	}
}
void UserInfoBase::getList(TabViewPtr parent) {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} catch(const QueueSelfException& e) {
		getOwnList(parent);
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError(), LogMessage::TYPE_ERROR, LogMessage::LOG_SHARE);
	}
}
void UserInfoBase::browseList(TabViewPtr parent) {
	if(!user.user->getCID())
		return;
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_PARTIAL_LIST);
	} catch(const QueueSelfException& e) {
		getOwnList(parent);
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError(), LogMessage::TYPE_ERROR, LogMessage::LOG_SHARE);
	}
}
void UserInfoBase::getOwnList(TabViewPtr parent) {
	try {
		DirectoryListingFrame::openOwnList(parent);
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError(), LogMessage::TYPE_ERROR, LogMessage::LOG_SHARE);
	}
}
void UserInfoBase::addFav() {
	FavoriteManager::getInstance()->addFavoriteUser(user);
}

void UserInfoBase::pm(TabViewPtr parent) {
	PrivateFrame::openWindow(parent, user, Util::emptyStringT);
}

void UserInfoBase::grant() {
	UploadManager::getInstance()->reserveSlot(user);
}

void UserInfoBase::removeFromQueue() {
	QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
}

void UserInfoBase::connectFav(TabViewPtr parent) {
	HubFrame::openWindow(parent, user.hint.empty() ? FavoriteManager::getInstance()->getUserURL(user) : user.hint);
}

void UserInfoBase::ignoreChat(bool ignore) {
	UserMatchManager::getInstance()->ignoreChat(user, ignore);
}
//DiCe Edit Possibly just piggyback getInfo?
void UserInfoBase::showInfo(TabViewPtr parent) {
	auto nick = Text::fromT(WinUtil::getNick(user));
	InfoFrame::InfoMap userInfo;
	string hub;
	
	{
		auto lock = ClientManager::getInstance()->lock();
		auto ou = ClientManager::getInstance()->findOnlineUser(user);
		if (!ou)
			return;

		const Identity& id = ou->getIdentity();
		userInfo = id.getInfo();
		hub = ou->getClient().getHubUrl();
	}

	InfoFrame::openWindow(parent, nick + " - " + hub, userInfo);
}

tstring UserInfoBase::getInfo(int flags) const {
	static const size_t maxChars = 100; // max chars per tooltip line

	tstring ret(WinUtil::getNicks(user));
	dwt::util::cutStr(ret, maxChars);

	auto addLine = [&ret](tstring line) {
		dwt::util::cutStr(line, maxChars);
		ret += _T("\r\n") + line;
	};

	addLine(str(TF_("Hubs: %1%") % WinUtil::getHubNames(user).first));

	auto lock = ClientManager::getInstance()->lock();
	auto ou = ClientManager::getInstance()->findOnlineUser(user);
	if(!ou)
		return ret;
	const Identity& id = ou->getIdentity();

	auto addValue = [&addLine](const tstring& descr, const string& value) {
		if(!value.empty())
			addLine(str(TF_("%1%: %2%") % descr % Text::toT(value)));
	};

	StringList status;
	if(status.empty()) {
		if(id.getStatus() & Identity::NORMAL) { status.push_back(_("Normal")); }
		if(id.getStatus() & Identity::AWAY) { status.push_back(_("Away")); }
		if(id.getStatus() & Identity::SERVER) { status.push_back(_("Server")); }
		if(id.getStatus() & Identity::FIREBALL) { status.push_back(_("Fireball")); }
		if(id.getStatus() & Identity::TLS) { status.push_back(_("TLS")); }
		//Technically status should never be empty BUT we should add a safeguard
		if(status.empty()) { status.push_back(_("None")); }
		
		addLine(str(TF_("Status: %1%") % Text::toT(Util::toString(status))));
	} else {
		status.clear();
		addLine(T_("None"));
	}

	if(id.isHidden())
		addLine(T_("Hidden user"));
	if(id.isBot())
		addLine(T_("Bot"));
	if(id.isOp())
		addLine(T_("Hub operator"));
	if(id.isAway())
		addLine(T_("In away mode"));

	addValue(T_("Shared"), Util::formatBytes(id.getBytesShared()));
	addValue(T_("Description"), id.getDescription());
	addValue(T_("Tag"), id.getTag());
	addValue(T_("Connection"), id.getConnection());
	addValue(T_("IP"), id.getIp());
	addValue(T_("Country"), id.getCountry());
	addValue(T_("E-mail"), id.getEmail());
	string slots = id.get("SL");
	if(!slots.empty()) {
		tstring value = Text::toT(slots);
		string fs = id.get("FS");
		if(!fs.empty())
			value = str(TF_("%1%/%2%") % Text::toT(fs) % value);
		addLine(str(TF_("%1%: %2%") % T_("Slots") % value));
	}
	if((flags & INFO_WITH_CID) == INFO_WITH_CID) {
		addValue(T_("CID"), user.user->getCID().toBase32());
	}

	return ret;
}

tstring UserInfoBase::getTooltip() const {
	return getInfo();
}

UserTraits::UserTraits() :
Flags(adcOnly | favOnly | nonFavOnly | chatIgnoredOnly | chatNotIgnoredOnly)
{
}

void UserTraits::parse(const UserInfoBase* ui) {
	if(ui->getUser().user->isSet(User::NMDC)) {
		unsetFlag(adcOnly);
	}

	bool fav = FavoriteManager::getInstance()->isFavoriteUser(ui->getUser());
	if(fav) {
		unsetFlag(nonFavOnly);
	} else {
		unsetFlag(favOnly);
	}

	auto lock = ClientManager::getInstance()->lock();
	auto ou = ClientManager::getInstance()->findOnlineUser(ui->getUser());
	if(ou) {
		if(ou->getIdentity().noChat()) {
			unsetFlag(chatNotIgnoredOnly);
		} else {
			unsetFlag(chatIgnoredOnly);
		}
	} else {
		// offline user: show both ignore & un-ignore commands
		unsetFlag(chatNotIgnoredOnly);
		unsetFlag(chatIgnoredOnly);
	}
}
