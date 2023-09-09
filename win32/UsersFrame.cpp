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
#include "UsersFrame.h"

#include <dcpp/FavoriteManager.h>
#include <dcpp/ClientManager.h>
#include <dcpp/QueueManager.h>
#include <dcpp/UploadManager.h>
#include <dcpp/ScopedFunctor.h>

#include <dcpp/version.h>

#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/MessageBox.h>
#include <dwt/widgets/Splitter.h>
#include <dwt/widgets/SplitterContainer.h>
#include <dwt/widgets/TextBox.h>

#include "ParamDlg.h"
#include "HoldRedraw.h"
#include "TypedTable.h"

using std::for_each;
using dwt::Grid;
using dwt::GridInfo;
using dwt::Label;
using dwt::SplitterContainer;

const string UsersFrame::id = "Users";
const string& UsersFrame::getId() const { return id; }

dwt::ImageListPtr UsersFrame::userIcons;

static const ColumnInfo usersColumns[] = {
	{ N_("Favorite"), 25, false },
	{ N_("Auto grant slot"), 25, false },
	{ N_("Nick"), 125, false },
	{ N_("Client"), 60, false },
	{ N_("Version"), 60, false },
	{ N_("Protocol"), 60, false },
	{ N_("Hub(s)"), 300, false },
	{ N_("Status / Time last seen"), 150, false },
	{ N_("Description"), 200, false },
	{ N_("CID"), 300, false }
};

struct FieldName {
	string field;
	tstring name;
	tstring (*convert)(const string &val);
};

static tstring formatBytes(const string& val) {
	return Text::toT(Util::formatBytes(val));
}

static tstring formatSpeed(const string& val) {
	return Text::toT(str(F_("%1%/s") % Util::formatBytes(val)));
}

static const FieldName fields[] =
{
	{ "NI", T_("Nick"), &Text::toT },
	{ "AW", T_("Away"), &Text::toT },
	{ "DE", T_("Description"), &Text::toT },
	{ "EM", T_("E-Mail"), &Text::toT },
	{ "SS", T_("Shared bytes"), &formatBytes },
	{ "SF", T_("Shared files"), &Text::toT },
	{ "US", T_("Upload speed"), &formatSpeed },
	{ "DS", T_("Download speed"), &formatSpeed },
	{ "SL", T_("Total slots"), &Text::toT },
	{ "FS", T_("Free slots"), &Text::toT },
	{ "HN", T_("Hubs (normal)"), &Text::toT },
	{ "HR", T_("Hubs (registered)"), &Text::toT },
	{ "HO", T_("Hubs (op)"), &Text::toT },
	{ "I4", T_("IP (v4)"), &Text::toT },
	{ "I6", T_("IP (v6)"), &Text::toT },
	{ "U4", T_("Search port (v4)"), &Text::toT },
	{ "U6", T_("Search port (v6)"), &Text::toT },
	{ "SU", T_("Features"), &Text::toT },
	{ "VE", T_("Application version"), &Text::toT },
	{ "AP", T_("Application"), &Text::toT },
	{ "ID", T_("CID"), &Text::toT },
	{ "KP", T_("TLS Keyprint"), &Text::toT },
	{ "CO", T_("Connection"), &Text::toT },
	{ "CT", T_("Client type"), &Text::toT },
	{ "TA", T_("Tag"), &Text::toT },
	//DiCe Addon
	{ "SP", T_("Supports"), &Text::toT },
	{ "LK", T_("Lock"), &Text::toT },
	{ "PK", T_("PK"), &Text::toT },
	{ "KY", T_("Key"), &Text::toT },
	{ "ST", T_("Status"), &Text::toT },
	{ "LC", T_("Locale"), &Text::toT },
	{ "BO", T_("Bot(NMDC)"), &Text::toT },
	{ "RG", T_("Registered(NMDC)"), &Text::toT },
	{ "OP", T_("Operator(NMDC)"), &Text::toT },

	{ "", _T(""), 0 }
};

