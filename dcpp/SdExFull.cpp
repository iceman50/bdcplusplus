
/*	0017: SdExFull.cpp
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
#include "SdExFull.h"

#include "Bdcpp.h"
#include "BDCManager.h"
#include "Text.h"
#include "Util.h"
#include "version.h"

namespace dcpp {

SdExFull::Function SdExFull::functions[SdExFull::Function::FUNCTION_LAST] = {
	// conditions
	SdExFull::Function("consists",	"cs",	&SdExFull::Function::consists,		0,										&SdExFull::Function::example_consists,	"[characters] - True if data of [parameter] consists only of [characters]" ),
	SdExFull::Function("contains",	"ct",	&SdExFull::Function::contains,		0,										&SdExFull::Function::example_contains,	"[characters] - True if data of [parameter] contains any of [characters]"),
	SdExFull::Function("data",		"d",	&SdExFull::Function::data,			0,										&SdExFull::Function::example_data,		"[data] - True if data of [parameter] equals [value]"),
	SdExFull::Function("find",		"f",	&SdExFull::Function::find,			SdExFunctionBase::CPNT2_INT32,			&SdExFull::Function::example_find,		"[data],[position] - True if found [data] in data of [parameter] after [position]"),
	SdExFull::Function("ifelse",		"if",	&SdExFull::Function::ifelse,		SdExFunctionBase::CPNT123_SDEX,			&SdExFull::Function::example_ifelse,	"[condition],[if],[else] - If data of [parameter] matches pattern [condition] it will also be matched against [if] otherwise [else]"),
	SdExFull::Function("length",		"l",	&SdExFull::Function::length,		SdExFunctionBase::CPNT12_INT32,			&SdExFull::Function::example_length,	"[minlength],[maxlength] - True if length of [parameter] equals or is between [minlength] and [maxlength]"),
	SdExFull::Function("number",		"n32",	&SdExFull::Function::number32,		SdExFunctionBase::CPNT12_INT32,			&SdExFull::Function::example_number32,	"[minnumber],[maxnumber] - True if the value of [parameter] equals or is between [minnumber] and [maxnumber]"),
	SdExFull::Function("number64",	"n64",	&SdExFull::Function::number64,		SdExFunctionBase::CPNT12_INT64,			&SdExFull::Function::example_number64,	"[minnumber],[maxnumber] - True if the value of [parameter] equals or is between [minnumber] and [maxnumber]"),
	SdExFull::Function("pattern",	"p",	&SdExFull::Function::pattern,		SdExFunctionBase::CPNT1234_SDEX,		&SdExFull::Function::example_pattern,	"[pattern] - True if data of [parameter] matches [pattern]"),
	SdExFull::Function("validate",	"vd",	&SdExFull::Function::validate,		SdExFunctionBase::CPNT_VALIDATE,		&SdExFull::Function::example_validate,	"[name],[pattern] - True if data of parameter [name] matches [pattern]"),
	// operations
	SdExFull::Function("append",		"ap",	&SdExFull::Function::append,		SdExFunctionBase::CPNT_INSERT,			&SdExFull::Function::example_append,	"[data] - Appends [data] to the end of data of [parameter]"),
	SdExFull::Function("copy",		"cp",	&SdExFull::Function::copy,			SdExFunctionBase::CPNT1234_KEEPCASE,	&SdExFull::Function::example_copy,		"[name] - Copies data of [parameter] to parameter [name]"),
	SdExFull::Function("delete",		"del",	&SdExFull::Function::deletedata,	0,										&SdExFull::Function::example_delete,	"[data] - Deletes [data] from data of [parameter]"),
	SdExFull::Function("delimit",	"de",	&SdExFull::Function::delimit,		SdExFunctionBase::CPNT2_INT32,			&SdExFull::Function::example_delimit,	"[data],[type],[begindelimiters],[enddelimiters] - Find [data] in data of [parameter], delimited between [begindelimiters] and [enddelmiters] according to any combination of [type]: 1 = EQUALS, 2 = CONTAINS, 4 = ONLY, 8 = STARTS, 16 = ENDS"),
	SdExFull::Function("format",		"fmt",	&SdExFull::Function::format,		SdExFunctionBase::CPNT1234_SDEX,		&SdExFull::Function::example_format,	"[format] - Formats the data of [parameter] according to [format]"),
	SdExFull::Function("insert",		"ins",	&SdExFull::Function::insert,		SdExFunctionBase::CPNT_INSERT,			&SdExFull::Function::example_insert,	"[data],[position] - Inserts [data] in data of [parameter] at [position]"),
	SdExFull::Function("replace",	"r",	&SdExFull::Function::replace,		SdExFunctionBase::CPNT1234_KEEPCASE,	&SdExFull::Function::example_replace,	"[data] - Replaces data of [parameter] with [data]"),
	SdExFull::Function("resize",		"rs",	&SdExFull::Function::resize,		SdExFunctionBase::CPNT12_INT32,			&SdExFull::Function::example_resize,	"[position],[length] - Resizes the data of [parameter] to [position] with [length]"),
	SdExFull::Function("split",		"spl",	&SdExFull::Function::split,			SdExFunctionBase::CPNT_SPLIT,			&SdExFull::Function::example_split,		"[pattern1],[pattern2],[position],[data] - Splits data of [parameter] to [pattern1] and [pattern2] at the position of [data], found after [position]"),
	SdExFull::Function("strip",		"str",	&SdExFull::Function::strip,			0,										&SdExFull::Function::example_strip,		"[characters] - Deletes [characters] from data of [parameter]"),
};

bool SdExFull::check(SdExString::List& sdstrings, const string::value_type* pbegin, const string::value_type* pend, ParamMap* params) const {
	auto phi = pbegin;
	ParamMap tmpParams;

	if(params)
		tmpParams = *params; // copy over the params

	for(auto pi = parts.begin(), piend = parts.end(); pi != piend; ++pi) { // no ranged-based for loop --> pi gets updated in the function itself
		// check all the texts until the first following param
		while(!pi->isParam()) {
			if(strcmp(phi, pend, &(*pi->begin()), &(*pi->end()), pi->isNocase()) == string::npos)
				return false;

			sdstrings.emplace_back((const string&)(*pi), phi-pbegin);

			phi += pi->size();
			pi++;
			if(pi == piend)
				break; // no param found, break from search loop
		}

		if(pi == piend)
			break; // no param found, break from parts loop

		auto	cur = pi;
		size_t	pls = 0; // minimum length of all consecutive parameters combined
		bool	cmp = true; // fixed length, only true when (hasParamLengthFixed && !hasParamLengthMinimum)

		// find the first following text and process all consecutive parameters
		do { // first one at this point is always a param, so run the loop at least once
			if(pi->hasParamLengthFixed()) {
				pls += pi->getParamLengthFixed();
			} else {
				cmp = false;
			}

			if(pi->hasParamLengthMinimum()) {
				pls += pi->getParamLengthMinimum();
				cmp = false;
			}

			pi++;
		} while((pi != piend) && pi->isParam());

		if(pls > size_t(pend-phi))
			return false; // combined (min)paramlengths exceed actual data length

		auto dbegin = phi;
		auto dend = pend;

		if(pi == piend) { // no text part found
			if(cmp && (pls != size_t(dend-dbegin))) // with a fixed length parameter the rest of the line should match in length
				return false;
			phi = pend;
		} else {
			phi += pls; // update search pos

			if(cmp) { // fixed length of the parameter
				if(strcmp(phi, pend, &(*pi->begin()), &(*pi->end()), pi->isNocase()) == string::npos)
					return false;
			} else {
				string::size_type i = strfnd(phi, pend, &(*pi->begin()), &(*pi->end()), pi->isNocase());
				if(i == string::npos)
					return false;
				phi += i;
			}

			dend = phi;
			phi += pi->size();
		}

		pls = 0; // reset pls to beginning of the data
		do { // check all the found params next to each other with the same data, and at this point there is at least one parameter
			auto tmpbegin = dbegin + pls;
			auto tmpend = dend;

			if(cur->hasParamLengthFixed()) {
				pls += cur->getParamLengthFixed();
				tmpend = dbegin + pls;
			}

			SdExString sdstring(tmpbegin, tmpend, tmpbegin-pbegin, true, (const string&)(*cur));
			if(!cur->checkTags(sdstring, tmpParams))
				return false;

			auto ibegin = pbegin + sdstring.getInitialPos(); // initial data range
			auto iend = ibegin + sdstring.getInitialLength();

			if(ibegin != tmpbegin) // data has been skipped
				sdstrings.emplace_back(tmpbegin, ibegin, tmpbegin-pbegin);

			sdstrings.emplace_back(sdstring);

			if(iend != tmpend) // data has been cut off
				sdstrings.emplace_back(iend, tmpend, iend-pbegin);

			if(params && !sdstring.getName().empty())
				tmpParams[sdstring.getName()] = (const string&)(sdstring);

			cur++;
		} while(cur != pi);

		if(pi == piend)
			break;

		sdstrings.emplace_back((const string&)(*pi), phi-pbegin);
	}

	if(phi != pend)
		return false; // apparently there's more data than we checked

	if(params)
		const_cast<ParamMap&>(*params) = std::forward<ParamMap>(tmpParams);
	return true;
}

bool SdExFull::format(SdExString::List& sdstrings, ParamMap& params, bool ignoreparamfind, bool ignoretags, bool removeparam) const {
	ParamMap tmpParams = params;

	for(const auto& pi: parts) {
		if(!pi.isParam()) {
			sdstrings.emplace_back((const string&)(pi));
			continue;
		}

		SdExString sdstring;

		if(!pi.empty()) {
			auto pmi = tmpParams.find((const string&)(pi));
			if(pmi == tmpParams.end()) {
				if(!ignoreparamfind)
					return false;
			} else {
				sdstring = SdExString(boost::apply_visitor(GetString(), pmi->second), 0, true, (const string&)(pi));
				if(removeparam)
					tmpParams.erase(pmi);
			}
		}

		if(!pi.checkTags(sdstring, tmpParams)) {
			if(!ignoretags)
				return false;
		}

		sdstrings.emplace_back(sdstring);
	}

	const_cast<ParamMap&>(params) = std::forward<ParamMap>(tmpParams);
	return true;
}

void SdExFull::append(const string::value_type* pbegin, const string::value_type* pend) {
	if(pbegin == pend)
		return;

	pattern.append(pbegin, pend);

	do {
		auto phi = findParamBegin(pbegin, pend, readescapes, result);

		if(phi != pbegin)
			parts.emplace_back(this, pbegin, phi, false, nocase, readescapes, result); // just a part of the line
		if(phi == pend)
			break;

		pbegin = phi + 2; // paramBegin = "%["
		phi = findParamEnd(pbegin, pend, true, result); // always read escapes *in* a param

		parts.emplace_back(this, pbegin, phi, true, nocase, readescapes, result); // Part constructor taking int& result constructs a param part

		pbegin = phi + (phi != pend);
	} while(pbegin != pend);
}

bool SdExFull::get_example(StringPairList& list, ParamMap& params) const {
	StringPairList tmp;

	for(const auto& pi: parts) {
		if(!pi.hasTags()) // no tags no glory
			continue;
		SdExString sdstring(string("testp"), 0, true);

		for(const auto& ti: pi.getTags()) {
			for(const auto& vi: ti.second) {
				if(!ti.first.example(&vi, &sdstring, &params))
					return false;
			}
		}

		tmp.emplace_back((const string&)(pi), (const string&)(sdstring));
	}

	for(const auto& sp: tmp)
		list.emplace_back(sp);
	return true;
}

void SdExFull::get_functions(SdExFunctionBase::List& sdexfunctions) const {
	// copy over the base part of the functions
	for(const auto& fi: functions)
		sdexfunctions.emplace_back(fi);
	sdexfunctions.emplace_back("paramlengthfixed",	SdExFunctionBase::OP_SYSTEM | SdExFunctionBase::CPNT1_INT32,	"[length] - The data of [parameter] has a fixed length of [length]");
	sdexfunctions.emplace_back("paramlengthmin",	SdExFunctionBase::OP_SYSTEM | SdExFunctionBase::CPNT1_INT32,	"[length] - The data of [parameter] has a minimum length of [length]");
	sdexfunctions.emplace_back("sdexversion",		SdExFunctionBase::OP_SYSTEM,									"[version] - Gives a warning if the SdEx version does not match [version]");
}

string SdExFull::get_result() const {
	if(result.empty())
		return "No warnings in pattern";
	string txt;
	txt.reserve(128);
	for(const auto& si: result) {
		if(!txt.empty())
			txt += "\r\n";
		txt += si;
	}
	return txt;
}

int32_t SdExFull::getInt32(const SdExBase* aBase, const string::value_type* pbegin, const string::value_type* pend) {
	string d = aBase->get_data(pbegin, pend);
	string::size_type i = 0;
	while((i = Bdcpp::strfndfstnof(d, Bdcpp::digitsnumbers, i)) != string::npos)
		d.erase(i, 1);
	return Bdcpp::toInt32(d);
}

int64_t SdExFull::getInt64(const SdExBase* aBase, const string::value_type* pbegin, const string::value_type* pend) {
	string d = aBase->get_data(pbegin, pend);
	string::size_type i = 0;
	while((i = Bdcpp::strfndfstnof(d, Bdcpp::digitsnumbers, i)) != string::npos)
		d.erase(i, 1);
	return Bdcpp::toInt64(d);
}

bool SdExFull::readTag(const SdExBase* aBase, const string::value_type* pbegin, const string::value_type* pend, Tag* ptag, StringList& result) {
	string::size_type i = Bdcpp::strfnd(pbegin, pend, "(", 1);
	int ops = 0;

	if(i != string::npos) {
		auto dbegin = pbegin + i + 1;

		i = Bdcpp::strfnd(dbegin, pend, ")", 1);
		auto dend = (i == string::npos) ? pend : (dbegin + i);

		pend = dbegin - 1;

		while(dbegin != dend) {
			auto dhi = findSeparator(dbegin, dend, ',', true, result);
			string mod = aBase->get_data(dbegin, dhi, true, false, true); // not case sensitive, no escapes, strip whitespace

#define SDEX_MOD(modtype) (Bdcpp::streql(&(*mod.begin()), &(*mod.end()), modtype, strlen(modtype)) == 0)

			if(SDEX_MOD("a") || SDEX_MOD("and") || SDEX_MOD("all"))	{ ops |= SdExFunctionBase::OP_AND;			}
			else if(SDEX_MOD("o") || SDEX_MOD("or"))				{ ops |= SdExFunctionBase::OP_OR;			}
			else if(SDEX_MOD("x") || SDEX_MOD("xor"))				{ ops |= SdExFunctionBase::OP_XOR;			}
			else if(SDEX_MOD("xn") || SDEX_MOD("xnor"))				{ ops |= SdExFunctionBase::OP_XNOR;			}
			else if(SDEX_MOD("n") || SDEX_MOD("not"))				{ ops |= SdExFunctionBase::OP_NOT;			}
			else if(SDEX_MOD("na") || SDEX_MOD("nand"))				{ ops |= SdExFunctionBase::OP_NAND;			}
			else if(SDEX_MOD("no") || SDEX_MOD("nor"))				{ ops |= SdExFunctionBase::OP_NOR;			}
			else if(SDEX_MOD("nx") || SDEX_MOD("nxor"))				{ ops |= SdExFunctionBase::OP_NXOR;			}
			else if(SDEX_MOD("nxn") || SDEX_MOD("nxnor"))			{ ops |= SdExFunctionBase::OP_NXNOR;		}
			else if(SDEX_MOD("nc") || SDEX_MOD("nocase"))			{ ops |= SdExFunctionBase::OP_NOCASE;		}
			else if(SDEX_MOD("fc") || SDEX_MOD("forcecase"))		{ ops |= SdExFunctionBase::OP_FORCECASE;	}
			else { addResult(result, "Warning: unknown tag modifier: " + mod); } // unknown modifier

			if(dhi == dend) // there was no comma
				break;
			dbegin = dhi + 1;
		}
	}

	Bdcpp::stripWhitespace(&pbegin, &pend);

	const Function* f = getSdExFunction(pbegin, pend, true); // search for short cmd
	if(f) {
		ptag->setFunction(f, ops);
		return true;
	}

	f = getSdExFunction(pbegin, pend, false); // search for full cmd
	if(f) {
		ptag->setFunction(f, ops);
		return true;
	}

	if(readLegacyTag(pbegin, pend, ptag, ops, result)) {
		addResult(result, "Warning: legacy SdEx tag: " + string(pbegin, pend));
		if(!ptag->hasFunction()) {
			addResult(result, "Warning: legacy SdEx tag not supported: " + string(pbegin, pend));
			return false;
		}
		return true;
	}

	addResult(result, "Warning: tag not supported: " + string(pbegin, pend));
	return false;
}

bool SdExFull::readLegacyTag(const string::value_type* pbegin, const string::value_type* pend, Tag* ptag, int aOps, StringList& result) {
#define SDEX_TAG(cmdtype) (Bdcpp::streql(pbegin, pend, cmdtype, strlen(cmdtype)) == 0)
	string cmd;
	int ops = aOps | SdExFunctionBase::OP_OR; // by default, OP_OR is specified

	if(*pbegin == 'n') {
		ops |= SdExFunctionBase::OP_NOT;
		pbegin++;
	}

	if((pend-pbegin) > 2) {
		if(*(pend-1) == 'c') {
			if(*(pend-2) == 'f') {
				ops |= SdExFunctionBase::OP_FORCECASE;
				pend -= 2;
			} else if(*(pend-2) == 'n') {
				ops |= SdExFunctionBase::OP_NOCASE;
				pend -= 2;
			}
		}
	}

	if(SDEX_TAG("data")) {
		// catch the one exception
	} else if((pend-pbegin) > 1) {
		if(((pend-pbegin) > 3) && (*(pend-3) == 'a') && (*(pend-2) == 'l') && (*(pend-1) == 'l')) {
			ops &= ~SdExFunctionBase::OP_OR;
			ops |= SdExFunctionBase::OP_AND;
			pend -= 3;
		} else if(*(pend-1) == 'a') { // short legacy tag
			ops &= ~SdExFunctionBase::OP_OR;
			ops |= SdExFunctionBase::OP_AND;
			pend -= 1;
		}
	}

	if(		SDEX_TAG("char"		) || SDEX_TAG("c"	)) { cmd = "char";		}
	else if(SDEX_TAG("consists"	) || SDEX_TAG("cs"	)) { cmd = "consists";	}
	else if(SDEX_TAG("contains"	) || SDEX_TAG("ct"	)) { cmd = "contains";	}
	else if(SDEX_TAG("find"		) || SDEX_TAG("f"	)) { cmd = "find";		}
	else if(SDEX_TAG("pattern"	) || SDEX_TAG("p"	)) { cmd = "pattern";	}
	else if(SDEX_TAG("testparam") || SDEX_TAG("tp"	)) { cmd = "validate";	}
	else if(SDEX_TAG("delete"	) || SDEX_TAG("del"	)) { cmd = "delete";	}
	else if(SDEX_TAG("delimit"	) || SDEX_TAG("de"	)) { cmd = "delimit";	}
	else if(SDEX_TAG("replace"	) || SDEX_TAG("r"	)) { cmd = "replace";	}
	else if(SDEX_TAG("split"	) || SDEX_TAG("s"	)) { cmd = "split";		}
	else if(SDEX_TAG("strip"	) || SDEX_TAG("str"	)) { cmd = "strip";		}
	// these are handled differently on OP_NOT
	else if(SDEX_TAG("data"		) || SDEX_TAG("d"	)) { cmd = "data";		if(ops & SdExFunctionBase::OP_NOT) { ops &= ~SdExFunctionBase::OP_OR; ops |= SdExFunctionBase::OP_AND; } }
	else if(SDEX_TAG("len"		) || SDEX_TAG("l"	)) { cmd = "length";	if(ops & SdExFunctionBase::OP_NOT) { ops &= ~SdExFunctionBase::OP_OR; ops |= SdExFunctionBase::OP_AND; } }
	else if(SDEX_TAG("size"		) || SDEX_TAG("sz"	)) { cmd = "length";	if(ops & SdExFunctionBase::OP_NOT) { ops &= ~SdExFunctionBase::OP_OR; ops |= SdExFunctionBase::OP_AND; } }

	if(!cmd.empty()) {
		ptag->setFunction(getSdExFunction(&(*cmd.begin()), &(*cmd.end())), ops);
		return true; 
	}

	return false;
}

const SdExFull::Function* SdExFull::getSdExFunction(const string::value_type* pbegin, const string::value_type* pend, bool shortcmd /*= false*/) {
	for(const auto& fi: functions) {
		if(shortcmd) {
			if(Bdcpp::streql(pbegin, pend, &(*fi.cmdshort.begin()), &(*fi.cmdshort.end())) == 0)
				return &fi;
		} else {
			if(Bdcpp::streql(pbegin, pend, &(*fi.cmd.begin()), &(*fi.cmd.end())) == 0)
				return &fi;
		}
	}
	return nullptr;
}

