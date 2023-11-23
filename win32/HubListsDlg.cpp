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

#include "stdafx.h"

#include "resource.h"

#include "HubListsDlg.h"

#include <dcpp/FavoriteManager.h>
#include <dcpp/StringTokenizer.h>
#include "ParamDlg.h"

HubListsDlg::HubListsDlg(dwt::Widget* parent) :
StringListDlg(parent, getHubLists())
{
}

int HubListsDlg::run() {
	int ret = StringListDlg::run();
	if(ret == IDOK) {
		SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS,
			Text::fromT(Util::toString(_T(";"), getValues())));
	}
	return ret;
}

tstring HubListsDlg::getTitle() const {
	return T_("Configured Public Hub Lists");
}

tstring HubListsDlg::getEditTitle() const {
	return T_("Hublist");
}

tstring HubListsDlg::getEditDescription() const {
	return T_("Address");
}

void HubListsDlg::add(const tstring& s) {
	StringTokenizer<tstring> t(s, ';');
	for(auto& i: t.getTokens())
		if(!i.empty())
			insert(i);
}

void HubListsDlg::edit(unsigned row, const tstring& s) {
	ParamDlg dlg(this, getEditTitle(), getEditDescription(), s);
	if(dlg.run() == IDOK) {
		bool modified = false;
		StringTokenizer<tstring> t(dlg.getValue(), ';');
		for(auto& i: t.getTokens())
			if(!i.empty()) {
				if(!modified) {
					modify(row, i);
					modified = true;
				} else {
					insert(i, ++row);
				}
			}
	}
}

TStringList HubListsDlg::getHubLists() {
	TStringList ret;
	for(auto& i: FavoriteManager::getInstance()->getHubLists())
		ret.push_back(Text::toT(i));
	return ret;
}
