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

#ifndef DCPLUSPLUS_DCPP_FINISHED_MANAGER_LISTENER_H
#define DCPLUSPLUS_DCPP_FINISHED_MANAGER_LISTENER_H

#include "forward.h"

namespace dcpp {

class FinishedManagerListener {
public:
	virtual ~FinishedManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> AddedFile;
	typedef X<1> AddedUser;
	typedef X<2> UpdatedFile;
	typedef X<3> UpdatedUser;
	typedef X<4> RemovedFile;
	typedef X<5> RemovedUser;
	typedef X<6> RemovedAll;

	virtual void on(AddedFile, bool, const string&, const FinishedFileItemPtr&) noexcept { }
	virtual void on(AddedUser, bool, const HintedUser&, const FinishedUserItemPtr&) noexcept { }
	virtual void on(UpdatedFile, bool, const string&, const FinishedFileItemPtr&) noexcept { }
	virtual void on(UpdatedUser, bool, const HintedUser&) noexcept { }
	virtual void on(RemovedFile, bool, const string&) noexcept { }
	virtual void on(RemovedUser, bool, const HintedUser&) noexcept { }
	virtual void on(RemovedAll, bool) noexcept { }
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_FINISHED_MANAGER_LISTENER_H)
