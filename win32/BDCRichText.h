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

#ifndef DCPLUSPLUS_WIN32_BDCPP_RICH_TEXT_H
#define DCPLUSPLUS_WIN32_BDCPP_RICH_TEXT_H

#include <dcpp/FastAlloc.h>
#include <dcpp/Flags.h>
#include <dcpp/Bdcpp.h>
#include <dcpp/SimpleXML.h>
#include <dcpp/Text.h>

#include <dwt/Point.h>

#include "forward.h"

#include <RichOle.h>

#include <type_traits>

#ifndef OLERENDER_FORMAT
#define OLERENDER_FORMAT 2
#endif

class RichTextBox;

class BDCRichText : private SimpleXMLReader::CallBack {
public:
	static void addChatHtml(RichTextBox* aRichText, const string& aHtml);
	static void addChatPlain(RichTextBox* aRichText, const tstring& aLine);
	static void addSystemLog(RichTextBox* aRichText, const LogMessage& logMsg);
	static void addExamples(RichTextBox* aRichText, const TextFormat& aFormat, int type); // Do we really need this?

private:
	enum State {
		STATE_DECLARATION,
		STATE_FONT,
		STATE_TEXTCOLOR,
		STATE_BGCOLOR,
		STATE_TEXTDECORATION,
		STATE_TEXTTRANSFORM,
		STATE_BORDER,
		STATE_BORDERBOTTOM,
		STATE_BORDERBOTTOMSTYLE,
		STATE_BORDERBOTTOMWIDTH,
		STATE_UNKNOWN
	};

	class Context : public Flags, public FastAlloc<Context> {
	public:

// getRtf size_t compare old with *this
#define CONTEXT_RTFSIZE(has, member, add) \
	if(isSet(has)) { \
		if(old.isSet(has)) { \
			if(member != old.member) \
				context += add + Util::toString(int(member)); \
		} else { \
			context += add + Util::toString(int(member)); \
		} \
	}

// getRtf bool compare old with *this
// only unset if old was set and true
#define CONTEXT_RTFBOOL(has, member, on, off) \
	if(isSet(has)) { \
		if(old.isSet(has)) { \
			if(member != old.member) \
				context += member ? on : off; \
		} else { \
			if(member) \
				context += on; \
		} \
	} else { \
		if(old.isSet(has)) \
			context += off; \
	}

// fillContext check and add
#define CONTEXT_FILL(has, member) \
	if(!context.isSet(has) && isSet(has)) { \
		context.member = member; \
		context.setFlag(has); \
	}

// fillcontext check and add underline type
#define CONTEXT_FILLUNDERLINE(has) \
	if(!context.isSet(has) && isSet(has)) { \
		context.underline |= (underline & has); \
		context.setFlag(has); \
	}

		Context(const string& aName = Util::emptyString) : name(aName),
			fontindex(-1), fontsize(-1), textcolorindex(-1), bgcolorindex(-1),
			bold(false), italic(false), strikeout(false), allcaps(false), subscript(false), superscript(false),
			underline(0) { }
		Context(const Context& rhs) : Flags(rhs), name(rhs.name), link(rhs.link), image(rhs.image),
			fontindex(rhs.fontindex), fontsize(rhs.fontsize), textcolorindex(rhs.textcolorindex), bgcolorindex(rhs.bgcolorindex),
			bold(rhs.bold), italic(rhs.italic), strikeout(rhs.strikeout), allcaps(rhs.allcaps), subscript(rhs.subscript), superscript(rhs.superscript),
			underline(rhs.underline) { }

		// get rtf tags relative to the previously used context
		string getRtf(const Context& old) const {
			string context;
			CONTEXT_RTFSIZE(FONTINDEX, fontindex, "\\f");
			CONTEXT_RTFSIZE(FONTSIZE, fontsize, "\\fs");
			CONTEXT_RTFSIZE(TEXTCOLORINDEX, textcolorindex, "\\cf");
			CONTEXT_RTFSIZE(BGCOLORINDEX, bgcolorindex, "\\highlight");
			CONTEXT_RTFBOOL(BOLD, bold, "\\b", "\\b0");
			CONTEXT_RTFBOOL(ITALIC, italic, "\\i", "\\i0");
			CONTEXT_RTFBOOL(STRIKEOUT, strikeout, "\\strike", "\\strike0");
			CONTEXT_RTFBOOL(ALLCAPS, allcaps, "\\caps", "\\caps0");

			if(isSet(SUPERSCRIPT) && superscript) {
				if(old.isSet(SUPERSCRIPT)) {
					if(!old.superscript)
						context += "\\super";
				} else {
					context += "\\super";
				}
			} else if(isSet(SUBSCRIPT) && subscript) {
				if(old.isSet(SUBSCRIPT)) {
					if (!old.subscript)
						context += "\\sub";
				} else {
					context += "\\sub";
				}
			} else {
				if(old.isSet(SUBSCRIPT) && old.subscript) {
					context += "\\nosupersub";
				} else if(old.isSet(SUPERSCRIPT) && old.superscript) {
					context += "\\nosupersub";
				}
			}

			// underlining is only visible if UNDERLINE is set to true
			// the other underline settings specify the type of underline, if underlining is visible
			if(isSet(UNDERLINE) && (underline & UNDERLINE)) {
				if(!old.isSet(UNDERLINE) || (old.isSet(UNDERLINE) && (underline != old.underline))) {
					if(isSet(UNDERLINE_THICK) && (underline & UNDERLINE_THICK)) { context += "\\ulth"; }
					else if(isSet(UNDERLINE_DASH) && (underline & UNDERLINE_DASH)) { context += "\\uldash"; }
					else if(isSet(UNDERLINE_DOT) && (underline & UNDERLINE_DOT)) { context += "\\uld"; }
					else if(isSet(UNDERLINE) && (underline & UNDERLINE)) { context += "\\ul"; }
				} 
			} else {
				if(old.isSet(UNDERLINE) && (old.underline & UNDERLINE))
					context += "\\ul0";
			}
			return context;
		}

