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

#include "SettingsDialog.h"

#include <dcpp/BDCManager.h>
#include <dcpp/SettingsManager.h>

#include <dwt/util/GDI.h>
#include <dwt/widgets/Grid.h>
#include <dwt/widgets/ScrolledContainer.h>

#include "WinUtil.h"

#include "GeneralPage.h"

#include "ConnectivityPage.h"
#include "ConnectivityManualPage.h"
#include "BandwidthLimitPage.h"
#include "ProxyPage.h"

#include "DownloadPage.h"
#include "FavoriteDirsPage.h"
#include "QueuePage.h"

#include "UploadPage.h"
#include "UploadFilteringPage.h"

//DiCe Addon
#include "BDCPage.h"
#include "BdcppTextFormattingPage.h"
#include "BdcppTextElementsPage.h"

#include "AppearancePage.h"
#include "StylesPage.h"
#include "TabsPage.h"
#include "WindowsPage.h"

#include "NotificationsPage.h"

#include "HistoryPage.h"
#include "LogPage.h"

#include "AdvancedPage.h"
#include "ExpertsPage.h"
#include "UCPage.h"
#include "CertificatesPage.h"
#include "SearchTypesPage.h"
#include "UserMatchPage.h"

#include "PluginPage.h"

using dwt::Grid;
using dwt::GridInfo;

using dwt::ToolTip;

const int SettingsDialog::pluginPagePos = 26; // remember to change when adding pages...

SettingsDialog::SettingsDialog(dwt::Widget* parent) :
dwt::ModalDialog(parent),
currentPage(0),
grid(0),
tree(0)
{
	onInitDialog([this] { return initDialog(); });
	onClosing([this] { return handleClosing(); });
}

int SettingsDialog::run() {
	auto sizeVal = [](SettingsManager::IntSetting setting) {
		return std::max(SettingsManager::getInstance()->get(setting), 200);
	};
	create(Seed(dwt::Point(sizeVal(SettingsManager::SETTINGS_WIDTH), sizeVal(SettingsManager::SETTINGS_HEIGHT)),
		WS_SIZEBOX | DS_CENTERMOUSE));
	return show();
}

SettingsDialog::~SettingsDialog() {
}

