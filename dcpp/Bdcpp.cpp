/*
* Copyright (C) 2023 iceman50
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
#include "Bdcpp.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include "Client.h"
#include "GeoManager.h"
#include "Util.h"
#include "version.h"

namespace dcpp {

const string Bdcpp::actions[Bdcpp::ACTION_LAST] = {
	"Get file list",
	"Browse file list",
	"Match queue",
	"Send private message",
	"Add to favorites",
	"Grant extra slot",
	"Remove from queue",
	"Ignore chat",
	"Unignore chat"
};

const string Bdcpp::protocols[Bdcpp::PROTOCOL_LAST] = {
	"magnet:?",
	"://",
	"mailto:",
	"@",
	"www."
};

const string Bdcpp::logTypes[LogMessage::Type::TYPE_LAST] = {
	"Debug",
	"General",
	"Warning",
	"Error"
};

const string Bdcpp::logLevels[LogMessage::Level::LOG_LAST] = {
	"System",
	"Share",
	"Private",
	"Spam",
	"Server",
	"Plugins"
};

const string Bdcpp::charsemaillocal		= "abcdefghijklmnopqrstuvwxzyABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.!#$%&'*+/=?^_`{|}~"; // not completely correct
const string Bdcpp::charsemaildomain	= "abcdefghijklmnopqrstuvwxzyABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-."; // not completely correct
const string Bdcpp::charslink			= "abcdefghijklmnopqrstuvwxzyABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~:/?#[]@!$&'()*+,;=%"; // aside from a few exceptions
const string Bdcpp::lettersnumbers		= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const string Bdcpp::digitshex			= "1234567890ABCDEFabcdef";
const string Bdcpp::digitsnumbers		= "1234567890.,-";
const string Bdcpp::delimiters			= ",;.: \r\n\t";

string Bdcpp::getBdcppSettingsFile()	{ return Util::getPath(Util::PATH_USER_CONFIG) + "BDCPlusPlus.xml";	}

void Bdcpp::getClientParams(const Client* aClient, ParamMap& params) {
//	aClient->getHubIdentity().getParams(params, "hub", false);
	params["hubDE"]		= aClient->getHubDescription();
	params["hubDNS"]	= aClient->getAddress();
	params["hubI4"]		= aClient->getIp();
	params["hubGeoIP"]	= GeoManager::getInstance()->getCountry(aClient->getIp());
	params["hubNI"]		= aClient->getHubName();
	params["hubURL"]	= aClient->getHubUrl();
}

// I'm sure this could be done easier, but this way i know for sure it works
double Bdcpp::toDouble(const string& aDouble) {
	string dhi = aDouble;
	string::size_type i;

	// clean up
	i = 0;
	while((i = strfndfstnof(dhi, digitsnumbers, i)) != string::npos)
		dhi.erase(i, 1);

	i = strfndlstof(dhi, ".,");
	if(i == string::npos)
		return double(atoi(dhi.c_str()));

	{	// fix here for multiple subversions: 0.309.1
		// locale could be like this: 1,000.24 or 1.000,24
		auto j = strfnd(dhi.c_str(), i, dhi.c_str()+i, 1);
		if(j != string::npos)
			i = j;
	}

	string dlo(dhi.begin()+i+1, dhi.end());
	dhi.erase(i);

	// clean up some more
	i = 0;
	while((i = strfndfstof(dhi, ".,", i)) != string::npos)
		dhi.erase(i, 1);
	i = 0;
	while((i = strfndfstof(dlo, ".,-", i)) != string::npos)
		dlo.erase(i, 1);

	double	hi	= double(atoi(dhi.c_str()));
	double	lo	= double(atoi(dlo.c_str()));
	int		pow	= 1;
	size_t	dec	= dlo.size();

	if(hi < 0)
		lo *= -1;

	while(dec != 0) {
		pow *= 10;
		dec--;
	}

	return (hi + (lo / pow));
}

int32_t Bdcpp::toInt32(const string& aInt32) {
	return atoi(aInt32.c_str());
}

int64_t Bdcpp::toInt64(const string& aInt64) {
#ifdef _WIN32
		return _atoi64(aInt64.c_str());
#else
		return strtoll(aInt64.c_str(), (char**)(NULL), 10);
#endif
}

void Bdcpp::stripWhitespace(const string::value_type** ppbegin, const string::value_type** ppend) {
	// this function strips off whitespace at the beginning and the end of the pchar range
	string::size_type i = Bdcpp::strfndfstnof(*ppbegin, *ppend, " \r\n\t", 4);

	if(i == string::npos) {
		*ppbegin = *ppend;
		return;
	}

	*ppbegin += i;
	i = Bdcpp::strfndlstnof(*ppbegin, *ppend, " \r\n\t", 4);

	dcassert(i != string::npos); // this should not fail at all
	// pbegin + pos + 1 (include found char) + 1 if !EOL and found char is escape char '\\'
	*ppend = *ppbegin + i + 1 + ((((*ppbegin)+i+1) != *ppend) && ((*ppbegin)[i] == '\\'));
}

/*	0017: escaping characters

	Basically any character can be escaped using SdEx
	using the following format with hexadecimals: #FF#
	As a bonus, you can escape multiple characters in
	a row: #FFFFFF#
	Every character takes 2 hex values. If a string of
	escaped characters ends on an odd, it's converted to
	0x0F, so be aware!

	This function escapes all characters which are hard to read/type
*/
string& Bdcpp::escape(string& line) {
	if(line.empty())
		return line;

	auto	pbegin	= line.c_str();
	auto	phi		= pbegin;
	auto	pend	= pbegin + line.size();
	string	buf;

	buf.reserve(line.size() * 23 / 20); // guesstimate 1 / 20 escapes, means adding 3 character per escaped character

	while(phi != pend) {
		switch(*phi) {
		case '\\':	buf += *phi; phi++; if(phi != pend) { buf += *phi; phi++; } continue; // ignore backslash escaped characters
		case '#':	break;
		default:	if((*phi > 0x1F) && (*phi < 0x7F)) { buf += *phi; phi++; continue; } break;
		}

		buf += '#';
		charToHex(*phi, buf);
		for(phi++; phi != pend; ++phi) { // search and escape until a non-escape has been found
			if((*phi != '#') && ((*phi > 0x1F) && (*phi < 0x7F)))
				break;
			charToHex(*phi, buf);
		}
		buf += '#';
	}

	// move the string from buf to line
	const_cast<string&>(line) = std::forward<string>(buf);
	return line;
}

