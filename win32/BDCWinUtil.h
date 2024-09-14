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


#ifndef DCPLUSPLUS_WIN32_BDC_WIN_UTIL_H
#define DCPLUSPLUS_WIN32_BDC_WIN_UTIL_H

//#include <dwt/resources/Brush.h>

#include <dcpp/Bdcpp.h>
#include <dcpp/LogMessage.h>
#include <dcpp/Util.h>

using namespace dcpp;

class BDCWinUtil {

public:
	//Stats
	static bool getSysInfo(tstring& line);
	static tstring getUptime(bool standalone = true);
	static tstring getClientInfo();
	static tstring getSystemInfo();
	static tstring getLibs();
	static tstring getOSInfo();
	static TStringList findVolumes();
	static tstring diskSpaceInfo(bool onlyTotal = false);
	static tstring diskInfoList();
	static bool getNetStats(tstring& line);
	static tstring formatTimeDifference(uint64_t diff, size_t levels = 3);
	static time_t getStartTime() { return startTime; }

	//UI

private:
	static time_t startTime;
};

#endif // DCPLUSPLUS_WIN32_BDC_WIN_UTIL_H