bool SettingsDialog::initDialog() {

	grid = addChild(Grid::Seed(1, 2));
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;
	grid->setSpacing(8);

	grid->column(0).size = 170;
	grid->column(0).mode = GridInfo::STATIC;
	grid->column(1).mode = GridInfo::FILL;

	{
		auto seed = Tree::Seed();
		seed.style |= WS_BORDER;
		tree = grid->addChild(seed);
		tree->setItemHeight(tree->getItemHeight() * 5 / 4);
		tree->onSelectionChanged([this] { handleSelectionChanged(); });
	}

	{
		const dwt::Point size(16, 16);
		dwt::ImageListPtr images(new dwt::ImageList(size));
		tree->setNormalImageList(images);

		auto cur = grid->addChild(Grid::Seed(3, 1));
		cur->row(0).mode = GridInfo::FILL;
		cur->row(0).align = GridInfo::STRETCH;
		cur->column(0).mode = GridInfo::FILL;
		cur->setSpacing(grid->getSpacing());

		auto container = cur->addChild(dwt::ScrolledContainer::Seed(WS_BORDER));

		const size_t setting = SETTING(SETTINGS_PAGE);
		auto addPage = [&](const tstring& title, PropPage* page, unsigned icon, HTREEITEM parent) -> HTREEITEM {
			auto index = pages.size();
			if(BDSETTING(ENABLE_ICON_THEMING)) {
				try { images->add(dwt::Icon(WinUtil::iconFilename(icon), size)); } catch(const dwt::DWTException&) { images->add(dwt::Icon(icon, size)); }
			} else {
				images->add(dwt::Icon(icon, size));
			}

			page->onVisibilityChanged([=](bool b) { if(b) {
					setSmallIcon(WinUtil::createIcon(icon, 16));
					setLargeIcon(WinUtil::createIcon(icon, 32));
			} });
			auto item = tree->insert(title, parent, TVI_LAST, 0, true, index);
			if(index == setting)
				callAsync([=] { tree->setSelected(item); tree->ensureVisible(item); });
			pages.emplace_back(page, item);
			return item;
		};

		addPage(T_("Personal information"), new GeneralPage(container), IDI_USER, TVI_ROOT);

		{
			HTREEITEM item = addPage(T_("Connectivity"), new ConnectivityPage(container), IDI_CONN_BLUE, TVI_ROOT);
			addPage(T_("Manual configuration"), new ConnectivityManualPage(container), IDI_CONN_GREY, item);
			addPage(T_("Bandwidth limiting"), new BandwidthLimitPage(container), IDI_BW_LIMITER, item);
			addPage(T_("Proxy"), new ProxyPage(container), IDI_PROXY, item);
		}

		{
			HTREEITEM item = addPage(T_("Downloads"), new DownloadPage(container), IDI_DOWNLOAD, TVI_ROOT);
			addPage(T_("Favorites"), new FavoriteDirsPage(container), IDI_FAVORITE_DIRS, item);
			addPage(T_("Queue"), new QueuePage(container), IDI_QUEUE, item);
		}

		{
			HTREEITEM item = addPage(T_("Sharing"), new UploadPage(container), IDI_UPLOAD, TVI_ROOT);
			addPage(T_("Filtering"), new UploadFilteringPage(container), IDI_UPLOAD_FILTERING, item);
		}

		{
			HTREEITEM item = addPage(T_("BDC++"), new BDCPage(container), IDI_FAVORITE_USER_ON, TVI_ROOT);
			addPage(T_("Text Formatting"), new BdcppTextFormattingPage(container), IDI_STYLES, item);
			addPage(T_("Text Elements"), new BdcppTextElementsPage(container), IDI_STYLES, item);
		}

		{
			HTREEITEM item = addPage(T_("Appearance"), new AppearancePage(container), IDI_DCPP, TVI_ROOT);
			addPage(T_("Styles"), new StylesPage(container), IDI_STYLES, item);
			addPage(T_("Tabs"), new TabsPage(container), IDI_TABS, item);
			addPage(T_("Windows"), new WindowsPage(container), IDI_WINDOWS, item);
		}

		addPage(T_("Notifications"), new NotificationsPage(container), IDI_NOTIFICATIONS, TVI_ROOT);

		{
			HTREEITEM item = addPage(T_("History"), new HistoryPage(container), IDI_CLOCK, TVI_ROOT);
			addPage(T_("Logs"), new LogPage(container), IDI_LOGS, item);
		}

		{
			HTREEITEM item = addPage(T_("Advanced"), new AdvancedPage(container), IDI_ADVANCED, TVI_ROOT);
			addPage(T_("Experts only"), new ExpertsPage(container), IDI_EXPERT, item);
			addPage(T_("User commands"), new UCPage(container), IDI_USER_OP, item);
			addPage(T_("Security & certificates"), new CertificatesPage(container), IDI_SECURE, item);
			addPage(T_("Search types"), new SearchTypesPage(container), IDI_SEARCH, item);
			addPage(T_("User matching"), new UserMatchPage(container), IDI_USERS, item);
		}

		addPage(T_("Plugins"), new PluginPage(container), IDI_PLUGINS, TVI_ROOT);
		// remember to change pluginPagePos accordingly...

		cur = cur->addChild(Grid::Seed(1, 2));
		cur->column(0).mode = GridInfo::FILL;
		cur->column(0).align = GridInfo::BOTTOM_RIGHT;
		cur->setSpacing(grid->getSpacing());

		WinUtil::addDlgButtons(cur,
			[this] { handleClosing(); handleOKClicked(); },
			[this] { handleClosing(); endDialog(IDCANCEL); });
	}

	WinUtil::setColor(tree);

	/*
	* catch WM_SETFOCUS messages (onFocus events) sent to every children of this dialog. the normal
	* way to do it would be to use an Application::Filter, but unfortunately these messages don't
	* go through there but instead are sent directly to the control's wndproc.
	*/
	/// @todo when dwt has better tracking of children, improve this
	::EnumChildWindows(handle(), EnumChildProc, reinterpret_cast<LPARAM>(this));

	addAccel(FCONTROL, VK_TAB, [this] { handleCtrlTab(false); });
	addAccel(FCONTROL | FSHIFT, VK_TAB, [this] { handleCtrlTab(true); });
	initAccels();

	updateTitle();

	centerWindow();
	onWindowPosChanged([this](const dwt::Rectangle &) { layout(); });

	return false;
}

