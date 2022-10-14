/*
 * Copyright (C) 2001-2022 Jacek Sieka, arnetheduck on gmail point com
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
#include "CertificatesPage.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/CryptoManager.h>

#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/MessageBox.h>

#include "resource.h"
#include "WinUtil.h"

using dwt::Grid;
using dwt::GridInfo;
using dwt::Label;

PropPage::ListItem CertificatesPage::listItems[] = {
	{ SettingsManager::REQUIRE_TLS, N_("Require TLS ADC client-client connections") },
	{ SettingsManager::ALLOW_UNTRUSTED_HUBS, N_("Allow TLS connections to hubs without trusted certificate") },
	{ SettingsManager::ALLOW_UNTRUSTED_CLIENTS, N_("Allow TLS connections to clients without trusted certificate") },
	{ SettingsManager::ENABLE_CCPM, N_("Support direct encrypted private message channels") },
	{ SettingsManager::ALWAYS_CCPM, N_("Always attempt to establish direct encrypted private message channels") },
	{ 0, 0 }
};

CertificatesPage::CertificatesPage(dwt::Widget* parent) :
PropPage(parent, 4, 1),
options(0)
{
	grid->column(0).mode = GridInfo::FILL;
	grid->row(1).mode = GridInfo::FILL;
	grid->row(1).align = GridInfo::STRETCH;

	{
		GroupBoxPtr group = grid->addChild(GroupBox::Seed(T_("Security certificates")));

		GridPtr cert = group->addChild(Grid::Seed(2, 1));
		cert->column(0).mode = GridInfo::FILL;

		{
			auto cur = cert->addChild(Grid::Seed(3, 3));
			cur->column(0).align = GridInfo::BOTTOM_RIGHT;
			cur->column(1).mode = GridInfo::FILL;

			Button::Seed dots(_T("..."));
			dots.padding.x = 10;

			cur->addChild(Label::Seed(T_("Private key file")));
			TextBoxPtr box = cur->addChild(WinUtil::Seeds::Dialog::textBox);
			items.emplace_back(box, SettingsManager::TLS_PRIVATE_KEY_FILE, PropPage::T_STR);
			ButtonPtr button = cur->addChild(dots);
			button->onClicked([this, box] { handleBrowseFile(box, SettingsManager::TLS_PRIVATE_KEY_FILE); });

			cur->addChild(Label::Seed(T_("Own certificate file")));
			box = cur->addChild(WinUtil::Seeds::Dialog::textBox);
			items.emplace_back(box, SettingsManager::TLS_CERTIFICATE_FILE, PropPage::T_STR);
			button = cur->addChild(dots);
			button->onClicked([this, box] { handleBrowseFile(box, SettingsManager::TLS_CERTIFICATE_FILE); });

			cur->addChild(Label::Seed(T_("Trusted certificates path")));
			box = cur->addChild(WinUtil::Seeds::Dialog::textBox);
			items.emplace_back(box, SettingsManager::TLS_TRUSTED_CERTIFICATES_PATH, PropPage::T_STR);
			button = cur->addChild(dots);
			button->onClicked([this, box] { handleBrowseDir(box, SettingsManager::TLS_TRUSTED_CERTIFICATES_PATH); });

			cur = cert->addChild(Grid::Seed(1, 1));
			cur->column(0).mode = GridInfo::FILL;
			cur->column(0).align = GridInfo::BOTTOM_RIGHT;

			auto gen = cur->addChild(Button::Seed(T_("Generate certificates")));
			gen->onClicked([this] { handleGenerateCertsClicked(); });
		}

	}

	options = grid->addChild(GroupBox::Seed(T_("Secure connection settings")))->addChild(WinUtil::Seeds::Dialog::optionsTable); 

	grid->addChild(Label::Seed(T_("Most of these options require that you restart DC++.")));
	grid->addChild(Label::Seed(T_("Note: TLS might not protect search results and non-keyprinted hubs.")));

	PropPage::read(items);
	PropPage::read(listItems, options);
}

CertificatesPage::~CertificatesPage() {
}

void CertificatesPage::write() {
	PropPage::write(items);
	PropPage::write(options);
}

void CertificatesPage::handleGenerateCertsClicked() {
	try {
		CryptoManager::getInstance()->generateCertificate();
		dwt::MessageBox(this).show(T_("New certificates are successfully generated."), T_("Certificate generation"), dwt::MessageBox::BOX_OK, dwt::MessageBox::BOX_ICONINFORMATION);
	} catch(const CryptoException& e) {
		dwt::MessageBox(this).show(Text::toT(e.getError()), T_("Error generating certificate"), dwt::MessageBox::BOX_OK, dwt::MessageBox::BOX_ICONSTOP);
	}
}