UsersFrame::UsersFrame(TabViewPtr parent) :
BaseType(parent, T_("Users"), IDI_USERS, false),
grid(0),
splitter(0),
users(0),
userInfo(0),
infoBox(0),
filter(usersColumns, COLUMN_LAST, [this] { updateList(); }),
selected(-1)
{
	grid = addChild(Grid::Seed(2, 1));
	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;

	{
		splitter = grid->addChild(SplitterContainer::Seed(SETTING(USERS_PANED_POS)));

		if(!userIcons) {
			const dwt::Point size(16, 16);
			userIcons = dwt::ImageListPtr(new dwt::ImageList(size));
			if(SETTING(USE_THEME)) {
				try {
					userIcons->add(dwt::Icon(WinUtil::iconFilename(IDI_FAVORITE_USER_OFF), size));
					userIcons->add(dwt::Icon(WinUtil::iconFilename(IDI_FAVORITE_USER_ON), size));
					userIcons->add(dwt::Icon(WinUtil::iconFilename(IDI_RED_BALL), size));
					userIcons->add(dwt::Icon(WinUtil::iconFilename(IDI_GREEN_BALL), size));
					userIcons->add(dwt::Icon(WinUtil::iconFilename(IDI_DCPP), size));
					userIcons->add(dwt::Icon(WinUtil::iconFilename(IDI_WHATS_THIS), size));
					userIcons->add(dwt::Icon(WinUtil::iconFilename(IDI_USER_BOT), size));
				} catch (const dwt::DWTException&) {
					userIcons->add(dwt::Icon(IDI_FAVORITE_USER_OFF, size));
					userIcons->add(dwt::Icon(IDI_FAVORITE_USER_ON, size));
					userIcons->add(dwt::Icon(IDI_RED_BALL, size));
					userIcons->add(dwt::Icon(IDI_GREEN_BALL, size));
					userIcons->add(dwt::Icon(IDI_DCPP, size));
					userIcons->add(dwt::Icon(IDI_WHATS_THIS, size));
					userIcons->add(dwt::Icon(IDI_USER_BOT, size));
				}
			} else {
				userIcons->add(dwt::Icon(IDI_FAVORITE_USER_OFF, size));
				userIcons->add(dwt::Icon(IDI_FAVORITE_USER_ON, size));
				userIcons->add(dwt::Icon(IDI_RED_BALL, size));
				userIcons->add(dwt::Icon(IDI_GREEN_BALL, size));
				userIcons->add(dwt::Icon(IDI_DCPP, size));
				userIcons->add(dwt::Icon(IDI_WHATS_THIS, size));
				userIcons->add(dwt::Icon(IDI_USER_BOT, size));
			}
		}

		WidgetUsers::Seed cs(WinUtil::Seeds::table);
		cs.lvStyle |= LVS_EX_SUBITEMIMAGES;
		users = splitter->addChild(cs);
		addWidget(users);

		WinUtil::makeColumns(users, usersColumns, COLUMN_LAST, SETTING(USERSFRAME_ORDER), SETTING(USERSFRAME_WIDTHS));
		WinUtil::setTableSort(users, COLUMN_LAST, SettingsManager::USERSFRAME_SORT, COLUMN_NICK);

		// TODO check default (browse vs get)
		users->onDblClicked([this] { handleGetList(getParent()); });
		users->onKeyDown([this](int c) { return handleKeyDown(c); });
		users->onContextMenu([this](dwt::ScreenCoordinate pt) { return handleContextMenu(pt); });
		users->setSmallImageList(userIcons);
		users->onLeftMouseDown([this](const dwt::MouseEvent &me) { return handleClick(me); });
		users->setTimer([this] { updateAllUsers(); return true; }, 5000);

		userInfo = splitter->addChild(Grid::Seed(1,1));
		userInfo->column(0).mode = GridInfo::FILL;
		userInfo->column(0).align = GridInfo::STRETCH;
		userInfo->row(0).mode = GridInfo::FILL;
		userInfo->row(0).align = GridInfo::STRETCH;

		auto group = userInfo->addChild(GroupBox::Seed(T_("User information")));
		auto seed = WinUtil::Seeds::textBox;
		seed.style &= ~ES_AUTOHSCROLL;
		seed.style |= ES_MULTILINE | WS_VSCROLL | ES_READONLY;
		infoBox = group->addChild(seed);
	}

	{
		auto cur = grid->addChild(Grid::Seed(1, 8));
		cur->column(1).mode = GridInfo::FILL;

		auto ls = WinUtil::Seeds::label;
		ls.caption = T_("Filter:");
		cur->addChild(ls);

		filter.createTextBox(cur);
		filter.text->setCue(T_("Filter users"));
		addWidget(filter.text);

		filter.createColumnBox(cur);
		addWidget(filter.column);

		filter.createMethodBox(cur);
		addWidget(filter.method);

		auto addFilterBox = [this, cur](const tstring& text, SettingsManager::BoolSetting setting) {
			auto box = cur->addChild(WinUtil::Seeds::checkBox);
			addWidget(box);
			box->setText(text);
			box->setChecked(SettingsManager::getInstance()->get(setting, true));
			box->onClicked([=] {
				SettingsManager::getInstance()->set(setting, box->getChecked());
				updateList();
			});
		};

		addFilterBox(T_("Online"), SettingsManager::USERS_FILTER_ONLINE);
		addFilterBox(T_("Favorite"), SettingsManager::USERS_FILTER_FAVORITE);
		addFilterBox(T_("Pending download"), SettingsManager::USERS_FILTER_QUEUE);
		addFilterBox(T_("Pending upload"), SettingsManager::USERS_FILTER_WAITING);
	}

	{
		auto lock = ClientManager::getInstance()->lock();
		for(auto& i: ClientManager::getInstance()->getUsers()) {
			addUser(i.second);
		}
	}

	FavoriteManager::getInstance()->addListener(this);
	ClientManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
	QueueManager::getInstance()->addListener(this);

	initStatus();

	setTimer([this]() -> bool { updateUserInfo(); return true; }, 500);

	addAccel(FALT, 'I', [this] { filter.text->setFocus(); });
	initAccels();

	layout();

	updateList();
}

