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

#ifndef DCPLUSPLUS_DCPP_BDC_LOG_MESSAGE_H
#define DCPLUSPLUS_DCPP_BDC_LOG_MESSAGE_H

#include "typedefs.h"

#include "TimerManager.h"

namespace dcpp {

class LogMessage {
public:
	enum Type : uint8_t { TYPE_DEBUG, TYPE_GENERAL, TYPE_WARNING, TYPE_ERROR, TYPE_LAST };
	enum Level : uint8_t { LOG_SYSTEM, LOG_SHARE, LOG_PRIVATE, LOG_SPAM, LOG_SERVER, LOG_PLUGIN, LOG_LAST };

	LogMessage(const string& msg, Type _type, Level _level) noexcept;

	uint64_t getId() const noexcept {
		return id;
	}

	const string& getText() const noexcept {
		return text;
	}

	time_t getTime() const noexcept {
		return time;
	}

	Type getMessageType() const noexcept {
		return type;
	}

	Level getLogLevel() const noexcept {
		return level;
	}

private:
	const uint64_t id;
	string text;
	const time_t time;
	const Type type;
	const Level level;
};

} // namespace dcpp

#endif // DCPLUSPLUS_DCPP_BDC_LOG_MESSAGE_H