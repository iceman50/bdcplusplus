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
#include "ADLSProperties.h"

#include <dcpp/ADLSearch.h>
#include <dcpp/FavoriteManager.h>

#include <dwt/widgets/Grid.h>

#include "resource.h"
#include "WinUtil.h"

using dwt::CheckBox;
using dwt::Grid;
using dwt::GridInfo;

ADLSProperties::ADLSProperties(dwt::Widget* parent, ADLSearch& search_) :
GridDialog(parent, 290, DS_CENTERMOUSE),
searchString(0),
searchType(0),
minSize(0),
maxSize(0),
sizeType(0),
destDir(0),
active(0),
autoQueue(0),
search(search_)
{
	onInitDialog([this] { return handleInitDialog(); });
}

ADLSProperties::~ADLSProperties() {
}

bool ADLSProperties::handleInitDialog() {
	grid = addChild(Grid::Seed(4, 2));
	grid->column(0).mode = GridInfo::FILL;

	GroupBoxPtr group = grid->addChild(GroupBox::Seed(T_("Search String")));
	{
		GridPtr cur = group->addChild(Grid::Seed(2, 1));
		cur->column(0).mode = GridInfo::FILL;

		searchString = cur->addChild(WinUtil::Seeds::Dialog::textBox);
		searchString->setText(Text::toT(search.searchString));

		reg = cur->addChild(CheckBox::Seed(T_("Regular Expression")));
		reg->setChecked(search.isRegEx());
	}

	group = grid->addChild(GroupBox::Seed(T_("Source Type")));
	searchType = group->addChild(WinUtil::Seeds::Dialog::comboBox);
	searchType->addValue(T_("Filename"));
	searchType->addValue(T_("Directory"));
	searchType->addValue(T_("Full Path"));
	searchType->setSelected(search.sourceType);

	{
		GridPtr cur = grid->addChild(Grid::Seed(1, 2));
		cur->column(0).mode = GridInfo::FILL;
		cur->column(1).mode = GridInfo::FILL;

		group = cur->addChild(GroupBox::Seed(T_("Min FileSize")));
		minSize = group->addChild(WinUtil::Seeds::Dialog::intTextBox);
		minSize->setText((search.minFileSize > 0) ? Text::toT(std::to_string(search.minFileSize)) : Util::emptyStringT);

		group = cur->addChild(GroupBox::Seed(T_("Max FileSize")));
		maxSize = group->addChild(WinUtil::Seeds::Dialog::intTextBox);
		maxSize->setText((search.maxFileSize > 0) ? Text::toT(std::to_string(search.maxFileSize)) : Util::emptyStringT);
	}

	group = grid->addChild(GroupBox::Seed(T_("Size Type")));
	sizeType = group->addChild(WinUtil::Seeds::Dialog::comboBox);
	sizeType->addValue(T_("B"));
	sizeType->addValue(T_("KiB"));
	sizeType->addValue(T_("MiB"));
	sizeType->addValue(T_("GiB"));
	sizeType->setSelected(search.typeFileSize);

	group = grid->addChild(GroupBox::Seed(T_("Destination Directory")));
	destDir = group->addChild(WinUtil::Seeds::Dialog::textBox);
	destDir->setText(Text::toT(search.destDir));

	{
		GridPtr cur = grid->addChild(Grid::Seed(2, 1));

		active = cur->addChild(CheckBox::Seed(T_("Enabled")));
		active->setChecked(search.isActive);
		autoQueue = cur->addChild(CheckBox::Seed(T_("Download Matches")));
		autoQueue->setChecked(search.isAutoQueue);
	}

	WinUtil::addDlgButtons(grid,
		[this] { handleOKClicked(); },
		[this] { endDialog(IDCANCEL); });

	setText(T_("ADLSearch Properties"));

	layout();
	centerWindow();

	return false;
}

void ADLSProperties::handleOKClicked() {
	search.searchString = Text::fromT(searchString->getText());
	search.setRegEx(reg->getChecked());

	search.sourceType = ADLSearch::SourceType(searchType->getSelected());

	tstring minFileSize = minSize->getText();
	search.minFileSize = minFileSize.empty() ? -1 : Util::toInt64(Text::fromT(minFileSize));

	tstring maxFileSize = maxSize->getText();
	search.maxFileSize = maxFileSize.empty() ? -1 : Util::toInt64(Text::fromT(maxFileSize));

	search.typeFileSize = ADLSearch::SizeType(sizeType->getSelected());

	search.destDir = Text::fromT(destDir->getText());

	search.isActive = active->getChecked();

	search.isAutoQueue = autoQueue->getChecked();

	endDialog(IDOK);
}
