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

#ifndef DCPLUSPLUS_DCPP_HINTEDUSER_H_
#define DCPLUSPLUS_DCPP_HINTEDUSER_H_

#include <string>

#include "forward.h"
#include "Client.h"
#include "OnlineUser.h"
#include "User.h"

namespace dcpp {

using std::string;

/** User pointer associated to a hub url */
struct HintedUser {
	UserPtr user;
	string hint;

	HintedUser() : user(nullptr) { }
	HintedUser(const UserPtr& user, const string& hint) : user(user), hint(hint) { }
	HintedUser(const OnlineUser& ou) : HintedUser(ou.getUser(), ou.getClient().getHubUrl()) { }

	bool operator==(const UserPtr& rhs) const {
		return user == rhs;
	}
	bool operator==(const HintedUser& rhs) const {
		return user == rhs.user;
		// ignore the hint, we don't want lists with multiple instances of the same user...
	}

	operator UserPtr() const { return user; }
	operator const CID&() const { return user->getCID(); }

	explicit operator bool() const { return user.get(); }
};

}

#endif /* HINTEDUSER_H_ */
