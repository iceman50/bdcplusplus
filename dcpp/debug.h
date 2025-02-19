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

#ifndef DCPP_DCPLUSPLUS_DEBUG_H_
#define DCPP_DCPLUSPLUS_DEBUG_H_

#include <cstdio>

#ifdef _DEBUG

#include <cassert>

#define dcdebug printf
#ifdef _MSC_VER

#include <crtdbg.h>

#define dcassert(exp) \
do { if (!(exp)) { \
	dcdebug("Assertion hit in %s(%d): " #exp "\n", __FILE__, __LINE__); \
	if(1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, #exp)) \
_CrtDbgBreak(); } } while(false)
#else
#define dcassert(exp) assert(exp)
#endif
#define dcdrun(exp) exp
#else //_DEBUG
#define dcdebug if (false) printf
#define dcassert(exp)
#define dcdrun(exp)
#endif //_DEBUG

#endif /* DCPP_DCPLUSPLUS_DEBUG_H_ */