		// fill non-set members of context with set members of *this
		void fillContext(Context& context) const {
			if(context.link.empty() && !link.empty())
				context.link = link;
			if(context.image.empty() && !image.empty())
				context.image = image;

			CONTEXT_FILL(FONTINDEX, fontindex);
			CONTEXT_FILL(FONTSIZE, fontsize);
			CONTEXT_FILL(TEXTCOLORINDEX, textcolorindex);
			CONTEXT_FILL(BGCOLORINDEX, bgcolorindex);
			CONTEXT_FILL(BOLD, bold);
			CONTEXT_FILL(ITALIC, italic);
			CONTEXT_FILL(STRIKEOUT, strikeout);
			CONTEXT_FILL(ALLCAPS, allcaps);
			CONTEXT_FILL(SUBSCRIPT, subscript);
			CONTEXT_FILL(SUPERSCRIPT, superscript);

			CONTEXT_FILLUNDERLINE(UNDERLINE);
			CONTEXT_FILLUNDERLINE(UNDERLINE_DOT);
			CONTEXT_FILLUNDERLINE(UNDERLINE_DASH);
			CONTEXT_FILLUNDERLINE(UNDERLINE_THICK);
		}

		void setLink(const string& aLink) { 
			link = aLink;
		}
		void setImage(const string& aImage) { 
			image = aImage;
		}
		void setFontIndex(size_t index) {
			fontindex = index;
			setFlag(FONTINDEX);
		}
		void setFontSize(size_t size) {
			fontsize = size;
			setFlag(FONTSIZE);
		}
		void setTextColorIndex(size_t index) {
			textcolorindex = index;
			setFlag(TEXTCOLORINDEX); 
		}
		void setBgColorIndex(size_t index) {
			bgcolorindex = index;
			setFlag(BGCOLORINDEX); 
		}
		void setBold(bool b) { 
			bold = b;
			setFlag(BOLD);
		}
		void setItalic(bool b) { 
			italic = b;		
			setFlag(ITALIC);
		}
		void setStrikeout(bool b) { 
			strikeout = b;
			setFlag(STRIKEOUT);
		}
		void setAllcaps(bool b) { 
			allcaps = b;
			setFlag(ALLCAPS);
		}
		void setSubscript(bool b) {
			subscript = b;	
			setFlag(SUBSCRIPT);
		}
		void setSuperscript(bool b) { 
			superscript = b;
			setFlag(SUPERSCRIPT);
		}

		void setUnderline(bool b) { 
			if (b) {
				underline |= UNDERLINE;
			} else { 
				underline &= ~UNDERLINE;
			}	
				setFlag(UNDERLINE); 
		}

		void setUnderlineDot(bool b) { 
			if (b) { 
				underline |= UNDERLINE_DOT; 
			} else {
				underline &= ~UNDERLINE_DOT; 
			}
			setFlag(UNDERLINE_DOT); 
		}

		void setUnderlineDash(bool b) {
			if (b) {
				underline |= UNDERLINE_DASH; 
			} else { 
				underline &= ~UNDERLINE_DASH; 
			}	
			setFlag(UNDERLINE_DASH); 
		}

		void setUnderlineThick(bool b) { 
			if (b) { 
				underline |= UNDERLINE_THICK; 
			} else {
				underline &= ~UNDERLINE_THICK; 
			}	
			setFlag(UNDERLINE_THICK); 
		}

		const string& getName() const { return name; }
		const string& getLink() const { return link; }
		const string& getImage() const { return image; }

