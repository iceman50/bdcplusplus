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

#ifndef DCPLUSPLUS_DCPP_CRYPTO_MANAGER_H
#define DCPLUSPLUS_DCPP_CRYPTO_MANAGER_H

#include "typedefs.h"

#include "SettingsManager.h"

#include "Exception.h"
#include "Singleton.h"
#include "SSL.h"

#ifndef X509_V_ERR_UNSPECIFIED
#define X509_V_ERR_UNSPECIFIED 1
#endif

#include <Boost/optional.hpp>

namespace dcpp {

using std::pair;
using std::string;

STANDARD_EXCEPTION(CryptoException);

class CryptoManager : public Singleton<CryptoManager>
{
public:
	typedef pair<bool, string> SSLVerifyData;

	enum TLSTmpKeys {
		KEY_FIRST = 0,
		KEY_DH_2048 = KEY_FIRST,
		KEY_DH_4096,
		KEY_RSA_2048,
		KEY_LAST
	};

	enum SSLContext {
		SSL_CLIENT,
		SSL_SERVER
	};

	string makeKey(const string& aLock);
	const string& getLock() { return lock; }
	const string& getPk() { return pk; }
	bool isExtended(const string& aLock) { return strncmp(aLock.c_str(), "EXTENDEDPROTOCOL", 16) == 0; }

	void decodeBZ2(const uint8_t* is, size_t sz, string& os);

	SSL_CTX* getSSLContext(SSLContext wanted);

	void loadCertificates() noexcept;
	void generateCertificate();
	bool checkCertificate(int minValidityDays) noexcept;
	const ByteVector& getKeyprint() const noexcept;

	bool TLSOk() const noexcept;

	static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx);

	static int idxVerifyData;

	// Options that can also be shared with external contexts
	static void setContextOptions(SSL_CTX* aSSL, bool aServer);
	static string keyprintToString(const ByteVector& aKP) noexcept;
private:

	friend class Singleton<CryptoManager>;

	CryptoManager();
	virtual ~CryptoManager();

	ssl::SSL_CTX clientContext;
	ssl::SSL_CTX clientVerContext;
	ssl::SSL_CTX serverContext;
	ssl::SSL_CTX serverVerContext;

	void sslRandCheck();

	int getKeyLength(TLSTmpKeys key);

	bool certsLoaded;

	static char idxVerifyDataName[];
	static SSLVerifyData trustedKeyprint;

	ByteVector keyprint;
	const string lock;
	const string pk;

	string keySubst(const uint8_t* aKey, size_t len, size_t n);
	bool isExtra(uint8_t b) {
		return (b == 0 || b==5 || b==124 || b==96 || b==126 || b==36);
	}

	static string formatError(X509_STORE_CTX *ctx, const string& message);
	static string getNameEntryByNID(X509_NAME* name, int nid) noexcept;

	void loadKeyprint(const string& file) noexcept;

};

} // namespace dcpp

#endif // !defined(CRYPTO_MANAGER_H)