void SdExFull::addResult(StringList& result, const string& aText) {
	if(find(result.begin(), result.end(), aText) == result.end())
		result.emplace_back(aText);
}

const string::value_type* SdExFull::findParamBegin(const string::value_type* pbegin, const string::value_type* pend, bool readescapes, StringList& result) {
	auto plo = pbegin;
	while(pbegin != pend) {
		switch(*pbegin) {
		case '\\':	pbegin += readescapes; pbegin += (pbegin != pend); continue; // escaped with '\\', skip next character if(!EOL)
		case '%':	if((pbegin+1) == pend) { return pend; } if(*(pbegin+1) == '[') { return pbegin; } pbegin += 1 + (*pbegin != '%'); continue;
		case '[':	addResult(result, "Warning: unescaped '[' in pattern: " + string(plo, pend)); break;
		case ']':	addResult(result, "Warning: unescaped ']' in pattern: " + string(plo, pend)); break;
		case '{':	addResult(result, "Warning: unescaped '{' in pattern: " + string(plo, pend)); break;
		case '}':	addResult(result, "Warning: unescaped '}' in pattern: " + string(plo, pend)); break;
		}
		pbegin++;
	}
	return pend; // none found
}

const string::value_type* SdExFull::findParamEnd(const string::value_type* pbegin, const string::value_type* pend, bool readescapes, StringList& result) {
	size_t	count1	= 0; // count brackets [ ]
	size_t	count2	= 0; // count curly brackets { }
	auto	plo		= pbegin;
	auto	popt	= pend;

	while(pbegin != pend) {
		switch(*pbegin) {
		case '\\':	pbegin += readescapes; pbegin += (pbegin != pend); continue; // escaped with '\\', skip next character if(!EOL)
		case '[':	if(count2 == 0) { addResult(result, "Warning: unescaped '[' in parameter: " + string(plo, pend)); } else { count1++; } break;
		case ']':
			{
				if(count1 == 0) {
					if(count2 == 0) {
						if(popt != pend) // we had an unescaped ']'!
							addResult(result, "Warning: unescaped ']' in tag: " + string(plo, pend));
						return pbegin;
					}
					if(popt == pend)
						popt = pbegin;
				} else {
					count1--;
				}
				break;
			}
		case '{':	count2++; break;
		case '}':	if(count2 == 0) { addResult(result, "Warning: unescaped '}' in parameter: " + string(plo, pend)); } else { count2--; } break;
		}
		pbegin++;
	}

	if(popt == pend) {
		if(count2 != 0)
			addResult(result, "Warning: tag starting with '{' missing ending '}': " + string(plo, pend)); // even an incomplete tag
		addResult(result, "Warning: parameter starting with \"%[\" missing ending ']': " + string(plo, pend)); // not even one is ending found
	} else {
		addResult(result, "Warning: unescaped '{' in parameter: " + string(plo, pend)); // assume a rogue curly bracket
	}
	return popt;
}