	private:
		enum { // member validity flags
			FONTINDEX = 0x0001,
			FONTSIZE = 0x0002,
			TEXTCOLORINDEX = 0x0004,
			BGCOLORINDEX = 0x0008,
			BOLD = 0x0010,
			ITALIC = 0x0020,
			STRIKEOUT = 0x0040,
			ALLCAPS = 0x0080,
			SUBSCRIPT = 0x0100,
			SUPERSCRIPT = 0x0200,
			UNDERLINE = 0x0400,
			UNDERLINE_DOT = 0x0800,
			UNDERLINE_DASH = 0x1000,
			UNDERLINE_THICK = 0x2000,
			UNDERLINE_ANY = UNDERLINE | UNDERLINE_DOT | UNDERLINE_DASH | UNDERLINE_THICK
		};

		string name;
		string link;
		string image;
		size_t fontindex;
		size_t fontsize;
		size_t textcolorindex;
		size_t bgcolorindex;
		bool bold;
		bool italic;
		bool strikeout;
		bool allcaps;
		bool subscript;
		bool superscript;
		int underline;
	};

	class Part : public FastAlloc<Part> {
	public:
		Part(const Context& aContext) : context(aContext) { }
		Part(const Part& rhs) : data(rhs.data), context(rhs.context) { }

		void getRtf(tstring& buffer) const {
			if(empty()) // no text, nothing to add/show
				return;
			string c = context.getRtf(Context()); // compare against an empty dummy, we need all the valid rtf tags

			buffer += L'{';
			if(!context.getLink().empty())
				buffer += _T("\\field{\\*\\fldinst HYPERLINK \"") + Text::toT(context.getLink()) + _T("\"}{\\fldrslt ");
			if(!c.empty()) {
				buffer += Text::toT(c);
				buffer += L' ';
			}

			// richedit bug: if the link data is the same as the text data (without rtf codes), link formatting may get lost
			// add an invisible space to prevent this from happening
			if(!context.getLink().empty())
				buffer += _T("{\\v  }");

			buffer += data; // data is already rtf escaped

			if(!context.getLink().empty())
				buffer += L'}';
			buffer += '}';
		}

		void addData(const string& aData) { data += Text::toT(aData); }
		void addData(const tstring& aData) { data += aData; }
		bool isLink() const { return !context.getLink().empty(); }
		const string& getImage() const { return context.getImage(); }

	private:
		tstring data;
		Context context;

		bool empty() const { // check to see if this part actually contains any text to be shown
			auto pbegin = &(*data.begin());
			auto pend = &(*data.end());
			bool rtf = false;
			while (pbegin != pend) {
				if(rtf) {
					if(*pbegin == L' ')
						rtf = false;
				} else {
					if(*pbegin != L'\\') // either rtf or an escaped character
						return false;
					pbegin++;
					if(pbegin == pend) // weird but no data
						return true;
					if((*pbegin == L'\\') || (*pbegin == L'{') || (*pbegin == L'}')) // escaped character, we have data
						return false;
					rtf = true;
				}
				pbegin++;
			}
			return true;
		}
	};

	class ImageObject : IDataObject {
	public:
		ImageObject() : refcount(0) { }
		virtual ~ImageObject() { ::ReleaseStgMedium(&medium); }

		SCODE SetBitmap(HBITMAP hBitmap) {
			dcassert(hBitmap);
			STGMEDIUM stgm;
			stgm.tymed = TYMED_GDI;
			stgm.hBitmap = hBitmap;
			stgm.pUnkForRelease = NULL;
			FORMATETC fe;
			fe.cfFormat = CF_BITMAP;
			fe.ptd = NULL;
			fe.dwAspect = DVASPECT_CONTENT;
			fe.lindex = -1;
			fe.tymed = TYMED_GDI;
			return SetData(&fe, &stgm, TRUE);
		}

		SCODE CreateStorage(IStorage** ppStorage) {
			LPLOCKBYTES	lpLockBytes = NULL;
			SCODE sc;
			if((sc = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes)) != S_OK)
				return sc;
			if((sc = ::StgCreateDocfileOnILockBytes(lpLockBytes, STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, ppStorage)) != S_OK) {
				lpLockBytes->Release();
				return sc;
			}
			return S_OK;
		}

		SCODE GetOle(HWND hWnd, IStorage* pStorage, LPRICHEDITOLE* pRichEditOle, IOleClientSite** ppOleClientSite, IOleObject** ppOleObject) {
			dcassert(medium.hBitmap);
			SCODE sc;
			::SendMessage(hWnd, EM_GETOLEINTERFACE, 0, LPARAM(pRichEditOle));
			dcassert(pRichEditOle);
			if((sc = (*pRichEditOle)->GetClientSite(ppOleClientSite)) != S_OK)
				return sc;
			if((sc = ::OleCreateStaticFromData(this, IID_IOleObject, OLERENDER_FORMAT, &format, *ppOleClientSite, pStorage, (void**)(ppOleObject))) != S_OK)
				return sc;
			OleSetContainedObject(*ppOleObject, TRUE);
			return S_OK;
		}

