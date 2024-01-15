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

#include "stdafx.h"
#include "BDCWinUtil.h"
#include "resource.h"

#include "WinUtil.h"

//#include <chrono>
#include <random>

#include <dcpp/DownloadManager.h>
#include <dcpp/GeoManager.h>
#include <dcpp/LogManager.h>
#include <dcpp/UploadManager.h>
#include <dcpp/SettingsManager.h>
#include <dcpp/version.h>

#include <bzlib.h>
#include <maxminddb/maxminddb.h>
#include <miniupnpc/miniupnpc.h>
#include <openssl/opensslv.h>
#include <zlib/zlib.h>

#include <direct.h> // _getdrives()
/*#include <dxgi.h> */// for GPU info instead of DISPLAY_DEVICE

time_t BDCWinUtil::startTime = time(nullptr);

bool BDCWinUtil::getSysInfo(tstring& line) {

	line += Text::toT("\r\n_[ ") + Text::toT(APPNAME) + Text::toT(" ") + Text::toT(VERSIONSTRING) + Text::toT(" Client Information ]_");
	line += Text::toT("\r\n |");
	line += getUptime(false);
	line += Text::toT("\r\n |");
	line += getClientInfo();
	line += Text::toT("\r\n |");
	line += getOSInfo();
	line += getSystemInfo();
	line += Text::toT("\r\n |");
	line += getLibs();
	line += _T("\r\n");

	return true;
}

tstring BDCWinUtil::getUptime(bool standalone /*= true*/) {
	tstring line;

	standalone ? line += Text::toT("\r\n | ") + _T(APPNAME) + T_(" Uptime")
		       : line += Text::toT("\r\n | ") + T_("Uptime");
	line += Text::toT("\r\n |\tCLI\t") + formatTimeDifference(time(NULL) - startTime);
	line += Text::toT("\r\n |\tSYS\t") + formatTimeDifference(::GetTickCount64() / 1000);

	return line;
}

tstring BDCWinUtil::getClientInfo() {
#if defined(_MSC_VER)
tstring ver;
	//TODO Build a table of all versions
	if(_MSC_VER >= 1910 && _MSC_VER <= 1916) { ver = Text::toT("2017"); }
	if(_MSC_VER >= 1920 && _MSC_VER <= 1929) { ver = Text::toT("2019"); }
	else if(_MSC_VER >= 1930) { ver = Text::toT("2022"); }

#define BDCPP_COMPILED_BY "Microsoft Visual C++ " + ver
#elif defined(__MINGW32__)
#define BDCPP_COMPILED_BY "MinGW " __VERSION__
#elif defined(__CYGWIN__)
#define BDCPP_COMPILED_BY "CygWin"
#elif defined(__GNUC__)
#define BDCPP_COMPILED_BY "GCC"
#endif
	tstring line;

	line += Text::toT("\r\n | ") + T_("Client");
	line += Text::toT("\r\n |\tAPP\t") + Text::toT(APPNAME);
	line += Text::toT("\r\n |\tVER\t") + Text::toT(VERSIONSTRING);
	line += Text::toT("\r\n |\tCMP\t") + _T(BDCPP_COMPILED_BY);
	line += Text::toT("\r\n |\tDOB\t") + Text::toT(__DATE__ " " __TIME__);

	return line; 
}

