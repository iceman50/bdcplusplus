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

#include "stdinc.h"
#include "SearchResult.h"

#include "UploadManager.h"
#include "Text.h"
#include "User.h"
#include "ClientManager.h"
#include "Client.h"

namespace dcpp {

SearchResult::SearchResult(const HintedUser& aUser, Types aType, int aSlots, int aFreeSlots,
	int64_t aSize, const string& aFile, const string& aHubName,
	const string& ip, TTHValue aTTH, const string& aToken, const Style& aStyle) :
file(aFile), hubName(aHubName), user(aUser),
	size(aSize), type(aType), slots(aSlots), freeSlots(aFreeSlots), IP(ip),
	tth(aTTH), token(aToken), style(aStyle) { }

SearchResult::SearchResult(Types aType, int64_t aSize, const string& aFile, const TTHValue& aTTH) :
	file(aFile), user(ClientManager::getInstance()->getMe(), Util::emptyString), size(aSize),
	type(aType), slots(SETTING(SLOTS)), freeSlots(UploadManager::getInstance()->getFreeSlots()),
	tth(aTTH) { }

string SearchResult::toSR(const Client& c) const {
	// File:		"$SR %s %s%c%s %d/%d%c%s (%s)|"
	// Directory:	"$SR %s %s %d/%d%c%s (%s)|"
	string tmp;
	tmp.reserve(128);
	tmp.append("$SR ", 4);
	tmp.append(Text::fromUtf8(c.getMyNick(), c.getEncoding()));
	tmp.append(1, ' ');
	string acpFile = Text::fromUtf8(file, c.getEncoding());
	if(type == TYPE_FILE) {
		tmp.append(acpFile);
		tmp.append(1, '\x05');
		tmp.append(Util::toString(size));
	} else {
		tmp.append(acpFile, 0, acpFile.length() - 1);
	}
	tmp.append(1, ' ');
	tmp.append(Util::toString(freeSlots));
	tmp.append(1, '/');
	tmp.append(Util::toString(slots));
	tmp.append(1, '\x05');
	tmp.append("TTH:" + getTTH().toBase32());
	tmp.append(" (", 2);
	tmp.append(c.getIpPort());
	tmp.append(")|", 2);
	return tmp;
}

AdcCommand SearchResult::toRES(char type) const {
	AdcCommand cmd(AdcCommand::CMD_RES, type);
	cmd.addParam("SI", Util::toString(size));
	cmd.addParam("SL", Util::toString(freeSlots));
	cmd.addParam("FN", Util::toAdcFile(file));
	cmd.addParam("TR", getTTH().toBase32());
	return cmd;
}

string SearchResult::getFileName() const {
	if(getType() == TYPE_FILE)
		return Util::getFileName(getFile());

	if(getFile().size() < 2)
		return getFile();

	string::size_type i = getFile().rfind('\\', getFile().length() - 2);
	if(i == string::npos)
		return getFile();

	return getFile().substr(i + 1);
}

string SearchResult::getSlotString() const {
	return Util::toString(getFreeSlots()) + '/' + Util::toString(getSlots());
}

}
