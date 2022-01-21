/*
 * Copyright (C) 2022 iceman50
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DCPLUSPLUS_WIN32_INFO_FRAME_H
#define DCPLUSPLUS_WIN32_INFO_FRAME_H

#include "MDIChildFrame.h"

#include <map>

class InfoFrame : public MDIChildFrame<InfoFrame>
{
	typedef MDIChildFrame<InfoFrame> BaseType;
public:
	typedef std::map<string, string> InfoMap;

	static const string id;
	const string& getId() const;

	static void openWindow(TabViewPtr parent, const string& userName, const InfoFrame::InfoMap& userInfo, bool activate = true);

	enum Status {
		STATUS_STATUS,
		STATUS_LAST
	};

private:
	friend class MDIChildFrame<InfoFrame>;

	InfoFrame(TabViewPtr parent, const string& userName, const InfoFrame::InfoMap& userInfo);
	virtual ~InfoFrame() { }

	void layout();

	void handleFontChange();

	GridPtr grid;
	TextBoxPtr pad;

	const string userName;
	const string info;
	const InfoMap userInfo;
};

#endif