tstring BDCWinUtil::getSystemInfo() {
	TCHAR buf[255];
	tstring line;
	HKEY hKey;
	DWORD speedMhz;
	DWORD bufLen = 255;
	DWORD dwLen	= 4;
	MEMORYSTATUSEX memoryStatusEx;
	DISPLAY_DEVICE displayDevice;
	SYSTEM_INFO	systemInfo;

	GetSystemInfo(&systemInfo);

	memoryStatusEx.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memoryStatusEx);

	displayDevice.cb = sizeof(DISPLAY_DEVICE);
	EnumDisplayDevices(NULL, 0, &displayDevice, 1); // EDD_GET_DEVICE_INTERFACE_NAME


	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Hardware\\Description\\System\\CentralProcessor\\0\\"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		bufLen = sizeof(buf);
		if(RegQueryValueEx(hKey, _T("ProcessorNameString"), NULL, NULL, LPBYTE(&buf), &bufLen) == ERROR_SUCCESS)
			line += Text::toT("\r\n |\tCPU\t") + tstring(buf);
		RegCloseKey(hKey);
	}

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Hardware\\Description\\System\\CentralProcessor\\0\\"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
		if(RegQueryValueEx(hKey, _T("~MHz"), NULL, NULL, LPBYTE(&speedMhz), &dwLen) == ERROR_SUCCESS)
			line += Text::toT("\r\n |\tCLK\t") + Text::toT(Util::toString(float(speedMhz) / 1000)) + _T(" GHz");
		RegCloseKey(hKey);
	}

	line += Text::toT("\r\n |\tTHD\t") + Text::toT(Util::toString(systemInfo.dwNumberOfProcessors));
	line += Text::toT("\r\n |\tRAM\t") + Text::toT(Util::formatBytes(memoryStatusEx.ullAvailPhys)) + Text::toT(" / ") + Text::toT(Util::formatBytes(memoryStatusEx.ullTotalPhys)) + Text::toT(" (");
	line += Text::toT(Util::toString(memoryStatusEx.dwMemoryLoad)) + Text::toT("% used)");
	line += Text::toT("\r\n |\tSTR\t") + BDCWinUtil::diskSpaceInfo(true) + _T(" (free/total)");
	line += Text::toT("\r\n |\tGFX\t") + tstring(displayDevice.DeviceString);
	line += Text::toT("\r\n |\tRES\t") + Text::toT(Util::toString(GetSystemMetrics(SM_CXSCREEN))) + Text::toT("x") + Text::toT(Util::toString(GetSystemMetrics(SM_CYSCREEN)));

	return line;
}

tstring BDCWinUtil::getOSInfo() {
	//This requires that your exe is properly manifested in order to return the correct win version
	tstring line, os, bit;
	OSVERSIONINFOEX osv = { 0 };
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO*)&osv);

	HKEY hKey;
	TCHAR buf[255];
	DWORD bufLen = 255;

	line = Text::toT("\r\n | ") + T_("System");
	line += Text::toT("\r\n |\tOS\t");

#ifdef _WIN64
	bit = Text::toT(" 64-bit");
#else
	{
		BOOL b64 = FALSE;
		IsWow64Process(GetCurrentProcess(), &b64);
		bit = (b64 == TRUE) ? Text::toT(" 64-bit") : Text::toT(" 32-bit");
	}