const string::value_type* SdExFull::findTagBegin(const string::value_type* pbegin, const string::value_type* pend, bool readescapes, StringList& result) {
	auto plo = pbegin;
	while(pbegin != pend) {
		switch(*pbegin) {
		case '\\':	pbegin += readescapes; pbegin += (pbegin != pend); continue; // escaped with '\\', skip next character if(!EOL)
		case '{':	return pbegin;
		case '}':	addResult(result, "Warning: unescaped '}' in parameter: " + string(plo, pend)); break;
		case '[':	addResult(result, "Warning: unescaped '[' in parameter: " + string(plo, pend)); break;
		case ']':	addResult(result, "Warning: unescaped ']' in parameter: " + string(plo, pend)); break;
		}
		pbegin++;
	}
	return pend; // none found
}

const string::value_type* SdExFull::findTagEnd(const string::value_type* pbegin, const string::value_type* pend, bool readescapes, StringList& result) {
	size_t	count1	= 0;
	size_t	count2	= 0;
	auto	plo		= pbegin;
	auto	popt	= pend;

	while(pbegin != pend) {
		switch(*pbegin) {
		case '\\':	pbegin += readescapes; pbegin += (pbegin != pend); continue; // escaped with '\\', skip next character if(!EOL)
		case '{':	if(count2 == 0) { addResult(result, "Warning: unescaped '{' in tag: " + string(plo, pend)); } else { count1++; } break;
		case '}':
			{
				if(count1 == 0) {
					if(count2 == 0) {
						if(popt != pend) // we had an unescaped '}'!
							addResult(result, "Warning: unescaped '}' in nested parameter: " + string(plo, pend));
						return pbegin;
					}
					if(popt == pend)
						popt = pbegin;
				} else {
					count1--;
				}
				break;
			}
		case '[':	count2++; break;
		case ']':	if(count2 == 0) { addResult(result, "Warning: unescaped ']' in parameter: " + string(plo, pend)); } else { count2--; } break;
		}
		pbegin++;
	}

	if(popt == pend) {
		if(count2 != 0)
			addResult(result, "Warning: nested parameter starting with '[' missing ending ']': " + string(plo, pend)); // even an incomplete nested parameter
		addResult(result, "Warning: tag starting with '{' missing ending '}': " + string(plo, pend)); // not even one ending is found
	} else {
		addResult(result, "Warning: unescaped '[' in tag: " + string(plo, pend)); // assume a rogue bracket
	}
	return popt;
}

