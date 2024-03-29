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

#ifndef DCPLUSPLUS_DCPP_MD5HASH_H_
#define DCPLUSPLUS_DCPP_MD5HASH_H_

#include <openssl/md5.h>

#include "HashValue.h"

namespace dcpp {

class MD5Hash {
public:
	/** Hash size in bytes */
	static const size_t BITS = 128;
	static const size_t BYTES = BITS / 8;

	MD5Hash() { MD5_Init(&ctx); }

	~MD5Hash() { }

	/** Calculates the Tiger hash of the data. */
	void update(const void* data, size_t len) { MD5_Update(&ctx, data, len); }
	/** Call once all data has been processed. */
	uint8_t* finalize() { MD5_Final(reinterpret_cast<unsigned char*>(&res), &ctx); return res; }

	uint8_t* getResult() { return res; }
private:
	MD5_CTX ctx;
	uint8_t res[BYTES];
};

typedef HashValue<MD5Hash> MD5Value;

} // namespace dcpp

#endif /* DCPLUSPLUS_DCPP_MD5HASH_H_ */
