
/*	0017: SdEx.cpp
 *
 *	This file, with all its containing structures, has been
 *	created by Agent 0017, unless otherwise specified.
 *
 *	For all intents and purposes the normal DC++ license applies,
 *	as my addition to it is free for all to use. Also, there is
 *	no guarantee that my addition to the original program will be
 *	flawless or even useful. The reason for the additions is to
 *	enhance the original program and work out new ideas.
 *
 *	If you decide to use/modify my additions to DC++, just some
 *	credits to my original programming would do fine.
 *
 *		Agent 0017
 *		0017agent@gmail.com
 */

#include "stdinc.h"
#include "SdEx.h"

#include "BDCManager.h"
#include "SdExFull.h"

namespace dcpp {

// this should eventually be exported to a plugin
SdExBase* CreateSdExInstance() {
	return new SdExFull();
}

SdExBase* CopySdExInstance(const SdExBase* aBase) {
	return new SdExFull((const SdExFull&)(*aBase));
}

/* SdExBase */
string SdExBase::get_data(const string::value_type* pbegin, const string::value_type* pend, bool nocase /*= false*/, bool readescapes /*= true*/, bool strip /*= true*/) const {
	if(strip)
		Bdcpp::stripWhitespace(&pbegin, &pend);

	string d;
	d.reserve(pend - pbegin);

	while(pbegin != pend) {
		if((*pbegin == '\\') && readescapes) {
			pbegin++; // skip the escape char
			if(pbegin == pend)
				break;
		}
		d += *pbegin;
		pbegin++;
	}

	if(readescapes)
		Bdcpp::checkEscape(d);

	return (nocase ? Text::toLower(d) : d);
}

string& SdExBase::escape_data(string& data) const {
	if(data.empty())
		return data;

	auto	pbegin	= data.c_str();
	auto	phi		= pbegin;
	auto	pend	= pbegin + data.size();
	string	buf;

	buf.reserve(data.size() * 11 / 10); // guesstimate 1/10 escapes, adding 1 character per escaped character

	while(phi != pend) { // escape SdEx format characters with '\\'
		switch(*phi) {
		// because of whitespace stripping, data beginning or ending with whitespace should have escapes
		case '\r':	// fall through
		case '\n':
		case '\t':
		case ' ':	if((phi != pbegin) && (phi != (pend-1))) { buf += *phi; phi++; continue; }
		// normal escape characters
		case '{':	// fall through
		case '}':
		case ',':
		case ';':
		case '[':
		case ']':
//		case '%':	// no real need to ever escape this character
		case '\\':	buf += '\\'; // fall through and add the character itself
		default:	buf += *phi; phi++; continue;
		}
	}

	// move the string from buf to data
	const_cast<string&>(data) = std::forward<string>(buf);
	return Bdcpp::escape(data); // finally escape hard-to-read/type characters
}

/* SdEx */
SdEx::SdEx(const string& aPattern /*= ""*/, bool aNocase /*= false*/, bool aReadEscapes /*= true*/) : base(nullptr) {
	createInstance();
	BDCManager::addHook(this);
	getBaseSafe()->assign(&(*aPattern.begin()), &(*aPattern.end()), aNocase, aReadEscapes);
}

SdEx::SdEx(const string::value_type* pbegin, const string::value_type* pend, bool aNocase /*= false*/, bool aReadEscapes /*= true*/) : base(nullptr) {
	createInstance();
	BDCManager::addHook(this);
	getBaseSafe()->assign(pbegin, pend, aNocase, aReadEscapes);
}

SdEx::SdEx(const SdEx& rhs) : base(nullptr) {
	BDCManager::addHook(this);
	copy(rhs);
}

SdEx::~SdEx() {
	BDCManager::removeHook(this);
	deleteInstance();
}

void SdEx::update() {
	string pattern = getBaseSafe()->pattern;
	bool nc = getBaseSafe()->nocase;
	bool re = getBaseSafe()->readescapes;
	createInstance(); // update to a new base
	getBaseSafe()->assign(&(*pattern.begin()), &(*pattern.end()), nc, re);
}

void SdEx::createInstance() {
	UpdateLock lock(mutex);
	deleteInstance(); // lets make sure we are not creating memory leaks
	base = CreateSdExInstance();
}

void SdEx::copyInstance(const SdExBase* aBase) {
	UpdateLock lock(mutex);
	deleteInstance();
	base = CopySdExInstance(aBase);
}

void SdEx::deleteInstance() {
	if(base)
		delete base; // the virtual destructor should destroy the derived class as well
	base = nullptr;
}

}
