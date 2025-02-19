/*
 * Copyright (C) 2001-2024 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_FORWARD_H_
#define DCPLUSPLUS_DCPP_FORWARD_H_

/** @file
 * This file contains forward declarations for the various DC++ classes
 */

#include <boost/smart_ptr/intrusive_ptr.hpp>

namespace dcpp {

class AdcCommand;

class ADLSearch;

class BufferedSocket;

struct ChatMessage;

class CID;

class Client;

class ClientManager;

class ConnectionQueueItem;

class CRC32Filter;

struct DcextInfo;

class Download;
typedef Download* DownloadPtr;

class FavoriteHubEntry;
typedef FavoriteHubEntry* FavoriteHubEntryPtr;

class FavoriteUser;

class File;

class FinishedFileItem;
typedef boost::intrusive_ptr<FinishedFileItem> FinishedFileItemPtr;

class FinishedUserItem;
typedef boost::intrusive_ptr<FinishedUserItem> FinishedUserItemPtr;

class FinishedManager;

template<class Hasher>
struct HashValue;

struct HintedUser;

class HttpConnection;

class HubEntry;

class Identity;

class InputStream;

class LogManager;

class LogMessage;
typedef std::shared_ptr<LogMessage> LogMessagePtr;
typedef std::deque<LogMessagePtr> LogMessageList;

class OnlineUser;
typedef OnlineUser* OnlineUserPtr;

class OutputStream;

class QueueItem;
typedef QueueItem* QueueItemPtr;

class SearchResult;
typedef boost::intrusive_ptr<SearchResult> SearchResultPtr;

class Socket;
class SocketException;

class StringOutputStream;
class StringRefOutputStream;

class StringSearch;

class Tagger;

class TigerHash;

class Transfer;

typedef HashValue<TigerHash> TTHValue;

class UnZFilter;

class Upload;
typedef Upload* UploadPtr;

class User;
typedef boost::intrusive_ptr<User> UserPtr;

class UserCommand;

class UserConnection;
typedef UserConnection* UserConnectionPtr;

struct UserMatch;

class WindowInfo;

} // namespace dcpp

#endif /*DCPLUSPLUS_DCPP_FORWARD_H_*/
