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

#include "stdinc.h"

#include "LogMessage.h"

namespace dcpp {

std::atomic<uint64_t> idCounter { 1 };

LogMessage::LogMessage(const string& aMessage, Type _type, Level _level) noexcept :
	id(idCounter++), text(aMessage), time(GET_TIME()), type(_type), level(_level)
{
}

}