string& Bdcpp::checkEscape(string& line) {
	if(line.size() < 3) // the bare minimum I tell you: #F#
		return line;

	auto	pbegin	= line.c_str();
	auto	phi		= pbegin;
	auto	pend	= pbegin + line.size();
	auto	hbegin	= digitshex.c_str();
	size_t	hlen	= digitshex.size();
	string	buf;

	buf.reserve(line.size());

	while(phi != pend) {
		switch(*phi) {
		case '\\':	buf += *phi; phi++; if(phi != pend) { buf += *phi; phi++; } continue; // ignore backslash escaped characters
		case '#':	break;
		default:	buf += *phi; phi++; continue;
		}

		auto plo = phi;
		phi++;

		{
			bool nohex = false;
			while(phi != pend) {
				if(*phi == '#')
					break;
				nohex = (strfnd(hbegin, hlen, phi, 1) == string::npos);
				phi++;
				if(nohex)
					break;
			}
			if(phi == pend)	{ buf.append(plo, phi); break;		} // no ending '#', add the remainder to the buf
			if(nohex)		{ buf.append(plo, phi); continue;	}// assume faulty format, add to buf and search for next '#'
		}

		plo++;
		if(phi == plo) { // no value, add '#' to buf and check from '#' at phi
			buf += '#';
			continue;
		}

		if(phi < (plo+3)) {
			char c = hexToChar(plo, phi);
			if(c != 0x00)
				buf += c;
		} else { // escaped hex string
			char c;
			while((plo+2) < phi) {
				if((c = hexToChar(plo, plo+2)) != 0x00)
					buf += c;
				plo += 2;
			}
			if((c = hexToChar(plo, phi)) != 0x00)
				buf += c;
		}

		phi++;
	}

	// move the string from buf to line
	const_cast<string&>(line) = std::forward<string>(buf);
	return line;
}