UsersFrame::~UsersFrame() {
}

void UsersFrame::layout() {
	dwt::Rectangle r { getClientSize() };

	r.size.y -= status->refresh();

	grid->resize(r);
}

bool UsersFrame::preClosing() {
	FavoriteManager::getInstance()->removeListener(this);
	ClientManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);
	QueueManager::getInstance()->removeListener(this);

	return true;
}

void UsersFrame::postClosing() {
	SettingsManager::getInstance()->set(SettingsManager::USERS_PANED_POS, splitter->getSplitterPos(0));

	SettingsManager::getInstance()->set(SettingsManager::USERSFRAME_ORDER, WinUtil::toString(users->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::USERSFRAME_WIDTHS, WinUtil::toString(users->getColumnWidths()));
	SettingsManager::getInstance()->set(SettingsManager::USERSFRAME_SORT, WinUtil::getTableSort(users));
}

UsersFrame::UserInfo::UserInfo(const UserPtr& u, bool visible) :
UserInfoBase(HintedUser(u, Util::emptyString)),
isUnknown(false),
isBot(false),
isNMDC(u->isNMDC())
{
	ver = app = _("Unknown");
	update(u, visible);
}

int UsersFrame::UserInfo::getStyle(HFONT& font, COLORREF& textColor, COLORREF& bgColor, int) const {
	auto ident = ClientManager::getInstance()->getIdentities(user);
	if(!ident.empty()) {
	
		auto style = ident[0].getStyle();

		if(!style.font.empty()) {
			auto cached = WinUtil::getUserMatchFont(style.font);
			if(cached.get()) {
				font = cached->handle();
			}
		}

		if(style.textColor != -1) {
			textColor = style.textColor;
		}

		if(style.bgColor != -1) {
			bgColor = style.bgColor;
		}
	}

	return CDRF_NEWFONT;
}

void UsersFrame::UserInfo::remove() {
	FavoriteManager::getInstance()->removeFavoriteUser(user);
}

void UsersFrame::UserInfo::update(const UserPtr& u, bool visible) {
	auto ident = ClientManager::getInstance()->getIdentities(u);
	if(ident.empty()) {
		return;
	}

	auto info = ident[0].getInfo();
	for(size_t i = 1; i < ident.size(); ++i) {
		for(auto& j: ident[i].getInfo()) {
			info[j.first] = j.second;
		}
	}	

	if(ident[0].isBot() || ident[0].isHub() || ident[0].isHidden()) {
		isBot = true;
	}

	auto application = ident[0].getApplication();
	if(application.empty() && !isBot) {
		isUnknown = true;
	} else {
		isUnknown = false;
		const auto& idx = application.find(' ');
		app = application.substr(0, idx);
		if(app == "++") {
			app = APPNAME;//++ is DC++ let's make it look nicer in the table
		}
		ver = application.substr(idx + 1);
	}

	columns[COLUMN_CLIENT] = app.empty() ? T_("Bot") : Text::toT(app);
	columns[COLUMN_VERSION] = ver.empty() ? T_("Bot") : Text::toT(ver);
	columns[COLUMN_PROTOCOL] = isNMDC ? T_("NMDC") : T_("ADC");

	auto fu = FavoriteManager::getInstance()->getFavoriteUser(u);
	if(fu) {
		isFavorite = true;
		grantSlot = fu->isSet(FavoriteUser::FLAG_GRANTSLOT);
		columns[COLUMN_NICK] = Text::toT(fu->getNick());
		if(!visible) {
			return;
		}
		columns[COLUMN_DESCRIPTION] = Text::toT(fu->getDescription());

		if(u->isOnline()) {
			columns[COLUMN_SEEN] = T_("Online");
			columns[COLUMN_HUB] = WinUtil::getHubNames(u->getCID()).first;
		} else {
			columns[COLUMN_SEEN] = fu->getLastSeen() > 0 ? Text::toT(Util::formatTime("%Y-%m-%d %H:%M", fu->getLastSeen())) : T_("Offline");
			columns[COLUMN_HUB] = Text::toT(fu->getUrl());
		}
	} else {
		isFavorite = false;
		grantSlot = false;
		columns[COLUMN_NICK] = WinUtil::getNicks(u->getCID());

		if(!visible) {
			return;
		}
		if(u->isOnline()) {
			columns[COLUMN_SEEN] = T_("Online");
			columns[COLUMN_HUB] = WinUtil::getHubNames(u->getCID()).first;
		} else {
			columns[COLUMN_SEEN] = T_("Offline");
		}
	}

	columns[COLUMN_CID] = Text::toT(u->getCID().toBase32());
}

void UsersFrame::addUser(const UserPtr& aUser) {
	if(!show(aUser, true)) {
		return;
	}

	auto ui = userInfos.find(aUser);
	if(ui == userInfos.end()) {
		auto x = userInfos.emplace(aUser, UserInfo(aUser, false)).first;

		if(matches(x->second)) {
			x->second.update(aUser, true);
			users->insert(&x->second);
		}
	} else {
		updateUser(aUser);
	}
}

void UsersFrame::updateUser(const UserPtr& aUser) {
	auto ui = userInfos.find(aUser);
	if(ui != userInfos.end()) {
		ui->second.update(aUser, false);

		if(!show(aUser, false)) {
			users->erase(&ui->second);
			if(!show(aUser, true)) {
				userInfos.erase(aUser);
			}
			return;
		}

		if(matches(ui->second)) {
			ui->second.update(aUser, true);
			auto i = users->find(&ui->second);
			if(i != -1) {
				users->update(i);
			} else {
				users->insert(&ui->second);
			}
		}
	}
}

void UsersFrame::updateUserInfo() {
	auto prevSelected = selected;
	selected = users->countSelected() == 1 ? users->getSelected() : -1;
	if(selected == prevSelected)
		return;

	infoText.clear();

	if(users->countSelected() != 1) {
		return;
	}

	auto sel = users->getSelectedData();
	if(!sel) {
		return;
	}

	auto user = sel->getUser();
	auto idents = ClientManager::getInstance()->getIdentities(user);
	if(idents.empty()) {
		return;
	}

	auto info = idents[0].getInfo();
	for(size_t i = 1; i < idents.size(); ++i) {
		for(auto& j: idents[i].getInfo()) {
			info[j.first] = j.second;
		}
	}

	for(auto f = fields; !f->field.empty(); ++f) {
		auto i = info.find(f->field);
		if(i != info.end()) {
			infoText += f->name + Text::toT(": ") + f->convert(i->second) + Text::toT(" \r\n");
			info.erase(i);
		}
	}

	for(auto& i: info) {
		infoText += Text::toT(i.first) + Text::toT(": ") + Text::toT(i.second) + Text::toT("\r\n");
	}

	auto queued = QueueManager::getInstance()->getQueued(user);
	if(queued.first) {
		infoText += Text::toT("--- ") + T_("Pending downloads") + Text::toT(" ---\r\n");
		infoText += str(TF_("Queued files: %1%") % Text::toT(Util::toString(queued.first))) + Text::toT("\r\n");
		infoText += str(TF_("Queued bytes: %1%") % Text::toT(Util::formatBytes(queued.second))) + Text::toT("\r\n");
	}

	auto files = UploadManager::getInstance()->getWaitingUserFiles(user);
	if(!files.empty()) {
		infoText += Text::toT("--- ") + T_("Pending uploads") + Text::toT(" ---\r\n");
		for(auto& i : files) {
			infoText += T_("Filename:") + Text::toT(" ") + Text::toT(i) + Text::toT("\r\n");
		}
	}

	infoBox->setText(infoText);
}

void UsersFrame::updateAllUsers() {
	auto size = users->size();
	HoldRedraw hold { users };
	for(int i = 0; i <= static_cast<int>(size); ++i) {
		auto ui = users->getData(i);
		if(!ui) continue;
		auto &user = ui->getUser();
		updateUser(user);
	}
}

void UsersFrame::handleDescription() {
	if(users->countSelected() == 1) {
		int i = users->getSelected();
		UserInfo* ui = users->getData(i);
		ParamDlg dlg(this, ui->columns[COLUMN_NICK], T_("Description"), ui->columns[COLUMN_DESCRIPTION]);

		if(dlg.run() == IDOK) {
			FavoriteManager::getInstance()->setUserDescription(ui->getUser(), Text::fromT(dlg.getValue()));
			ui->columns[COLUMN_DESCRIPTION] = dlg.getValue();
		}
	}
}

void UsersFrame::handleRemove() {
	if(!SETTING(CONFIRM_USER_REMOVAL) || dwt::MessageBox(this).show(T_("Really remove?"), _T(APPNAME) _T(" ") _T(VERSIONSTRING), dwt::MessageBox::BOX_YESNO, dwt::MessageBox::BOX_ICONQUESTION) == dwt::MessageBox::RETBOX_YES)
		users->forEachSelected(&UsersFrame::UserInfo::remove);
}

bool UsersFrame::handleKeyDown(int c) {
	switch(c) {
	case VK_DELETE:
		handleRemove();
		return true;
	case VK_RETURN:
		handleDescription();
		return true;
	}
	return false;
}

bool UsersFrame::handleContextMenu(dwt::ScreenCoordinate pt) {
	size_t sel = users->countSelected();
	if(sel > 0) {
		if(pt.x() == -1 && pt.y() == -1) {
			pt = users->getContextMenuPos();
		}

		auto menu = addChild(WinUtil::Seeds::menu);
		menu->setTitle((sel == 1) ? escapeMenu(users->getSelectedData()->getText(COLUMN_NICK)) : str(TF_("%1% users") % sel));
		appendUserItems(getParent(), menu.get());
		menu->appendSeparator();
		menu->appendItem(T_("&Description"), [this] { handleDescription(); });
		menu->appendItem(T_("&Remove"), [this] { handleRemove(); });
		WinUtil::addCopyMenu(menu.get(), users);

		menu->open(pt);

		return true;
	}

	return false;
}

bool UsersFrame::handleClick(const dwt::MouseEvent &me) {
	auto item = users->hitTest(me.pos);
	if(item.first == -1 || item.second == -1) {
		return false;
	}

	auto ui = users->getData(item.first);
	switch(item.second) {
	case COLUMN_FAVORITE:
		if(ui->isFavorite) {
			FavoriteManager::getInstance()->removeFavoriteUser(ui->getUser());
		} else {
			FavoriteManager::getInstance()->addFavoriteUser(ui->getUser());
		}
		break;

	case COLUMN_SLOT:
		FavoriteManager::getInstance()->setAutoGrant(ui->getUser(), !ui->grantSlot);
		break;

	default:
		return false;
	}

	return true;
}

void UsersFrame::updateList() {
	auto i = userInfos.begin();

	auto filterPrep = filter.prepare();
	auto filterInfoF = [this, &i](int column) { return Text::fromT(i->second.getText(column)); };

	HoldRedraw h { users };
	users->clear();
	for(; i != userInfos.end(); ++i) {
		if((filter.empty() || filter.match(filterPrep, filterInfoF)) && show(i->second.getUser(), false)) {
			i->second.update(i->first, true);
			users->insert(&i->second);
		}
	}
}

bool UsersFrame::matches(const UserInfo &ui) {
	if(!filter.empty() && !filter.match(filter.prepare(), [this, &ui](int column) { return Text::fromT(ui.getText(column)); })) {
		return false;
	}

	return show(ui.getUser(), false);
}

static bool isFav(const UserPtr &u) { return FavoriteManager::getInstance()->isFavoriteUser(u); }
static bool isWaiting(const UserPtr &u) { return UploadManager::getInstance()->isWaiting(u); }
static bool hasDownload(const UserPtr &u) { return QueueManager::getInstance()->getQueued(u).first > 0; }

bool UsersFrame::show(const UserPtr &u, bool any) const {
	if(any && (u->isOnline() || isFav(u) || isWaiting(u) || hasDownload(u))) {
		return true;
	}

	if(SETTING(USERS_FILTER_ONLINE) && !u->isOnline()) {
		return false;
	}

	if(SETTING(USERS_FILTER_FAVORITE) && !isFav(u)) {
		return false;
	}

	if(SETTING(USERS_FILTER_WAITING) && !isWaiting(u)) {
		return false;
	}

	if(SETTING(USERS_FILTER_QUEUE) && !hasDownload(u)) {
		return false;
	}

	return true;
}

UsersFrame::UserInfoList UsersFrame::selectedUsersImpl() const {
	return usersFromTable(users);
}

void UsersFrame::on(UserAdded, const FavoriteUser& aUser) noexcept {
	auto u = aUser.getUser();
	callAsync([=] { addUser(u); });
}

void UsersFrame::on(FavoriteManagerListener::UserUpdated, const FavoriteUser& aUser) noexcept {
	auto u = aUser.getUser();
	callAsync([=] { updateUser(u); });
}
void UsersFrame::on(UserRemoved, const FavoriteUser& aUser) noexcept {
	auto u = aUser.getUser();
	callAsync([=] { updateUser(u); });
}

void UsersFrame::on(StatusChanged, const UserPtr& aUser) noexcept {
	callAsync([=] { updateUser(aUser); });
}

void UsersFrame::on(UserConnected, const UserPtr& aUser) noexcept {
	callAsync([=] { addUser(aUser); });
}

void UsersFrame::on(ClientManagerListener::UserUpdated, const UserPtr& aUser) noexcept {
	callAsync([=] { updateUser(aUser); });
}

void UsersFrame::on(UserDisconnected, const UserPtr& aUser) noexcept {
	callAsync([=] { updateUser(aUser); });
}

void UsersFrame::on(WaitingAddFile, const HintedUser& aUser, const string&) noexcept {
	callAsync([=] { addUser(aUser); });
}

void UsersFrame::on(WaitingRemoveUser, const HintedUser& aUser) noexcept {
	callAsync([=] { updateUser(aUser); });
}

void UsersFrame::on(Added, QueueItem* qi) noexcept {
	for(auto& i: qi->getSources()) {
		auto u = i.getUser().user;
		callAsync([=] { addUser(u); });
	}
}

void UsersFrame::on(SourcesUpdated, QueueItem* qi) noexcept {
	for(auto& i: qi->getSources()) {
		auto u = i.getUser().user;
		callAsync([=] { updateUser(u); });
	}
}

void UsersFrame::on(Removed, QueueItem* qi) noexcept {
	for(auto& i: qi->getSources()) {
		auto u = i.getUser().user;
		callAsync([=] { updateUser(u); });
	}
}