const string::value_type* SdExFull::findSeparator(const string::value_type* pbegin, const string::value_type* pend, const string::value_type& separator, bool readescapes, StringList& result) {
	auto plo = pbegin;
	while(pbegin != pend) {
		switch(*pbegin) {
		case '\\':	pbegin += readescapes; pbegin += (pbegin != pend); continue; // escaped with '\\', skip next character if(!EOL)
		case '%':	pbegin++; if(pbegin == pend) { break; } if(*pbegin == '[') { pbegin++; pbegin = findParamEnd(pbegin, pend, true, result); pbegin += (pbegin != pend); continue; } pbegin++; continue;
		case '[':	addResult(result, "Warning: unescaped '[' in tag: " + string(plo, pend)); break;
		case ']':	addResult(result, "Warning: unescaped ']' in tag: " + string(plo, pend)); break;
		case '{':	addResult(result, "Warning: unescaped '{' in tag: " + string(plo, pend)); break;
		case '}':	addResult(result, "Warning: unescaped '}' in tag: " + string(plo, pend)); break;
		}
		if(*pbegin == separator)
			return pbegin;
		pbegin++;
	}
	return pend;
}

string::size_type SdExFull::strcmp(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend, bool nocase) {
	if(!nocase)
		return Bdcpp::strcmp(pbegin, pend, dbegin, dend);
	string buf;
	Text::toLower(string(pbegin, pend), buf); // on nocase, dbegin...dend is already lowered
	return Bdcpp::strcmp(&(*buf.begin()), &(*buf.end()), dbegin, dend);
}

