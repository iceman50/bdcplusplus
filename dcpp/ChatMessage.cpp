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
#include "ChatMessage.h"

#include "BDCText.h"
#include "Client.h"
#include "format.h"
#include "Magnet.h"
#include "OnlineUser.h"
#include "PluginManager.h"
#include "SettingsManager.h"
#include "SimpleXML.h"
#include "Tagger.h"
#include "Util.h"

namespace dcpp {

ChatMessage::ChatMessage(const string& text, OnlineUser* from,
	const OnlineUser* to, const OnlineUser* replyTo,
	bool thirdPerson, time_t messageTimestamp) :
from(from->getUser()),
to(to ? to->getUser() : nullptr),
replyTo(replyTo ? replyTo->getUser() : nullptr),
timestamp(time(0)),
thirdPerson(thirdPerson),
messageTimestamp(messageTimestamp)
{
	BDCText bt(text, from, to || replyTo, thirdPerson, messageTimestamp);
	message = bt.getPlainText();
	htmlMessage = bt.getHtmlText();
	PluginManager::getInstance()->onChatDisplay(htmlMessage, from);
}

} // namespace dcpp