		STDMETHOD(QueryInterface)(REFIID iid, void** ppObject) {
			if((iid == IID_IUnknown) || (iid == IID_IDataObject)) {
				*ppObject = this;
				AddRef();
				return S_OK;
			}
			return E_NOINTERFACE;
		}

		STDMETHOD(GetData)(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium) {
			HANDLE hDst = ::OleDuplicateData(medium.hBitmap, CF_BITMAP, 0);
			if(hDst == NULL)
				return E_HANDLE;
			pStgMedium->tymed = TYMED_GDI;
			pStgMedium->hBitmap = HBITMAP(hDst);
			pStgMedium->pUnkForRelease = NULL;
			return S_OK;
		}

		STDMETHOD(SetData)(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium, BOOL bRelease) {
			format = *pFormatEtc;
			medium = *pStgMedium;
			return S_OK;
		}

		STDMETHOD_(ULONG, AddRef)(void) {
			refcount++;
			return refcount;
		}

		STDMETHOD_(ULONG, Release)(void) {
			if(--refcount == 0)
				delete this;
			return refcount;
		}

		// not implemented
		STDMETHOD(DAdvise)(FORMATETC* pFormatEtc, DWORD dwAdvf, IAdviseSink* pAdviseSink, DWORD* pDwConnection) { return E_NOTIMPL; }
		STDMETHOD(DUnadvise)(DWORD dwConnection) { return E_NOTIMPL; }
		STDMETHOD(EnumDAdvise)(IEnumSTATDATA** ppEnumStatData) { return E_NOTIMPL; }
		STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc) { return E_NOTIMPL; }
		STDMETHOD(GetCanonicalFormatEtc)(FORMATETC* pFormatEtcIn, FORMATETC* pFormatEtcOut) { return E_NOTIMPL; }
		STDMETHOD(GetDataHere)(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium) { return E_NOTIMPL; }
		STDMETHOD(QueryGetData)(FORMATETC* pFormatEtc) { return E_NOTIMPL; }

	private:
		unsigned refcount;
		STGMEDIUM medium;
		FORMATETC format;
	};

	// richtext loaded members
	RichTextBox* richText;
	const HWND hWnd;
	const size_t limit;
	bool adjustdpi;

	// richtext editing members
	SETTEXTEX config;
	dwt::Point scrollPos;
	DWORD crstart;
	DWORD crend;
	bool scroll;
	bool edit;

	// html converter members
	vector<Part> parts; // text or image parts
	vector<Context> contexts; // list of open tags
	StringList fonts; // fonts table
	StringList colors; // colors table
	size_t lengthhtml; // length of text to be added to the richtext

	BDCRichText(RichTextBox* aRichText);
	~BDCRichText() { if (edit) { editEnd(); } }

	// inherited from SimpleXMLReader::CallBack
	void startTag(const string& aName, StringPairList& attribs, bool simple);
	void data(const string& aData);
	void endTag(const string& aName);

	// rtf writing
	void write(bool doclear = true);
	void clear();
	void newData(bool simple = false, const string& aData = Util::emptyString, const Context* aContext = NULL);
	void addData(const string& aData, bool rtfcode = false);
	size_t addFont(const string& aFont);
	size_t addColor(const string& aColor);

	// context handling
	void addInitialContext();
	void addContext(const Context& aContext);
	void fillContext(Context& context);
	void removeContext(const string& aName);

	// html reading
	void decodeHtmlFont(const string& aFont, Context& context);
	void decodeHtmlColor(const string& aColor, bool isBg, Context& context);
	void decodeHtmlTextDecoration(const string& aDecoration, Context& context);
	void decodeHtmlTextTransform(const string& aTransform, Context& context);
	void decodeHtmlBorder(const string& aBorderInfo, Context& context);
	void decodeHtmlBorderBottom(const string& aBorderInfo, Context& context);
	void decodeHtmlBorderBottomStyle(const string& aBorderStyle, Context& context);
	void decodeHtmlBorderBottomWidth(const string& aBorderWidth, Context& context);

	// richtext controls
	void editBegin(size_t addlen);
	void editEnd();
	BOOL insertImage(const string& aFile);
	void insertImage();

	// image functions
	static void getExampleParams(ParamMap& params);
	static HBITMAP getBitmapFromIcon(const tstring& aFile, COLORREF crBgColor);
	static HBITMAP getMaskedBitmap(const tstring& aFile, COLORREF crBgColor);
	static HBITMAP createMask(HBITMAP hBitmap);
};

#endif // DCPLUSPLUS_WIN32_BDCPP_RICH_TEXT_H