string::size_type SdExFull::strfnd(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend, bool nocase) {
	if(!nocase)
		return Bdcpp::strfnd(pbegin, pend, dbegin, dend);
	string buf;
	Text::toLower(string(pbegin, pend), buf); // on nocase, dbegin...dend is already lowered
	return Bdcpp::strfnd(&(*buf.begin()), &(*buf.end()), dbegin, dend);
}

/* SdExFull::Part */
SdExFull::Part::Part(const SdExBase* aBase, const string::value_type* pbegin, const string::value_type* pend, bool isParam, bool aNocase, bool aReadEscapes, StringList& result) :
paramlenfixed(string::npos), paramlenmin(string::npos), param(isParam), nocase(aNocase) {
	if(!param) {
		*this = aBase->get_data(pbegin, pend, nocase, aReadEscapes, false);
		dcassert(!empty());
		return;
	}

	bool name = false;

	do {
		auto phi	= findTagBegin(pbegin, pend, aReadEscapes, result);
		auto dbegin	= pbegin; // temporary, might get edited by stripWhitespace
		auto dend	= phi; // temporary, might get edited by stripWhitespace

		Bdcpp::stripWhitespace(&dbegin, &dend);

		if(!name) {
			name = true;
			*this = aBase->get_data(dbegin, dend, false, aReadEscapes, false); // it's already stripped
		} else if(dbegin != dend) { // unused data in tag
			addResult(result, "Warning: unused data in tag " + (const string&)(*this) + ": " + string(dbegin, dend));
		}

		if(phi == pend)
			break;

		pbegin = phi + 1; // tag begin is '{'
		phi = findTagEnd(pbegin, pend, aReadEscapes, result);

		if(pbegin == phi) {
			addResult(result, "Warning: tag is empty");
			pbegin += (phi != pend);
			continue;
		}

		string::size_type i = Bdcpp::strfndfstof(pbegin, phi, "=:", 2);
		dbegin = pbegin;

		if(i == string::npos) {
			dend = phi;
			pbegin = phi;
		} else {
			dend = dbegin + i;
			pbegin = dend + 1;

			if(*dend == ':') { // legacy operation or system tag
				auto tbegin	= dbegin;
				auto tend	= dend;
				bool found	= true;

				Bdcpp::stripWhitespace(&tbegin, &tend);

				if((pbegin != phi) && (*pbegin == '=')) { // fix old system tags
					addResult(result, "Warning: legacy SdEx system tag operator \":=\" on: " + string(tbegin, tend));
					pbegin++;
				}

#define SDEX_SYSTEM_CMD(tag) (Bdcpp::streql(tbegin, tend, tag, strlen(tag)) == 0)

				if(SDEX_SYSTEM_CMD("sdexversion") || SDEX_SYSTEM_CMD("sdv")) {
					double v = Bdcpp::toDouble(aBase->get_data(pbegin, phi).c_str());
					if(v < double(SDEX_VERSION))		{ addResult(result, "Warning: pattern version is older than SdEx: pattern version is " + Util::toString(v) + ", SdEx version is " + Util::toString(double(SDEX_VERSION)));	}
					else if(v > double(SDEX_VERSION))	{ addResult(result, "Warning: pattern version is newer than SdEx: pattern version is " + Util::toString(v) + ", SdEx version is " + Util::toString(double(SDEX_VERSION)));	}
				} else if(SDEX_SYSTEM_CMD("paramlengthfixed") || SDEX_SYSTEM_CMD("plf")) {
					paramlenfixed = abs(atoi(aBase->get_data(pbegin, phi).c_str()));
				} else if(SDEX_SYSTEM_CMD("paramlengthmin") || SDEX_SYSTEM_CMD("plm")) {
					paramlenmin = abs(atoi(aBase->get_data(pbegin, phi).c_str()));
				} else {
					addResult(result, "Warning: legacy SdEx tag operator ':' on: " + string(tbegin, tend));
					found = false;
				}

				if(found) {
					pbegin = phi + (phi != pend);
					continue;
				}
			}
		}

		Tag t;
		if(readTag(aBase, dbegin, dend, &t, result))
			tags.emplace_back(t, Tokenizer(aBase, pbegin, phi, t.getOps(), nocase, result).getTokens());

		pbegin = phi + (phi != pend);
	} while(pbegin != pend);
}

bool SdExFull::Part::checkTags(SdExString& sdstring, ParamMap& params) const {
	for(const auto& ti: tags) {
		if(ti.second.empty())
			continue;

		bool	band	= (ti.first.getOps() & SdExFunctionBase::OP_AND);
		bool	bor		= (ti.first.getOps() & SdExFunctionBase::OP_OR);
		bool	bxor	= (ti.first.getOps() & SdExFunctionBase::OP_XOR);
		bool	bxnor	= (ti.first.getOps() & SdExFunctionBase::OP_XNOR);
		bool	bnot	= (ti.first.getOps() & SdExFunctionBase::OP_NOT);
		bool	valid	= false;
		int		prev	= -1;

		for(const auto& vi: ti.second) {
			valid = ti.first.check(&vi, &sdstring, &params);
			if(bnot)
				valid = !valid;

			if(band)		{ if(!valid) { return false; }											} // AND: all should be valid
			else if(bor)	{ if(valid) { break; }													} // OR: any should be valid
			else if(bxor)	{ if(valid) { if(prev == 1) { return false; } prev = 1; }				} // XOR: only one should be valid
			else if(bxnor)	{ if((prev != -1) && (prev != int(valid))) { return false; } prev = valid;	} // XNOR: either all or none should be valid
			else if(valid)	{ break; } // assume OR by default
		}

		if(!valid && (prev == -1)) // last one was not valid and there was no valid one before
			return false;
	}
	// either there were no tags or tag values, or everything checked out right
	return true;
}

/* SdExFull::Tokenizer */
SdExFull::Tokenizer::Tokenizer(const SdExBase* aBase, const string::value_type* pbegin, const string::value_type* pend, int ops, bool partNocase, StringList& result) : base(aBase) {
	if((partNocase || (ops & SdExFunctionBase::OP_NOCASE)) && !(ops & SdExFunctionBase::OP_FORCECASE))
		ops |= SdExFunctionBase::OP_NOCASE; // make sure the OP_NOCASE flag is set on the flags we will pass on

	while(true) {
		auto phi = findSeparator(pbegin, pend, ';', true, result);

		if(Bdcpp::strfndfstnof(pbegin, phi, " \r\n\t", 4) != string::npos) // not empty and not just whitespace
			addValue(pbegin, phi, ops, result);

		if(phi == pend) // there was no semicolon
			break;
		pbegin = phi + 1;
	}
}