char Bdcpp::hexToChar(const string::value_type* pbegin, const string::value_type* pend) {
	if(pbegin == pend)
		return 0x00;

	char c;
	char hex[3];

/*	// loops are generally slower, only handy on variable hex lengths
	size_t i = 0;
	for(; (i != 2) && (pbegin != pend); ++i) { // max 2 digits ended by 0x00
		hex[i] = *pbegin;
		pbegin++;
	}
	hex[i] = 0x00;
*/
	hex[0] = *pbegin;
	pbegin++;
	if(pbegin == pend)	{ hex[1] = 0x00;					}
	else				{ hex[1] = *pbegin; hex[2] = 0x00;	}

	sscanf(hex, "%x", (unsigned int*)(&c));
	return c;
}

void Bdcpp::charToHex(const char& c, string& hex) {
	char buf[8]; // arbitrary buf size
	_snprintf(buf, 8, "%X", c); // safe printing to prevent buffer overrun
	const size_t len = strlen(buf);
	dcassert(len != 0); // should be no reason to fail this assertion

	// always keep the 2-digit hex value
	if(len == 1)	{ hex += '0'; hex += buf[0];	}
	else			{ hex.append(buf, 2);			}
}

const string& Bdcpp::genUuid() {
	auto uuid = boost::uuids::random_generator()();
	return boost::lexical_cast<string>(uuid);
}

// equals
string::size_type Bdcpp::streql(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend) {
	if((pend-pbegin) != (dend-dbegin))
		return string::npos; // not the same length = false

	while(pbegin != pend) {
		if(*pbegin != *dbegin)
			return string::npos;
		pbegin++;
		dbegin++;
	}

	return 0; // because the comparison checked out at position 0
}

// compare
string::size_type Bdcpp::strcmp(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend) {
	if((pbegin == pend) || (dbegin == dend))
		return string::npos;
	if((pend-pbegin) < (dend-dbegin))
		return string::npos;

	while(dbegin != dend) {
		if(*pbegin != *dbegin)
			return string::npos;
		pbegin++;
		dbegin++;
	}

	return 0; // because the comparison checked out at position 0
}

// find
string::size_type Bdcpp::strfnd(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend) {
	if((pbegin == pend) || (dbegin == dend))
		return string::npos;
	if((pend-pbegin) < (dend-dbegin))
		return string::npos;

	auto phi = pbegin;
	auto pdend = pend - (dend - dbegin) + 1;

	while(phi != pdend) {
 		if(strcmp(phi, pend, dbegin, dend) != string::npos)
			return (phi - pbegin);
		phi++;
	}

	return string::npos;
}

// find_first_of
string::size_type Bdcpp::strfndfstof(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend) {
	if((pbegin == pend) || (dbegin == dend))
		return string::npos;

	auto phi = pbegin;

	while(phi != pend) {
		auto d = dbegin;
		while(d != dend) {
			if(*d == *phi)
				return (phi - pbegin);
			d++;
		}
		phi++;
	}

	return string::npos;
}

// find_first_not_of
string::size_type Bdcpp::strfndfstnof(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend) {
	if((pbegin == pend) || (dbegin == dend))
		return string::npos;

	auto phi = pbegin;

	while(phi != pend) {
		auto d = dbegin;
		while(d != dend) {
			if(*d == *phi)
				break;
			d++;
		}
		if(d == dend)
			return (phi - pbegin);
		phi++;
	}

	return string::npos;
}

// find_last_of
string::size_type Bdcpp::strfndlstof(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend) {
	if((pbegin == pend) || (dbegin == dend))
		return string::npos;

	auto phi = pend;

	while(phi != pbegin) { // backwards, we want to find the _last_ hit
		phi--;
		auto d = dbegin;
		while(d != dend) {
			if(*d == *phi)
				return (phi - pbegin);
			d++;
		}
	}

	return string::npos;
}

// find_last_of_not_of
string::size_type Bdcpp::strfndlstnof(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend) {
	if((pbegin == pend) || (dbegin == dend))
		return string::npos;

	auto phi = pend;

	while(phi != pbegin) { // backwards, we want to find the _last_ not hit
		phi--;
		auto d = dbegin;
		while(d != dend) {
			if(*d == *phi)
				break;
			d++;
		}
		if(d == dend)
			return (phi - pbegin);
	}

	return string::npos;
}

} // namespace dcpp