BOOL CALLBACK SettingsDialog::EnumChildProc(HWND hwnd, LPARAM lParam) {
	//DiCe Enumerate all Tables / Text Boxes / etc to setColor in WinUtil
	
	auto control = dwt::hwnd_cast<Control*>(hwnd);

	if(dynamic_cast<TablePtr>(control) || dynamic_cast<TextBoxPtr>(control) || dynamic_cast<RichTextBoxPtr>(control)) {
		WinUtil::setColor(control);
	}

	//auto combo = dynamic_cast<ComboBoxPtr>(control);
	//if(combo) {		
	//	WinUtil::setColor(combo->getListBox());
	//	WinUtil::setColor(combo->getTextBox());
	//}

	return TRUE;
}

bool SettingsDialog::handleClosing() {
	dwt::Point pt = getWindowSize();
	SettingsManager::getInstance()->set(SettingsManager::SETTINGS_WIDTH,
		static_cast<int>(static_cast<float>(pt.x) / dwt::util::dpiFactor()));
	SettingsManager::getInstance()->set(SettingsManager::SETTINGS_HEIGHT,
		static_cast<int>(static_cast<float>(pt.y) / dwt::util::dpiFactor()));

	SettingsManager::getInstance()->set(SettingsManager::SETTINGS_PAGE,
		static_cast<int>(find_if(pages.begin(), pages.end(), CompareFirst<PropPage*, HTREEITEM>(currentPage)) - pages.begin()));

	return true;
}

void SettingsDialog::handleSelectionChanged() {
	auto sel = tree->getSelected();
	if(sel) {
		auto page = find_if(pages.begin(), pages.end(), CompareSecond<PropPage*, HTREEITEM>(sel))->first;

		// move to the top of the Z order so the ScrolledContainer thinks this is the only child.
		page->setZOrder(HWND_TOP);
		page->setVisible(true);

		if(currentPage) {
			currentPage->setVisible(false);
		}
		currentPage = page;

		updateTitle();
		layout();
	}
}

void SettingsDialog::handleOKClicked() {
	write();
	endDialog(IDOK);
}

void SettingsDialog::handleCtrlTab(bool shift) {
	HTREEITEM sel = tree->getSelected();
	HTREEITEM next = 0;
	if(!sel)
		next = tree->getFirst();
	else if(shift) {
		if(sel == tree->getFirst())
			next = tree->getLast();
	} else if(sel == tree->getLast())
		next = tree->getFirst();
	if(!next)
		next = tree->getNext(sel, shift ? TVGN_PREVIOUSVISIBLE : TVGN_NEXTVISIBLE);
	tree->setSelected(next);
}

void SettingsDialog::updateTitle() {
	tstring title;
	auto item = tree->getSelected();
	while(item) {
		title = _T(" > ") + tree->getText(item) + title;
		item = tree->getParent(item);
	}
	setText(T_("Settings") + title);
}

void SettingsDialog::write() {
	for(auto& i: pages) {
		i.first->write();
	}
}

void SettingsDialog::layout() {
	dwt::Point sz = getClientSize();
	grid->resize(dwt::Rectangle(8, 8, sz.x - 16, sz.y - 16));

	if(currentPage) {
		currentPage->getParent()->layout();
	}
}