void SdExFull::Tokenizer::addValue(const string::value_type* pbegin, const string::value_type* pend, int aOps, StringList& result) {
	Value sdv(aOps);
	int index = 0;

	while(true) {
		auto phi = findSeparator(pbegin, pend, ',', true, result);
		int sw = (aOps >> (16 + index * 4)) & 0xF;

		switch(sw) {
		case SdExFunctionBase::CPNT_INT32:	sdv.addInt32(getInt32(base, pbegin, phi)); break;
		case SdExFunctionBase::CPNT_INT64:	sdv.addInt64(getInt64(base, pbegin, phi)); break;
		case SdExFunctionBase::CPNT_SDEX:	sdv.addSdEx(SdEx(pbegin, phi, sdv.isNocase(index))); break;
		case SdExFunctionBase::CPNT_STRING:	// fall through to default
		default:							sdv.addString(base->get_data(pbegin, phi, sdv.isNocase(index))); break;
		}
		index++;

		if(phi == pend) // there was no comma
			break;
		pbegin = phi + 1;
	}

	tokens.emplace_back(sdv);
}

/* SdExFull::Function */
SdExFull::Function::Function(const string& aCmd, const string& aCmdShort, Function::Func aFunction, int aComponents, Function::Func aExample, const string& aDescription) :
	SdExFunctionBase(aCmd, aComponents, aDescription), cmdshort(aCmdShort), function(aFunction), example(aExample) {
}

