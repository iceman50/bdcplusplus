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

#ifndef DCPLUSPLUS_DCPP_SSLSOCKET_H
#define DCPLUSPLUS_DCPP_SSLSOCKET_H

#include "typedefs.h"

#include "CryptoManager.h"
#include "Socket.h"
#include "Singleton.h"

#include "SSL.h"

namespace dcpp {

using std::unique_ptr;
using std::string;

class SSLSocketException : public SocketException 
{
public:
#ifdef _DEBUG
	SSLSocketException(const string& aError) noexcept : SocketException("SSLSocketException: " + aError) { }
#else //_DEBUG
	SSLSocketException(const string& aError) noexcept : SocketException(aError) { }
#endif // _DEBUG
	SSLSocketException(int aError) noexcept : SocketException(aError) { }

	virtual ~SSLSocketException() throw() { }
};

class SSLSocket : public Socket 
{
public:
	SSLSocket(CryptoManager::SSLContext context, bool allowUntrusted, const string& expKP, const string& hostName_);
	/** Creates an SSL socket without any verification */
	SSLSocket(CryptoManager::SSLContext context, const string& hostName_);

	virtual ~SSLSocket() { verifyData.reset(); }

	virtual uint16_t accept(const Socket& listeningSocket);
	virtual void connect(const string& aIp, const string& aPort);
	virtual int read(void* aBuffer, int aBufLen);
	virtual int write(const void* aBuffer, int aLen);
	virtual std::pair<bool, bool> wait(uint32_t millis, bool checkRead, bool checkWrite);
	virtual void shutdown() noexcept;
	virtual void close() noexcept;

	virtual bool isSecure() const noexcept { return true; }
	virtual bool isTrusted() const noexcept;
	virtual string getCipherName() const noexcept;
	virtual ByteVector getKeyprint() const noexcept;
	virtual bool verifyKeyprint(const string& expKeyp, bool allowUntrusted) noexcept;

	virtual bool waitConnected(uint32_t millis);
	virtual bool waitAccepted(uint32_t millis);

private:

	SSL_CTX* ctx;
	ssl::SSL ssl;

	unique_ptr<CryptoManager::SSLVerifyData> verifyData;	// application data used by CryptoManager::verify_callback(...)

	int checkSSL(int ret);
	bool waitWant(int ret, uint32_t millis);
	string hostName;
};

} // namespace dcpp

#endif // SSLSOCKET_H
