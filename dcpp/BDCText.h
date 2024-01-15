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

#ifndef DCPLUSPLUS_BDCPP_TEXT_H
#define DCPLUSPLUS_BDCPP_TEXT_H

#include "forward.h"
#include "Bdcpp.h"
#include "Style.h"
#include "typedefs.h"

namespace dcpp {

	class OnlineUser;
	class Tagger;

	class BDCText {
	public:
		static void addTag(Tagger* tags, string::size_type begin, string::size_type end, const TextElement* aTe, const string& aData);
		static string cssStyle(const Style& aStyle);
		static string htmlSpan(const string& aData, const Style& aStyle = Style(), const string& aId = "", const string& aLink = "", const string& aImage = "", bool htmlraw = false);
		static void extendedFormatting(string& data, const string& aId);

		BDCText() { }
		BDCText(const string& aText, OnlineUser* from, bool isPm, bool thirdPerson, const time_t& aMessageTimestamp); // chat message
		BDCText(const string& aText, bool isPm); // chat status message
		BDCText(const LogMessage& logMsg); // system message
		BDCText(const TextFormat& aFormat, ParamMap& params);
		BDCText(const BDCText& rhs) : plainText(rhs.plainText), htmlText(rhs.htmlText), bdcppformatting(rhs.bdcppformatting), extendedformatting(rhs.extendedformatting), linksupport(rhs.linksupport), textelements(rhs.textelements) { }
		~BDCText() { }

		const string& getPlainText() const { return plainText; }
		string getHtmlText() const { return "<span id=\"message\" style=\"white-space: pre-wrap;\">" + htmlText + "</span>"; }

	private:
		string plainText;
		string htmlText;
		bool bdcppformatting;
		bool extendedformatting;
		bool linksupport;
		bool friendlylinks;
		bool textelements;

		void loadSettings();
		void append(const TextFormat& aFormat, ParamMap& params, const Style* aUserMatchStyle = NULL, OnlineUser* from = NULL);
		void links(Tagger* tags, string& data, const Style& aDefaultStyle, const string::size_type pos = 0, int protid = 0);
	};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_BDCPP_TEXT_H)