SDEX_METHOD_FUNCTION(SdExFull::Function::consists) {
	const auto&		strings	= pValue->getStringList();
	const string	dlower	= pValue->isNocase() ? Text::toLower(*psdstring) : Util::emptyString;
	const string&	d		= pValue->isNocase() ? dlower : *psdstring;
	for(const auto& si: strings) {
		if(Bdcpp::strfndfstnof(d, si) != string::npos)
			return false;
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::contains) {
	const auto&		strings	= pValue->getStringList();
	const string	dlower	= pValue->isNocase() ? Text::toLower(*psdstring) : Util::emptyString;
	const string&	d		= pValue->isNocase() ? dlower : *psdstring;
	for(const auto& si: strings) {
		if(Bdcpp::strfndfstof(d, si) != string::npos)
			return true;
	}
	return false;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::data) {
	const auto&		strings	= pValue->getStringList();
	const string	dlower	= pValue->isNocase() ? Text::toLower(*psdstring) : Util::emptyString;
	const string&	d		= pValue->isNocase() ? dlower : *psdstring;
	for(const auto& si: strings) {
		if(d == si)
			return true;
	}
	return false;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::find) {
	const auto&		strings	= pValue->getStringList();
	const auto&		int32s	= pValue->getInt32List();
	const string	dlower	= pValue->isNocase() ? Text::toLower(*psdstring) : Util::emptyString;
	const string&	d		= pValue->isNocase() ? dlower : *psdstring;
	size_t			pos;
	{
		int i = (int32s.empty() ? 0 : *int32s.begin());
		if(i > int(d.size()))	{ pos = d.size();					}
		else if(i < 0)			{ pos = size_t(int(d.size()) + i);	}
		else					{ pos = size_t(i);					}
	}
	for(const auto& si: strings) {
		if(Bdcpp::strfnd(d, si, pos) != string::npos)
			return true;
	}
	return false;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::ifelse) {
	const auto& psdexs = pValue->getSdExList();
	if(psdexs.size() < 2)
		return false;
	const string dlower = pValue->isNocase() ? Text::toLower(*psdstring) : Util::emptyString;
	const string& d = pValue->isNocase() ? dlower : *psdstring;
	if((*psdexs.begin())->check(d, pparams))
		return (*(psdexs.begin()+1))->check(d, pparams);
	return (psdexs.size() == 2) ? false : (*(psdexs.begin()+2))->check(d, pparams);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::length) {
	const auto& int32s = pValue->getInt32List();
	if(int32s.empty())
		return false;
	size_t first	= size_t(*int32s.begin());
	size_t second	= size_t((int32s.size() > 1) ? *(int32s.begin()+1) : 0);
	size_t size		= psdstring->size();
	if(second == 0)		{ return (size == first);						} // equals
	if(first > second)	{ return (size >= first) || (size <= second);	} // NOT range
	return (size >= first) && (size <= second); // range
}

SDEX_METHOD_FUNCTION(SdExFull::Function::number32) {
	const auto& int32s = pValue->getInt32List();
	if(int32s.empty())
		return false;
	int32_t first	= size_t(*int32s.begin());
	int32_t second	= size_t((int32s.size() > 1) ? *(int32s.begin()+1) : 0);
	int32_t num		= atoi(psdstring->c_str());
	if(second == 0)		{ return (num == first);					}
	if(first > second)	{ return (num >= first) || (num <= second);	}
	return (num >= first) && (num <= second);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::number64) {
	const auto& int64s = pValue->getInt64List();
	if(int64s.empty())
		return false;
	int64_t first	= size_t(*int64s.begin());
	int64_t second	= size_t((int64s.size() > 1) ? *(int64s.begin()+1) : 0);
#ifdef _WIN32
	int64_t num		= _atoi64(psdstring->c_str());
#else
	int64_t num		= strtoll(psdstring->c_str(), (char**)NULL, 10);
#endif
	if(second == 0)		{ return (num == first);					}
	if(first > second)	{ return (num >= first) || (num <= second);	}
	return (num >= first) && (num <= second);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::pattern) {
	const auto&		psdexs	= pValue->getSdExList();
	const string	dlower	= pValue->isNocase() ? Text::toLower(*psdstring) : Util::emptyString;
	const string&	d		= pValue->isNocase() ? dlower : *psdstring;
	for(const auto& psi: psdexs) {
		if(psi->check(d, pparams))
			return true;
	}
	return false;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::validate) {
	const auto& strings = pValue->getStringList();
	if(strings.empty())
		return false;
	auto pmi = pparams->find(*strings.begin());
	if(pmi == pparams->end())
		return false;
	const auto& psdexs = pValue->getSdExList();
	string d = boost::apply_visitor(GetString(), pmi->second);
	if(pValue->isNocase())
		d = Text::toLower(d);
	for(const auto& psi: psdexs) {
		if(psi->check(d, pparams))
			return true;
	}
	return false;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::append) { // same functionality as insert()
	return insert(pValue, psdstring, pparams, ops);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::copy) {
	const auto& strings = pValue->getStringList();
	for(const auto& si: strings) {
		if(!si.empty())
			(*pparams)[si] = *psdstring;
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::deletedata) {
	const auto&		strings	= pValue->getStringList();
	const string	dlower	= pValue->isNocase() ? Text::toLower(*psdstring) : Util::emptyString;
	const string&	d		= pValue->isNocase() ? dlower : *psdstring;
	for(const auto& si: strings) {
		if(si.empty())
			continue;
		string::size_type i = 0;
		while((i = Bdcpp::strfnd(d, si, i)) != string::npos)
			psdstring->erase(i, si.size());
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::delimit) {
	const auto& strings = pValue->getStringList();
	if(strings.empty())
		return false;

	const string& element = *strings.begin();
	size_t elen = strings.begin()->size();

	if(elen == 0)
		return false;

	const auto&	int32s	= pValue->getInt32List();
	auto		pbegin	= psdstring->c_str();
	size_t		plen	= psdstring->size();
	auto		pend	= pbegin + plen;
	auto		ebegin	= element.c_str();
	int			type	= int32s.empty() ? Bdcpp::DELIMIT_ONLY : *int32s.begin();
	auto		i		= (pValue->isNocase() || (type & Bdcpp::DELIMIT_NOCASE)) ? Bdcpp::strfnd(Text::toLower(*psdstring).c_str(), plen, ((type & Bdcpp::DELIMIT_NOCASE) ? Text::toLower(element).c_str() : ebegin), elen) : Bdcpp::strfnd(pbegin, pend, ebegin, elen);

	if(i == string::npos)
		return false;

	const string& delimleft = (strings.size() > 1) ? *(strings.begin()+1) : Util::emptyString;
	const string& delimright = (strings.size() > 2) ? *(strings.begin()+2) : ((strings.size() > 1) ? *(strings.begin()+1) : Util::emptyString);

	if((type & Bdcpp::DELIMIT_ONLY) || (delimleft.empty() && delimright.empty())) { // no delimiters is synonym for DELIMIT_ONLY
		psdstring->assign(pbegin+i, elen);
		psdstring->updateInitial(i, elen); // update the initial positions
		return true;
	}

	auto	e		= i;
	size_t	len		= elen;
	bool	valid	= (type & Bdcpp::DELIMIT_CONTAINS);

	if(!delimleft.empty()) {
		if((i = Bdcpp::strfndlstof(pbegin, e, &(*delimleft.begin()), &(*delimleft.end()))) == string::npos)
				{ i = 0;	} // no delimiter found, start pos is beginning of data
		else	{ i++;		} // we don't add the found delimiter
	}

	if(!delimright.empty()) { // we using e + elen as a reference point to search from
		if((len = Bdcpp::strfndfstof(pbegin+e+elen, pend, &(*delimright.begin()), &(*delimright.end()))) == string::npos)
				{ len = plen - i;		} // no delimiter found, end pos is end of data
		else	{ len += e + elen - i;	} // update len accordingly with the starting position e + elen
	}

	valid |= (type & Bdcpp::DELIMIT_EQUALS) && (i == e) && (len == elen);
	valid |= (type & Bdcpp::DELIMIT_STARTS) && (i == e);
	valid |= (type & Bdcpp::DELIMIT_ENDS) && ((i+len) == (e+elen));
	
	if(valid) {
		psdstring->assign(pbegin+i, len); // update the data (part)
		psdstring->updateInitial(i, len); // update the initial positions
		return true;
	}

	return false;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::format) {
	const auto& sdexs = pValue->getSdExList();
	if(sdexs.empty())
		return true;
	const SdEx& fmt = *sdexs.begin();
	if(!psdstring->getName().empty()) // add this data for the formatting, if it is named
		(*pparams)[psdstring->getName()] = (const string&)(psdstring);
	*psdstring = fmt->format(*pparams);
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::insert) {
	const auto& strings = pValue->getStringList();
	if(strings.empty())
		return true;
	const auto&		int32s	= pValue->getInt32List();
	const string&	ins		= *strings.begin();
	size_t pos;
	{
		int i = (int32s.empty() ? 0 : *int32s.begin());
		if(i > int(psdstring->size()))	{ pos = psdstring->size();					}
		else if(i < 0)					{ pos = size_t(int(psdstring->size()) + i);	}
		else							{ pos = size_t(i);							}
	}
	psdstring->insert(pos, ins);
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::replace) {
	const auto& strings = pValue->getStringList();
	for(const auto& si: strings)
		*psdstring = si;
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::resize) {
	const auto& int32s = pValue->getInt32List();
	size_t pos;
	{
		int i = (int32s.empty() ? 0 : *int32s.begin());
		if(i > int(psdstring->size()))	{ pos = psdstring->size();					}
		else if(i < 0)					{ pos = size_t(int(psdstring->size()) + i);	}
		else							{ pos = size_t(i);							}
	}
	const size_t len = (int32s.size() > 1) ? ((*(int32s.begin()+1) == -1) ? psdstring->size()-pos : std::min(size_t(*(int32s.begin()+1)), psdstring->size()-pos)) : psdstring->size()-pos;
	*psdstring = psdstring->substr(pos, len);
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::split) {
	const auto& psdexs = pValue->getSdExList();
	if(psdexs.size() < 2)
		return false;

	const auto& int32s = pValue->getInt32List();
	size_t pos;
	{
		int i = (int32s.empty() ? 0 : *int32s.begin());
		if(i > int(psdstring->size()))	{ pos = psdstring->size();					}
		else if(i < 0)					{ pos = size_t(int(psdstring->size()) + i);	}
		else							{ pos = size_t(i);							}
	}

	if((pos == string::npos) || (pos > psdstring->size()))
		return false; // do not act on positions out of scope

	const auto&		strings	= pValue->getStringList();
	const string&	token	= strings.empty() ? Util::emptyString : *strings.begin();
	SdExString		one;
	SdExString		two;

	if(token.empty()) { // no token, split at the pos
		one.assign(psdstring->c_str(), pos);
		if(pos != psdstring->size())
			two.assign(psdstring->begin()+pos, psdstring->end());
	} else { // search for token, from position pos
		pos = pValue->isNocase() ? Bdcpp::strfnd(Text::toLower(*psdstring), token, pos) : Bdcpp::strfnd(*psdstring, token, pos);
		if(pos == string::npos)
			return false; // not found, not valid
		if(pos != 0)
			one.assign(psdstring->c_str(), pos);
		pos += token.size();
		if(pos != psdstring->size())
			two.assign(psdstring->begin()+pos, psdstring->end());
	}

	const SdEx&	part1		= *psdexs.begin();
	const SdEx&	part2		= *(psdexs.begin()+1);
	ParamMap	tmpParams	= *pparams;

	if(!part1->check(one, &tmpParams)) { return false; }
	if(!part2->check(two, &tmpParams)) { return false; }
	const_cast<ParamMap&>(*pparams) = std::forward<ParamMap>(tmpParams);
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::strip) {
	const auto&		strings	= pValue->getStringList();
	const string	dlower	= pValue->isNocase() ? Text::toLower(*psdstring) : Util::emptyString;
	const string&	d		= pValue->isNocase() ? dlower : *psdstring;
	for(const auto& si: strings) {
		string::size_type i = 0;
		if(si.empty()) { // default is whitespace
			while((i = Bdcpp::strfndfstof(d, " \r\n\t", i)) != string::npos)
				psdstring->erase(i, 1);
			continue;
		}
		while((i = Bdcpp::strfndfstof(d, si, i)) != string::npos)
			psdstring->erase(i, 1);
	}
	return true;
}

/*	0017: getting example conditions

	This function really has no use outside of processing SdEx
	conditions for the example edit control in one of the settings
	dialogs.

	It's not 100% correct but since it's for an example anyway
	there is no real need to go to extreme lengths for perfect
	validation of the conditions.

	What getExample does NOT take into account:
	- starting positions
	- multiple values with operators AND OR XOR XNOR NAND NOR NXOR NXNOR
	- mulitple conditional tags in a parameter
*/
SDEX_METHOD_FUNCTION(SdExFull::Function::example_consists) {
	const auto& strings = pValue->getStringList();
	if(strings.empty())
		return true;
	const string& first = *strings.begin();
	string::size_type i = 0;
	if(ops & OP_NOT) {
		char c = 0x20;
		while(Bdcpp::strfnd(&(*first.begin()), &(*first.end()), &c, 1) != string::npos)
			c++;
		*psdstring += c;
	} else {
		while((i = Bdcpp::strfndfstnof((ops & OP_NOCASE) ? Text::toLower(*psdstring) : *psdstring, first, i)) != string::npos)
			psdstring->erase(i, 1);
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_contains) {
	const auto& strings = pValue->getStringList();
	if(strings.empty())
		return true;
	const string& first = *strings.begin();
	if(ops & OP_NOT) {
		string::size_type i = 0;
		while((i = Bdcpp::strfndfstof((ops & OP_NOCASE) ? Text::toLower(*psdstring) : *psdstring, first, i)) != string::npos)
			psdstring->erase(i, 1);
	} else {
		if(Bdcpp::strfndfstof((ops & OP_NOCASE) ? Text::toLower(*psdstring) : *psdstring, first) == string::npos)
			*psdstring += first;
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_data) {
	const auto& strings = pValue->getStringList();
	if(strings.empty())
		return true;
	const string& first = *strings.begin();
	if(ops & OP_NOT) {
		if(((ops & OP_NOCASE) ? Text::toLower(*psdstring) : *psdstring) == first)
			*psdstring += 'c';
	} else {
		*psdstring = first;
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_find) {
	const auto& strings = pValue->getStringList();
	if(strings.empty())
		return true;
	const string& first = *strings.begin();
	if(ops & OP_NOT) {
		string::size_type i = Bdcpp::strfnd((ops & OP_NOCASE) ? Text::toLower(*psdstring) : *psdstring, first);
		if(i != string::npos)
			psdstring->erase(i, first.size());
	} else {
		if(Bdcpp::strfnd((ops & OP_NOCASE) ? Text::toLower(*psdstring) : *psdstring, first) == string::npos)
			*psdstring += first;
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_ifelse) { // initial behavior same as example_pattern
	return example_pattern(pValue, psdstring, pparams, ops);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_length) {
	const auto& int32s = pValue->getInt32List();
	if(int32s.empty())
		return true;
	const size_t first = size_t(*int32s.begin());
	if(ops & OP_NOT) {
		// this is not correct for ranges: 4,7
		if(psdstring->size() == first)
			*psdstring += 'i';
	} else {
		while(psdstring->size() != first) {
			if(psdstring->size() > first)	{ psdstring->erase(0, 1);	}
			else							{ *psdstring += 'i';			}
		}
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_number32) {
	const auto& int32s = pValue->getInt32List();
	if(int32s.empty())
		return true;
	int32_t first = *int32s.begin();
	if(ops & OP_NOT) {
		*psdstring = Util::toString(first + 1700); // arbitrary number
	} else {
		*psdstring = Util::toString(first);
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_number64) {
	const auto& int64s = pValue->getInt64List();
	if(int64s.empty())
		return true;
	int64_t first = *int64s.begin();
	if(ops & OP_NOT) {
		*psdstring = Util::toString(first + 1700);
	} else {
		*psdstring = Util::toString(first);
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_pattern) {
	const auto& psdexs = pValue->getSdExList();
	if(psdexs.empty())
		return true;
	const SdEx& first = *psdexs.begin();
	if(ops & OP_NOT) {
		if(psdstring->empty())
			*psdstring = "testdata";
		if(first->check(*psdstring, pparams))
			*psdstring += "test" + *psdstring + "test";
	} else {
		StringPairList tmp;
		first->get_example(tmp, *pparams);
		for(auto& sp: tmp)
			(*pparams)[sp.first] = sp.second;
		*psdstring = first->format(*pparams);
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_validate) {
	const auto& strings = pValue->getStringList();
	if(strings.empty())
		return true;
	const auto&		psdexs	= pValue->getSdExList();
	const string&	tname	= *strings.begin();
	SdEx			sdex;
	if(!psdexs.empty())
		sdex = *psdexs.begin();
	if((tname == "self") || (tname == "3rdperson") || (tname == "pm")) {
		auto pmi = pparams->find(tname);
		if(ops & OP_NOT) {
			if(pmi != pparams->end()) {
				const string& d = boost::apply_visitor(GetString(), pmi->second);
				if(sdex->check(d, pparams))
						return false;
			}
		} else {
			if(pmi == pparams->end())
				return false;
			const string& d = boost::apply_visitor(GetString(), pmi->second);
			if(!sdex->check(d, pparams))
				return false;
		}
	} else if(!tname.empty()) {
		if(ops & OP_NOT) {
			(*pparams)[tname] = sdex->format(*pparams) + "test";
		} else {
			(*pparams)[tname] = sdex->format(*pparams);
		}
	}
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_append) {
	return append(pValue, psdstring, pparams, ops);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_copy) {
	return copy(pValue, psdstring, pparams, ops);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_delete) {
	return deletedata(pValue, psdstring, pparams, ops);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_delimit) {
	const auto& strings = pValue->getStringList();
	if(strings.empty())
		return true;
	if(strings.size() > 1)
		*psdstring += *(strings.begin()+1);
	*psdstring += *strings.begin();
	if(strings.size() > 2)
		*psdstring += *(strings.begin()+2);
	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_format) {
	return format(pValue, psdstring, pparams, ops);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_insert) {
	return insert(pValue, psdstring, pparams, ops);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_replace) {
	return replace(pValue, psdstring, pparams, ops);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_resize) {
	return resize(pValue, psdstring, pparams, ops);
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_split) {
	const auto& psdexs = pValue->getSdExList();
	if(psdexs.size() < 2)
		return true;

	const SdEx& first = *psdexs.begin();
	StringPairList tmp;

	if(!first->get_example(tmp, *pparams))
		return false;
	for(auto& sp: tmp)
		(*pparams)[sp.first] = sp.second;
	*psdstring += first->format(*pparams);

	const auto&		strings	= pValue->getStringList();
	const string&	token	= (strings.empty()) ? Util::emptyString : *strings.begin();
	const SdEx&		second	= *(psdexs.begin()+1);

	*psdstring += token;

	tmp.clear();
	if(!second->get_example(tmp, *pparams))
		return false;
	for(auto& sp: tmp)
		(*pparams)[sp.first] = sp.second;
	*psdstring += second->format(*pparams);

	return true;
}

SDEX_METHOD_FUNCTION(SdExFull::Function::example_strip) {
	return strip(pValue, psdstring, pparams, ops);
}

}
