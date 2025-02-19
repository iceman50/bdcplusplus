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

#include "stdinc.h"

#include "FileReader.h"

#include "debug.h"
#include "File.h"
#include "TimerManager.h"
#include "Text.h"
#include "Util.h"

namespace dcpp {

using std::make_pair;
using std::swap;

namespace {
static const size_t READ_FAILED = static_cast<size_t>(-1);
}

size_t FileReader::read(const string& file, const DataCallback& callback) {
	size_t ret = READ_FAILED;

	if(direct) {
		dcdebug("Reading [overlapped] %s\n", file.c_str());
		ret = readDirect(file, callback);
	}

	if(ret == READ_FAILED) {
		dcdebug("Reading [full] %s\n", file.c_str());
		ret = readCached(file, callback);
	}

	return ret;
}


/** Read entire file, never returns READ_FAILED */
size_t FileReader::readCached(const string& file, const DataCallback& callback) {
	buffer.resize(getBlockSize(0));

	auto buf = &buffer[0];
	File f(file, File::READ, File::OPEN | File::SHARED);

	size_t total = 0;
	size_t n = buffer.size();
	bool go = true;
	while(f.read(buf, n) > 0 && go) {
		go = callback(buf, n);
		total += n;
		n = buffer.size();
	}

	return total;
}

size_t FileReader::getBlockSize(size_t alignment) {
	auto block = blockSize < DEFAULT_BLOCK_SIZE ? DEFAULT_BLOCK_SIZE : blockSize;
	if(alignment > 0) {
		block = ((block + alignment - 1) / alignment) * alignment;
	}

	return block;
}

void* FileReader::align(void *buf, size_t alignment) {
	return alignment == 0 ? buf
		: reinterpret_cast<void*>(((reinterpret_cast<size_t>(buf) + alignment - 1) / alignment) * alignment);
}

#ifdef _WIN32

struct Handle : boost::noncopyable {
	Handle(HANDLE h) : h(h) { }
	~Handle() { ::CloseHandle(h); }

	operator HANDLE() { return h; }

	HANDLE h;
};

size_t FileReader::readDirect(const string& file, const DataCallback& callback) {
	DWORD sector = 0, y;

	auto tfile = Text::toT(file);

	if (!::GetDiskFreeSpace(Util::getFilePath(tfile).c_str(), &y, &sector, &y, &y)) {
		dcdebug("Failed to get sector size: %s\n", Util::translateError(::GetLastError()).c_str());
		return READ_FAILED;
	}

	auto tmp = ::CreateFile(tfile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED | FILE_FLAG_POSIX_SEMANTICS, nullptr);

	if (tmp == INVALID_HANDLE_VALUE) {
		dcdebug("Failed to open unbuffered file: %s\n", Util::translateError(::GetLastError()).c_str());
		return READ_FAILED;
	}

	Handle h(tmp);

	DWORD bufSize = static_cast<DWORD>(getBlockSize(sector));
	buffer.resize(bufSize * 2 + sector);

	auto buf = align(&buffer[0], sector);

	DWORD hn = 0;
	DWORD rn = 0;
	uint8_t* hbuf = static_cast<uint8_t*>(buf) + bufSize;
	uint8_t* rbuf = static_cast<uint8_t*>(buf);
	OVERLAPPED over = { 0 };

	// Read the first block
	auto res = ::ReadFile(h, hbuf, bufSize, NULL, &over);
	auto err = ::GetLastError();

	if(!res && err != ERROR_IO_PENDING) {
		if(err != ERROR_HANDLE_EOF) {
			dcdebug("First overlapped read failed: %s\n", Util::translateError(::GetLastError()).c_str());
			return READ_FAILED;
		}

		return 0;
	}

	// Finish the read and see how it went
	if(!GetOverlappedResult(h, &over, &hn, TRUE)) {
		err = ::GetLastError();
		if(err != ERROR_HANDLE_EOF) {
			dcdebug("First overlapped read failed: %s\n", Util::translateError(::GetLastError()).c_str());
			return READ_FAILED;
		}
	}
	over.Offset = hn;

	bool go = true;
	for (; hn == bufSize && go;) {
		// Start a new overlapped read
		res = ::ReadFile(h, rbuf, bufSize, NULL, &over);
		auto err = ::GetLastError();

		// Process the previously read data
		go = callback(hbuf, hn);

		if (!res && err != ERROR_IO_PENDING) {
			if(err != ERROR_HANDLE_EOF) {
				throw FileException(Util::translateError(err));
			}

			rn = 0;
		} else {
			// Finish the new read
			if (!GetOverlappedResult(h, &over, &rn, TRUE)) {
				err = ::GetLastError();
				if(err != ERROR_HANDLE_EOF) {
					throw FileException(Util::translateError(err));
				}

				rn = 0;
			}
		}

		*((uint64_t*)&over.Offset) += rn;

		swap(rbuf, hbuf);
		swap(rn, hn);
	}

	if(hn != 0) {
		// Process leftovers
		callback(hbuf, hn);
	}

	return *((uint64_t*)&over.Offset);
}

#else

size_t FileReader::readDirect(const string& file, const DataCallback& callback) {
	return READ_FAILED;
}

#endif
}
