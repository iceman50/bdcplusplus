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

#ifndef DCPLUSPLUS_WIN32_FINISHED_UL_FRAME_H
#define DCPLUSPLUS_WIN32_FINISHED_UL_FRAME_H

#include "FinishedFrameBase.h"
#include "resource.h"

class FinishedULFrame : public FinishedFrameBase<FinishedULFrame, true>
{
	typedef FinishedFrameBase<FinishedULFrame, true> BaseType;
public:
	static const string id;
	const string& getId() const;

	FinishedULFrame(TabViewPtr parent);
	virtual ~FinishedULFrame() { }
};

#endif // !defined(DCPLUSPLUS_WIN32_FINISHED_UL_FRAME_H)
