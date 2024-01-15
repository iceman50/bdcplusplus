
/*	0017: SdExFull.h
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

#ifndef DCPLUSPLUS_SDEX_FULL_H
#define DCPLUSPLUS_SDEX_FULL_H

#include "SdEx.h"
#include "typedefs.h"

namespace dcpp {

class SdExFull : public SdExBase {
public:
	SdExFull() : SdExBase(EXAMPLE | FUNCTIONS | RESULT) { }
	SdExFull(const SdExFull& rhs) : SdExBase(rhs), parts(rhs.parts), result(rhs.result) { }
	~SdExFull() { }

	// pattern usage
	bool check(SdExString::List& sdstrings, const string::value_type* pbegin, const string::value_type* pend, ParamMap* params = nullptr) const;
	bool format(SdExString::List& sdstrings, ParamMap& params, bool ignoreparamfind, bool ignoretags, bool removeparam) const;
	// pattern modifying
	void append(const string::value_type* pbegin, const string::value_type* pend);
	void assign(const string::value_type* pbegin, const string::value_type* pend, bool aNocase = false, bool aReadEscapes = true) { clear(); nocase = aNocase; readescapes = aReadEscapes; append(pbegin, pend); }
	void clear() { pattern.clear(); parts.clear(); result.clear(); }
	// utilities
	// no need to overload these, the default implementation is fine
//	string get_data(const string::value_type* pbegin, const string::value_type* pend, bool nocase = false, bool readescapes = true, bool strip = true) const;
//	string& escape_data(string& data) const;
	bool get_example(StringPairList& list, ParamMap& params) const;
	void get_functions(SdExFunctionBase::List& sdexfunctions) const;
	string get_result() const;

	// overloads
//	virtual bool check(const string& aLine, ParamMap* params = NULL) const;
//	virtual string format(ParamMap& params) const;

private:
	friend class SdEx; // only used for a few external tools

	// copied from Util.cpp, needed for retrieving ParamMap second
	struct GetString : boost::static_visitor<string> {
		string operator()(const string& s) const { return s; }
		string operator()(const std::function<string ()>& f) const { return f(); }
	};

	class Value {
	public:
		typedef vector<Value> List;

		Value(int aOps) : ops(aOps) { }
		Value(const Value& rhs) : int32s(rhs.int32s), int64s(rhs.int64s), strings(rhs.strings), sdexs(rhs.sdexs), ops(rhs.ops) { }
		Value& operator=(const Value& rhs) { int32s = rhs.int32s; int64s = rhs.int64s; strings = rhs.strings; sdexs = rhs.sdexs; ops = rhs.ops; return *this; }
		~Value() { }

		const vector<int32_t>& getInt32List() const { return int32s; }
		const vector<int64_t>& getInt64List() const { return int64s; }
		const vector<string>& getStringList() const { return strings; }
		const vector<SdEx>& getSdExList() const { return sdexs; }

		bool isNocase(int index = -1) const { return ((index < 0) || (index > 3) || !((ops >> (12 + index)) & SdExFunctionBase::CPNT_KEEPCASE)) && (ops & SdExFunctionBase::OP_NOCASE); }

		void addInt32(int32_t val) { int32s.emplace_back(val); }
		void addInt64(int64_t val) { int64s.emplace_back(val); }
		void addString(const string& aString) { strings.emplace_back(aString); }
		void addSdEx(const SdEx& sdex) { sdexs.emplace_back(sdex); }

	private:
		vector<int32_t> int32s;
		vector<int64_t> int64s;
		vector<string> strings;
		vector<SdEx> sdexs;
		int ops;
	};

	class Function : public SdExFunctionBase {
	public:
#define SDEX_METHOD_FUNCTION(func) bool func(const Value* pValue, SdExString* psdstring, ParamMap* pparams, int ops) const

		enum { FUNCTION_LAST = 20 };

		typedef bool (Function::*Func)(const Value*, SdExString*, ParamMap*, int) const;

		Function(const string& aCmd, const string& aCmdShort, Func aFunction, int aComponents, Func aExample, const string& aDescription);
		Function(const Function& rhs) : SdExFunctionBase(rhs), cmdshort(rhs.cmdshort), function(rhs.function), example(rhs.example) { }
		~Function() { }

		const string cmdshort;
		Func function;
		Func example;

		SDEX_METHOD_FUNCTION(runFunction) { return (function ? (this->*function)(pValue, psdstring, pparams, ops | components) : false); }
		SDEX_METHOD_FUNCTION(runExample) { return (example ? (this->*example)(pValue, psdstring, pparams, ops | components) : false); }

		SDEX_METHOD_FUNCTION(consists);
		SDEX_METHOD_FUNCTION(contains);
		SDEX_METHOD_FUNCTION(data);
		SDEX_METHOD_FUNCTION(find);
		SDEX_METHOD_FUNCTION(ifelse);
		SDEX_METHOD_FUNCTION(length);
		SDEX_METHOD_FUNCTION(number32);
		SDEX_METHOD_FUNCTION(number64);
		SDEX_METHOD_FUNCTION(pattern);
		SDEX_METHOD_FUNCTION(validate);

		SDEX_METHOD_FUNCTION(append);
		SDEX_METHOD_FUNCTION(copy);
		SDEX_METHOD_FUNCTION(deletedata);
		SDEX_METHOD_FUNCTION(delimit);
		SDEX_METHOD_FUNCTION(format);
		SDEX_METHOD_FUNCTION(insert);
		SDEX_METHOD_FUNCTION(replace);
		SDEX_METHOD_FUNCTION(resize);
		SDEX_METHOD_FUNCTION(split);
		SDEX_METHOD_FUNCTION(strip);

		SDEX_METHOD_FUNCTION(example_consists);
		SDEX_METHOD_FUNCTION(example_contains);
		SDEX_METHOD_FUNCTION(example_data);
		SDEX_METHOD_FUNCTION(example_find);
		SDEX_METHOD_FUNCTION(example_ifelse);
		SDEX_METHOD_FUNCTION(example_length);
		SDEX_METHOD_FUNCTION(example_number32);
		SDEX_METHOD_FUNCTION(example_number64);
		SDEX_METHOD_FUNCTION(example_pattern);
		SDEX_METHOD_FUNCTION(example_validate);

		SDEX_METHOD_FUNCTION(example_append);
		SDEX_METHOD_FUNCTION(example_copy);
		SDEX_METHOD_FUNCTION(example_delete);
		SDEX_METHOD_FUNCTION(example_delimit);
		SDEX_METHOD_FUNCTION(example_format);
		SDEX_METHOD_FUNCTION(example_insert);
		SDEX_METHOD_FUNCTION(example_replace);
		SDEX_METHOD_FUNCTION(example_resize);
		SDEX_METHOD_FUNCTION(example_split);
		SDEX_METHOD_FUNCTION(example_strip);
	};

	class Tag {
	public:
		Tag() : function(nullptr), ops(0) { }
		Tag(const Function* pFunction, int aOps) : function(pFunction), ops(aOps) { }
		Tag(const Tag& rhs) : function(rhs.function), ops(rhs.ops) { }
		Tag& operator=(const Tag& rhs) { function = rhs.function; ops = rhs.ops; return *this; }
		~Tag() { }

		bool check(const Value* pValue, SdExString* psdstring, ParamMap* pparams) const { return (function ? function->runFunction(pValue, psdstring, pparams, ops) : false); }
		bool example(const Value* pValue, SdExString* psdstring, ParamMap* pparams) const { return (function ? function->runExample(pValue, psdstring, pparams, ops) : false); }
		int getOps() const { return (function ? (function->components | ops) : ops); }

		void setFunction(const Function* pFunction, int aOps) { function = pFunction; ops = aOps; }
		bool hasFunction() const { return (function != nullptr); }

	private:
		const Function* function;
		int ops;
	};

	typedef vector<pair<Tag, Value::List> > TagValueList;

	class Part : public string {
	public:
		typedef vector<Part> List;

		Part(const SdExBase* aBase, const string::value_type* pbegin, const string::value_type* pend, bool isParam, bool aNocase, bool aReadEscapes, StringList& result);
		Part(const Part& rhs) : string(rhs), tags(rhs.tags), paramlenfixed(rhs.paramlenfixed), paramlenmin(rhs.paramlenmin), param(rhs.param), nocase(rhs.nocase) { }
		Part& operator=(const Part& rhs) { (string&)(*this) = (const string&)(rhs); tags = rhs.tags; paramlenfixed = rhs.paramlenfixed; paramlenmin = rhs.paramlenmin; param = rhs.param; nocase = rhs.nocase; return *this; }
		string& operator=(const string& aData) { (string&)(*this) = aData; return (string&)(*this); }
		~Part() { }

		bool checkTags(SdExString& sdstring, ParamMap& params) const;

		const TagValueList& getTags() const { return tags; }
		const string::size_type& getParamLengthFixed() const { return paramlenfixed; }
		const string::size_type& getParamLengthMinimum() const { return paramlenmin; }
		bool isParam() const { return param; }
		bool isNocase() const { return nocase; }

		bool hasTags() const { return !tags.empty(); }
		bool hasParamLengthFixed() const { return (paramlenfixed != string::npos); }
		bool hasParamLengthMinimum() const { return (paramlenmin != string::npos); }

	private:
		TagValueList tags;
		string::size_type paramlenfixed;
		string::size_type paramlenmin;
		bool param;
		bool nocase;
	};

	class Tokenizer {
	public:
		Tokenizer(const SdExBase* aBase, const string::value_type* pbegin, const string::value_type* pend, int ops, bool partNocase, StringList& result);
		~Tokenizer() { }

		const Value::List& getTokens() const { return tokens; }

	private:
		Value::List tokens;
		const SdExBase* base;

		void addValue(const string::value_type* pbegin, const string::value_type* pend, int aOps, StringList& result);
	};

	static Function functions[Function::FUNCTION_LAST];
	Part::List parts;
	StringList result;

	// SdEx data handling
	static int32_t getInt32(const SdExBase* aBase, const string::value_type* pbegin, const string::value_type* pend);
	static int64_t getInt64(const SdExBase* aBase, const string::value_type* pbegin, const string::value_type* pend);
	static bool readTag(const SdExBase* aBase, const string::value_type* pbegin, const string::value_type* pend, Tag* ptag, StringList& result);
	static bool readLegacyTag(const string::value_type* pbegin, const string::value_type* pend, Tag* ptag, int aOps, StringList& result);
	static const Function* getSdExFunction(const string::value_type* pbegin, const string::value_type* pend, bool shortcmd = false);
	static void addResult(StringList& result, const string& aText);
	// SdEx data searching
	static const string::value_type* findParamBegin(const string::value_type* pbegin, const string::value_type* pend, bool readescapes, StringList& result);
	static const string::value_type* findParamEnd(const string::value_type* pbegin, const string::value_type* pend, bool readescapes, StringList& result);
	static const string::value_type* findTagBegin(const string::value_type* pbegin, const string::value_type* pend, bool readescapes, StringList& result);
	static const string::value_type* findTagEnd(const string::value_type* pbegin, const string::value_type* pend, bool readescapes, StringList& result);
	static const string::value_type* findSeparator(const string::value_type* pbegin, const string::value_type* pend, const string::value_type& separator, bool readescapes, StringList& result);
	// forwards
	static string::size_type strcmp(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend, bool nocase);
	static string::size_type strfnd(const string::value_type* pbegin, const string::value_type* pend, const string::value_type* dbegin, const string::value_type* dend, bool nocase);
};

}

#endif // !defined(DCPLUSPLUS_SDEX_FULL_H)
