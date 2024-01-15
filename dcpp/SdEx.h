
/*	0017: SdEx.h
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

#ifndef BDCPLUSPLUS_SDEX_H
#define BDCPLUSPLUS_SDEX_H

#include "debug.h"
#include "typedefs.h"

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace dcpp {

// SdExString
// a string class with added members for parameters and position in a larger string
class SdExString : public string {
public:
	typedef vector<SdExString> List;

	SdExString(const string& aData = "", string::size_type aInitialPos = 0, bool aParam = false, const string& aName = "") : string(aData), ipos(aInitialPos), ilen(aData.size()), param(aParam), name(aName) { }
	SdExString(const string::value_type* pbegin, const string::value_type* pend, string::size_type aInitialPos = 0, bool aParam = false, const string& aName = "") : string(pbegin, pend), ipos(aInitialPos), ilen(pend-pbegin), param(aParam), name(aName) { }
	SdExString(const string::value_type* pbegin, size_t plen, string::size_type aInitialPos = 0, bool aParam = false, const string& aName = "") : string(pbegin, plen), ipos(aInitialPos), ilen(plen), param(aParam), name(aName) { }
	SdExString(const SdExString& rhs) : string(rhs), ipos(rhs.ipos), ilen(rhs.ilen), param(rhs.param), name(rhs.name) { }
	SdExString& operator=(const SdExString& rhs) { (string&)(*this) = (const string&)(rhs); ipos = rhs.ipos; ilen = rhs.ilen; param = rhs.param; name = rhs.name; return *this; }
	string& operator=(const string& aData) { (string&)(*this) = aData; return (string&)(*this); }
	~SdExString() { }

	const string::size_type& getInitialPos() const { return ipos; }
	const size_t& getInitialLength() const { return ilen; }
	bool isParam() const { return param; }
	const string& getName() const { return name; }

	void updateInitial(string::size_type addPos, size_t newLength) { ipos += addPos; ilen = newLength; }

private:
	string::size_type ipos; // position in original line
	size_t ilen; // length in original line
	bool param; // is or is not a parameter
	string name; // for parameters
};

// SdExFunctionBase
// use this base to retrieve certain function members from the SdExBase derivation
// non-abstract base to make reference copies possible
class SdExFunctionBase {
public:
	typedef vector<SdExFunctionBase> List;

	enum {
		OP_NONE				= 0,
		OP_AND				= 0x001,				// AND			all values true
		OP_OR				= 0x002,				// OR			any value true
		OP_XOR				= 0x004,				// XOR			only one value true
		OP_XNOR				= 0x008,				// XNOR			all values true or all values false
		OP_NOT				= 0x010,				// NOT			invert true/false
		OP_NOCASE			= 0x020,				// NOCASE		not case sensitive
		OP_FORCECASE		= 0x040,				// FORCECASE	always case sensitive
		OP_NAND				= OP_AND	| OP_NOT,	// AND			all values false
		OP_NOR				= OP_OR		| OP_NOT,	// NOR			any value false
		OP_NXOR				= OP_XOR	| OP_NOT,	// NXOR			only one value false
		OP_NXNOR			= OP_XNOR	| OP_NOT,	// NXNOR		all values false or all values true (technically the same as XNOR)
		// special flag for system tags that require a different operator (: instead of =)
		OP_SYSTEM			= 0x100,
		// keep the cases as they are in components 1 - 4
		CPNT_KEEPCASE		= 1, // keep cases
		CPNT1_KEEPCASE		= CPNT_KEEPCASE << 12,
		CPNT2_KEEPCASE		= CPNT_KEEPCASE << 13,
		CPNT3_KEEPCASE		= CPNT_KEEPCASE << 14,
		CPNT4_KEEPCASE		= CPNT_KEEPCASE << 15,
		// component type spread for components 1 - 4, after that everything is handled as type string, default is type string (unmodified)
		CPNT_INT32			= 1, // type int
		CPNT_INT64			= 2, // type int64
		CPNT_STRING			= 3, // type string
		CPNT_SDEX			= 4, // type SdEx
		// component 1
		CPNT1_INT32			= CPNT_INT32 << 16,
		CPNT1_INT64			= CPNT_INT64 << 16,
		CPNT1_STRING		= CPNT_STRING << 16,
		CPNT1_SDEX			= CPNT_SDEX << 16,
		// component 2
		CPNT2_INT32			= CPNT_INT32 << 20,
		CPNT2_INT64			= CPNT_INT64 << 20,
		CPNT2_STRING		= CPNT_STRING << 20,
		CPNT2_SDEX			= CPNT_SDEX << 20,
		// component 3
		CPNT3_INT32			= CPNT_INT32 << 24,
		CPNT3_INT64			= CPNT_INT64 << 24,
		CPNT3_STRING		= CPNT_STRING << 24,
		CPNT3_SDEX			= CPNT_SDEX << 24,
		// component 4
		CPNT4_INT32			= CPNT_INT32 << 28,
		CPNT4_INT64			= CPNT_INT64 << 28,
		CPNT4_STRING		= CPNT_STRING << 28,
		CPNT4_SDEX			= CPNT_SDEX << 28,
		// combos
		CPNT12_INT32		= CPNT1_INT32		| CPNT2_INT32,
		CPNT12_INT64		= CPNT1_INT64		| CPNT2_INT64,
		CPNT123_SDEX		= CPNT1_SDEX		| CPNT2_SDEX		| CPNT3_SDEX,
		CPNT1234_KEEPCASE	= CPNT1_KEEPCASE	| CPNT2_KEEPCASE	| CPNT3_KEEPCASE	| CPNT4_KEEPCASE,
		CPNT1234_SDEX		= CPNT1_SDEX		| CPNT2_SDEX		| CPNT3_SDEX		| CPNT4_SDEX,
		CPNT_INSERT			= CPNT1_KEEPCASE	| CPNT2_INT32		| CPNT3_KEEPCASE	| CPNT4_KEEPCASE,
		CPNT_SPLIT			= CPNT1_SDEX		| CPNT2_SDEX		| CPNT3_INT32,
		CPNT_VALIDATE		= CPNT1_KEEPCASE	| CPNT2_SDEX		| CPNT3_SDEX		| CPNT4_SDEX
	};

	SdExFunctionBase(const string& aCmd, int aComponents, const string& aDescription) : cmd(aCmd), components(aComponents), description(aDescription) { }
	SdExFunctionBase(const SdExFunctionBase& rhs) : cmd(rhs.cmd), components(rhs.components), description(rhs.description) { }
	virtual ~SdExFunctionBase() { }

	// API member variables
	const string cmd;
	const int components;
	const string description;
};

// SdExBase
class SdExBase {
public:
	friend class SdEx; // let SdEx be able to access the protected members as well

	enum {
		EXAMPLE		= 0x01,
		FUNCTIONS	= 0x02,
		RESULT		= 0x04
	};

	SdExBase(int aSupports) : nocase(false), readescapes(true), supports(aSupports) { }
	SdExBase(const SdExBase& rhs) : pattern(rhs.pattern), nocase(rhs.nocase), readescapes(rhs.readescapes), supports(rhs.supports) { }
	virtual ~SdExBase() { }

	bool supportsExample() const { return (supports & EXAMPLE); }
	bool supportsFunctions() const { return (supports & FUNCTIONS); }
	bool supportsResult() const { return (supports & RESULT); }

	// API pattern usage (must-haves for derived classes)
	virtual bool check(SdExString::List& sdstrings, const string::value_type* pbegin, const string::value_type* pend, ParamMap* params = nullptr) const = 0;
	virtual bool format(SdExString::List& sdstrings, ParamMap& params, bool ignoreparamfind, bool ignoretags, bool removeparam) const = 0;
	// API pattern modifying
	virtual void append(const string::value_type* pbegin, const string::value_type* pend) { pattern.append(pbegin, pend); }
	virtual void assign(const string::value_type* pbegin, const string::value_type* pend, bool aNocase = false, bool aReadEscapes = true) { pattern.assign(pbegin, pend); nocase = aNocase; readescapes = aReadEscapes; }
	virtual void clear() { pattern.clear(); }
	// API utilities
	virtual string get_data(const string::value_type* pbegin, const string::value_type* pend, bool nocase = false, bool readescapes = true, bool strip = true) const;
	virtual string& escape_data(string& data) const;
	virtual bool get_example(StringPairList& list, ParamMap& params) const { return false; }
	virtual void get_functions(SdExFunctionBase::List& sdexfunctions) const { }
	virtual string get_result() const { return ""; }

	// overloads
	virtual bool check(const string& aLine, ParamMap* params = nullptr) const {
		SdExString::List sdstrings;
		return check(sdstrings, &(*aLine.begin()), &(*aLine.end()), params);
	}

	virtual string format(ParamMap& params) const {
		SdExString::List sdstrings;
		string f;
		format(sdstrings, params, true, false, false);
		for(const auto& si: sdstrings)
			f += (const string&)(si);
		return f;
	}

protected: // the derived class should be able to access these members
	string pattern;
	bool nocase;
	bool readescapes;
	const int supports;
};

// SdEx
// the SdEx object is a wrapper/smart pointer for the actual SdEx processing interface
class SdEx {
public:
	// SdExParameters
	static StringList getParamsTabText();
	static StringList getParamsTextFormat();
	static StringList getParamsWinamp();

	// only update when the base is not in use
	typedef boost::shared_lock<boost::shared_mutex> UseLock;
	typedef boost::unique_lock<boost::shared_mutex> UpdateLock;

	SdEx(const string& aPattern = "", bool aNocase = false, bool aReadEscapes = true);
	SdEx(const string::value_type* pbegin, const string::value_type* pend, bool aNocase = false, bool aReadEscapes = true);
	SdEx(const SdEx& rhs);
	~SdEx();

	// operators
	SdEx& operator=(const SdEx& rhs) { copy(rhs); return *this; }
	SdExBase* operator->() { return getBaseSafe(); } // use SdEx as a pointer instead of a reference to access base members
	const SdExBase* operator->() const { return getBaseSafe(); }

	void update();

	const string& getPattern() const { return getBaseSafe()->pattern; }

private:
	SdExBase* base;
	mutable boost::shared_mutex mutex; // mutex for base modification

	SdExBase* getBaseSafe() { if(base == nullptr) { createInstance(); } dcassert(base != nullptr); UseLock lock(mutex); return base; }
	const SdExBase* getBaseSafe() const { dcassert(base != nullptr); UseLock lock(mutex); return base; }
	void copy(const SdEx& rhs) { copyInstance(rhs.getBaseSafe()); }

	void createInstance();
	void copyInstance(const SdExBase* aBase);
	void deleteInstance();
};

} // namespace dcpp

#endif // !defined(BDCPLUSPLUS_SDEX_H)