#endif // _WIN64

	if(osv.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		if(osv.dwMajorVersion == 10 && osv.dwMinorVersion == 0 && osv.wProductType == VER_NT_WORKSTATION) {
			os = Text::toT("Windows 10 Build 1507"); // build 1507
			if(osv.dwBuildNumber == 10586)
				os = Text::toT("Windows 10 November Update (Version 1511)");
			else if(osv.dwBuildNumber == 14393)
				os = Text::toT("Windows 10 Anniversary Update (Version 1607)");
			else if(osv.dwBuildNumber == 15063)
				os = Text::toT("Windows 10 Creators Update (Version 1703)");
			else if(osv.dwBuildNumber == 16299)
				os = Text::toT("Windows 10 Fall Creators Update (Version 1709)");
			else if(osv.dwBuildNumber == 17134)
				os = Text::toT("Windows 10 Version 1803");
			else if(osv.dwBuildNumber == 17763)
				os = Text::toT("Windows 10 Version 1809");
			else if(osv.dwBuildNumber == 18362)
				os = Text::toT("Windows 10 Version 1903");
			else if(osv.dwBuildNumber == 18363)
				os = Text::toT("Windows 10 Version 1909");
			else if(osv.dwBuildNumber == 19041)
				os = Text::toT("Windows 10 Version 2004");
			else if(osv.dwBuildNumber == 19042)
				os = Text::toT("Windows 10 Version 20H2");
			else if(osv.dwBuildNumber == 19043)
				os = Text::toT("Windows 10 Version 21H1");
			else if((osv.dwBuildNumber >= 19044) && (osv.dwBuildNumber < 22000))
				os = Text::toT("Windows 10 Version 21H2");
			else if(osv.dwBuildNumber >= 22000)
				os = Text::toT("Windows 11 Build: ") + Text::toT(Util::toString(osv.dwBuildNumber));
		} else if(osv.dwMajorVersion == 6 && osv.dwMinorVersion == 3 && osv.wProductType == VER_NT_WORKSTATION) {
			os = Text::toT("Windows 8.1");
		} else if(osv.dwMajorVersion == 6 && osv.dwMinorVersion == 2 && osv.wProductType == VER_NT_WORKSTATION) {
			os = Text::toT("Windows 8");
		} else if(osv.dwMajorVersion == 6 && osv.dwMinorVersion == 1 && osv.wProductType == VER_NT_WORKSTATION) {
			os = Text::toT("Windows 7");
			if(osv.wServicePackMajor >= 1) {
				os = Text::toT(" SP1");
			}
		} else {
			os += Text::toT("Unsupported Windows version - build: ") + Text::toT(Util::toString(osv.dwBuildNumber));
		}

		os += _T(" ");

		//TODO - Use GetProductInfo(...) to pull the Edition 
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
			bufLen = sizeof(buf);
			if(RegQueryValueEx(hKey, _T("EditionID"), NULL, NULL, LPBYTE(&buf), &bufLen) == ERROR_SUCCESS)
				os += buf;
			RegCloseKey(hKey);
		}

		os += bit;
	}

	return line + os;
}

TStringList BDCWinUtil::findVolumes() {
	BOOL found;
	TCHAR buf[MAX_PATH];  
	HANDLE hVol;
	TStringList volumes;

	hVol = FindFirstVolume(buf, MAX_PATH);

	if(hVol != INVALID_HANDLE_VALUE) {
		volumes.push_back(buf);

		found = FindNextVolume(hVol, buf, MAX_PATH);

		//while we find drive volumes.
		while(found) { 
			volumes.push_back(buf);
			found = FindNextVolume(hVol, buf, MAX_PATH); 
		}

		found = FindVolumeClose(hVol);
	}

	return volumes;
}

tstring BDCWinUtil::diskSpaceInfo(bool onlyTotal) {
	auto ret = Util::emptyStringT;
	int64_t free = 0, totalFree = 0, size = 0, totalSize = 0, netFree = 0, netSize = 0;
	auto volumes = findVolumes();

	for(auto i = volumes.begin(); i != volumes.end(); i++) {
		if(GetDriveType((*i).c_str()) == DRIVE_CDROM || GetDriveType((*i).c_str()) == DRIVE_REMOVABLE)
			continue;
		if(GetDiskFreeSpaceEx((*i).c_str(), NULL, (PULARGE_INTEGER)&size, (PULARGE_INTEGER)&free)){
			totalFree += free;
			totalSize += size;
		}
	}

	//check for mounted Network drives
	ULONG drives = _getdrives();
	_TCHAR drive[3] = { _T('A'), _T(':'), _T('\0') };

	while(drives != 0) {
		if(drives & 1 && ( GetDriveType(drive) != DRIVE_CDROM && GetDriveType(drive) != DRIVE_REMOVABLE && GetDriveType(drive) == DRIVE_REMOTE) ){
			if(GetDiskFreeSpaceEx(drive, NULL, (PULARGE_INTEGER)&size, (PULARGE_INTEGER)&free)){
				netFree += free;
				netSize += size;
			}
		}
		++drive[0];
		drives = (drives >> 1);
	}

	if(totalSize != 0) {
		if(!onlyTotal) {
			ret += Text::toT("\r\n\t Local drive space (free / total): ") + Text::toT(Util::formatBytes(totalFree)) + Text::toT(" / ") + Text::toT(Util::formatBytes(totalSize));
			if(netSize != 0) {
				ret +=  Text::toT("\r\n\t Network drive space (free / total): ") + Text::toT(Util::formatBytes(netFree)) + Text::toT(" / ") + Text::toT(Util::formatBytes(netSize));
				ret +=  Text::toT("\r\n\t Total drive space (free / total): ") + Text::toT(Util::formatBytes((netFree + totalFree))) + Text::toT(" / ") + Text::toT(Util::formatBytes(netSize + totalSize));
			}
		} else {
			ret += Text::toT(Util::formatBytes(totalFree)) + Text::toT("/") + Text::toT(Util::formatBytes(totalSize));
		}
	} else {
		return Text::toT("Error in determining HDD space");
	}

	return ret;
}

