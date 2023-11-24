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

#ifndef DCPLUSPLUS_WIN32_BDC_FRAME_H
#define DCPLUSPLUS_WIN32_BDC_FRAME_H

#include <dcpp/LogManager.h>
#include <dcpp/LogManagerListener.h>

#include "StaticFrame.h"

class BDCFrame : public StaticFrame<BDCFrame>,
	private LogManagerListener
{
	typedef StaticFrame<BDCFrame> BaseType;
	friend class StaticFrame<BDCFrame>;
	friend class MDIChildFrame<BDCFrame>;

public:
	enum Status {
		STATUS_STATUS,
		STATUS_LAST
	};

	static const string id;
	const string& getId() const;

protected:
	BDCFrame(TabViewPtr parent);
	virtual ~BDCFrame();

	void layout();
	bool preClosing();
	void postClosing();

private:
	enum {
		COLUMN_FIRST,
		COLUMN_TIME = COLUMN_FIRST,
		COLUMN_ID,
		COLUMN_TYPE,
		COLUMN_LEVEL,
		COLUMN_MESSAGE,
		COLUMN_LAST
	};

	enum {
		DEBUG_ICON,
		GENERAL_ICON,
		WARNING_ICON,
		ERROR_ICON,
		SYSTEM_ICON,
		SHARE_ICON,
		PRIVATE_ICON,
		SPAM_ICON,
		SERVER_ICON,
		PLUGIN_ICON
	};

	class LogInfo {
	public:
		LogInfo(const LogMessagePtr& logMessage);

		const tstring& getText(int col) const {
			return columns[col];
		}

		int getImage(int col) const {
			switch(col) {
				case COLUMN_TYPE: return type;
				case COLUMN_LEVEL: return (level + 4);
				default: return -1;
			}
		}	

		tstring columns[COLUMN_LAST];

		string message;
		uint8_t type;
		uint8_t level;
	};

	typedef TypedTable<LogInfo, false> WidgetLogs;
	typedef WidgetLogs* WidgetLogsPtr;
	WidgetLogsPtr logTable;

	GridPtr grid;

	static dwt::ImageListPtr logIcons;

	bool handleContextMenu(dwt::ScreenCoordinate pt);
	bool handleClick(const dwt::MouseEvent& me);

	void addLog(const LogMessagePtr& logMessage);
	void openFile(const string& path) const;

	// LogManagerListener
	virtual void on(Message, const LogMessagePtr& logMsg) noexcept;
};

#endif // DCPLUSPLUS_WIN32_BDC_FRAME_H