tstring BDCWinUtil::diskInfoList() {
	auto result = Util::emptyStringT;		
	TCHAR buf[MAX_PATH];
	int64_t free = 0, size = 0 , totalFree = 0, totalSize = 0;
	int disk_count = 0;

	std::vector<tstring> results; //add in vector for sorting, nicer to look at :)
	// lookup drive volumes.
	auto volumes = findVolumes();

	for(auto i = volumes.begin(); i != volumes.end(); i++) {
		if(GetDriveType((*i).c_str()) == DRIVE_CDROM || GetDriveType((*i).c_str()) == DRIVE_REMOVABLE)
			continue;

		if((GetVolumePathNamesForVolumeName((*i).c_str(), buf, 256, NULL) != 0) &&
		   (GetDiskFreeSpaceEx((*i).c_str(), NULL, (PULARGE_INTEGER)&size, (PULARGE_INTEGER)&free) !=0)){
			tstring mountpath = buf; 
			if(!mountpath.empty()) {
				totalFree += free;
				totalSize += size;
				results.push_back((Text::toT("Mount path: ") + mountpath + Text::toT("  \tDrive space (free/total) ") + Text::toT(Util::formatBytes(free)) + Text::toT("/") +  Text::toT(Util::formatBytes(size))));
			}
		}
	}

	// and a check for mounted Network drives, todo fix a better way for network space
	ULONG drives = _getdrives();
	TCHAR drive[3] = { _T('A'), _T(':'), _T('\0') };

	while(drives != 0) {
		if(drives & 1 && ( GetDriveType(drive) != DRIVE_CDROM && GetDriveType(drive) != DRIVE_REMOVABLE && GetDriveType(drive) == DRIVE_REMOTE) ){
			if(GetDiskFreeSpaceEx(drive, NULL, (PULARGE_INTEGER)&size, (PULARGE_INTEGER)&free)){
				totalFree += free;
				totalSize += size;
				results.push_back((Text::toT("Network mount path: ") + (tstring)drive + Text::toT("  \tDrive space (free/total) ") + Text::toT(Util::formatBytes(free)) + Text::toT(" / ") +  Text::toT(Util::formatBytes(size))));
			}
		}

		++drive[0];
		drives = (drives >> 1);
	}

	result += Text::toT("\r\n");
	result += Text::toT("\r\n_[ ") + Text::toT(APPNAME) + Text::toT(" ") + Text::toT(VERSIONSTRING) + Text::toT(" Drive Statistics ]_");

	sort(results.begin(), results.end()); //sort it
	for(auto i = results.begin(); i != results.end(); ++i) {
		disk_count++;
		result += Text::toT("\r\n |\t ") + *i; 
	}
	result += Text::toT("\r\n |\t \r\n | \t Total drive space (free/total): ") + Text::toT(Util::formatBytes((totalFree))) + Text::toT(" / ") + Text::toT(Util::formatBytes(totalSize));
	result += Text::toT("\r\n |\t Total drive count: ") + Text::toT(Util::toString(disk_count));
	result += Text::toT("\r\n");

	results.clear();

	return result;
}

tstring BDCWinUtil::getLibs() {
	tstring line;

	line += Text::toT("\r\n | ") + Text::toT("Libs");
	line += Text::toT("\r\n |\tBoost version : ") + Text::toT(BOOST_LIB_VERSION);
	line += Text::toT("\r\n |\tZLib version : ") + Text::toT(ZLIB_VERSION);
	line += Text::toT("\r\n |\tBZip2 version: ") + Text::toT(BZ2_bzlibVersion());
	line += Text::toT("\r\n |\tOpenSSL version: ") + Text::toT(OPENSSL_FULL_VERSION_STR) + _T(" - ") + Text::toT(OPENSSL_RELEASE_DATE);
	line += Text::toT("\r\n |\tMiniUPnPc version : ") + Text::toT(MINIUPNPC_VERSION);
	line += Text::toT("\r\n |\tSdEx version : ") + Text::toT(Util::toString(SDEX_VERSION));
	line += Text::toT("\r\n |\tGeoIP2 version: ") + Text::toT(PACKAGE_VERSION);

	return line;
}

bool BDCWinUtil::getNetStats(tstring& line) {
	line += Text::toT("\r\n_[ ") + Text::toT(APPNAME) + Text::toT(" ") + Text::toT(VERSIONSTRING) + Text::toT(" Network Statistics ]_");

	line += Text::toT("\r\n |");
	line += Text::toT("\r\n | Uploads");
	line += Text::toT("\r\n |\tSUL\t") + Text::toT(Util::formatBytes(Socket::getTotalUp()));
	line += Text::toT("\r\n |\tTUL\t") + Text::toT(Util::formatBytes(SETTING(TOTAL_UPLOAD)));
	line += Text::toT("\r\n |\tRUL\t") + Text::toT(Util::toString(UploadManager::getInstance()->getUploadCount())) + Text::toT(" Running Upload(s)");
	line += Text::toT("\r\n |\tULS\t") + Text::toT(Util::formatBytes(UploadManager::getInstance()->getRunningAverage())) + Text::toT("/s");


	line += Text::toT("\r\n |");
	line += Text::toT("\r\n | Downloads");
	line += Text::toT("\r\n |\tSDL\t") + Text::toT(Util::formatBytes(Socket::getTotalDown()));
	line += Text::toT("\r\n |\tTDL\t") + Text::toT(Util::formatBytes(SETTING(TOTAL_DOWNLOAD)));
	line += Text::toT("\r\n |\tRDL\t") + Text::toT(Util::toString(DownloadManager::getInstance()->getDownloadCount())) + Text::toT(" Running Download(s)");
	line += Text::toT("\r\n |\tDLS\t") + Text::toT(Util::formatBytes(DownloadManager::getInstance()->getRunningAverage())) + Text::toT("/s");

	line += Text::toT("\r\n |");
	line += Text::toT("\r\n | Ratio\t") + Text::toT(Util::toString((((double)SETTING(TOTAL_UPLOAD)) / ((double)SETTING(TOTAL_DOWNLOAD)))));

	line += Text::toT("\r\n");
	return true;
}
tstring BDCWinUtil::formatTimeDifference(uint64_t diff, size_t levels /*= 3*/) {
	tstring	buf;
	int	n;

//o7 Agent_0017
#define BDC_FORMATTIME(calc, name) \
	if((n = (diff / (calc))) != 0) { \
		if(!buf.empty()) \
			buf += L' '; \
		buf += Text::toT(Util::toString(n)); \
		buf += L' '; \
		buf += Text::toT(name); \
		if(n != 1) \
			buf += L's'; \
		levels--; \
		if(levels == 0) \
			return buf; \
		diff %= (calc); \
	}

	BDC_FORMATTIME(60 * 60 * 24 * 7, "week");
	BDC_FORMATTIME(60 * 60 * 24, "day");
	BDC_FORMATTIME(60 * 60, "hour");
	BDC_FORMATTIME(60, "minute");
	BDC_FORMATTIME(1, "second");
	return buf;
}

