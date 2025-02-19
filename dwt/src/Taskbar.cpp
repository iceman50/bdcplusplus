/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2022, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor SmartWin++ nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <dwt/Taskbar.h>

#include <dwmapi.h>

#include <dwt/util/check.h>
#include <dwt/widgets/Container.h>
#include <dwt/widgets/Window.h>

namespace dwt {

Taskbar::Taskbar() :
taskbar(0),
window(0)
{
}

Taskbar::~Taskbar() {
	if(taskbar)
		taskbar->Release();
}

void Taskbar::initTaskbar(WindowPtr window_) {
	window = window_;
	dwtassert(window, "Taskbar: no widget set");

	/* init the ITaskbarList3 COM pointer. MSDN recommends waiting for the
	"TaskbarButtonCreated" message, but neither MFC nor Win SDK samples do that, so we don't
	either. greatly simplifies the logic of this interface. */
#ifdef __GNUC__
	/// @todo remove when GCC knows about ITaskbarList
	CLSID CLSID_TaskbarList;
	OLECHAR tbl[] = L"{56FDF344-FD6D-11d0-958A-006097C9A090}";
	CLSIDFromString(tbl, &CLSID_TaskbarList);
	IID IID_ITaskbarList;
	OLECHAR itbl[] = L"{56FDF342-FD6D-11d0-958A-006097C9A090}";
	CLSIDFromString(itbl, &IID_ITaskbarList);
#endif
	if(::CoCreateInstance(CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER, IID_ITaskbarList,
		reinterpret_cast<LPVOID*>(&taskbar)) != S_OK) { taskbar = 0; }
	if(taskbar && taskbar->HrInit() != S_OK) {
			taskbar->Release();
			taskbar = 0;
	}
}

class Proxy : public Frame {
	typedef Frame BaseType;
	friend class WidgetCreator<Proxy>;

public:
	typedef Proxy ThisType;
	typedef ThisType* ObjectType;

	struct Seed : BaseType::Seed {
		typedef ThisType WidgetType;
		Seed(const tstring& caption) : BaseType::Seed(caption, 0, 0) {
			style = WS_POPUP | WS_CAPTION;
			exStyle = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
			location = Rectangle();
		}
	};

	Proxy(Widget* parent) : BaseType(parent, NormalDispatcher::getDefault()) { }
};

void Taskbar::addToTaskbar(ContainerPtr tab) {
	/* for Windows to acknowledge that our tab window is worthy of having its own thumbnail in the
	taskbar, we have to create an invisible popup window that will act as a proxy between the
	taskbar and the actual tab window.
	this technique is illustrated in MFC as well as the Windows SDK sample at
	"Samples\winui\shell\appshellintegration\TabThumbnails". */

	auto proxy = window->addChild(Proxy::Seed(tab->getText()));
	tabs[tab] = proxy;

	/* call ChangeWindowMessageFilterEx on the 2 messages we use to dispatch bitmaps to the
	destktop manager to prevent blockings because of different privilege levels. */
	::ChangeWindowMessageFilterEx(proxy->handle(), WM_DWMSENDICONICTHUMBNAIL, 1/*MSGFLT_ALLOW*/, 0);
	::ChangeWindowMessageFilterEx(proxy->handle(), WM_DWMSENDICONICLIVEPREVIEWBITMAP, 1/*MSGFLT_ALLOW*/, 0);

	// keep the proxy window in sync with the actual tab window.
	tab->onTextChanging([proxy](const tstring& text) { proxy->setText(text); });
	tab->onSized([proxy](const SizedEvent&) { ::DwmInvalidateIconicBitmaps(proxy->handle()); });

	// forward taskbar events that were sent to the proxy window to the actual tab window.
	proxy->onActivate([this, tab](bool activate) {
		if(activate) {
			setActive(tab);
			::SetForegroundWindow(window->handle());
			if(window->isIconic())
				window->restore();
			else
				window->setVisible(true);
		}
	});
	proxy->onClosing([this, tab]() -> bool {
		if(window->getEnabled()) {
			tab->close(true);
		}
		return false; // don't close the proxy window just yet; wait for removeFromTaskbar.
	});

	proxy->onRaw([this, tab, proxy](WPARAM, LPARAM lParam) -> LRESULT {
		// generate a thumbnail to be displayed in the taskbar.
		BitmapPtr bitmap = getBitmap(tab, lParam);
		if(bitmap) {
			::DwmSetIconicThumbnail(proxy->handle(), bitmap->handle(), 0);
		}
		return 0;
	}, Message(WM_DWMSENDICONICTHUMBNAIL));

	proxy->onRaw([this, tab, proxy](WPARAM, LPARAM) -> LRESULT {
		// generate a preview of the tab to be shown in "Aero Peek" when the user hovers the thumbnail.
		BitmapPtr bitmap = getBitmap(tab, 0);
		if(bitmap) {
			POINT offset = { 0 };
			::MapWindowPoints(tab->handle(), window->handle(), &offset, 1);
			MENUBARINFO info = { sizeof(MENUBARINFO) };
			if(::GetMenuBarInfo(window->handle(), OBJID_MENU, 0, &info))
				offset.y += Rectangle(info.rcBar).height();
			::DwmSetIconicLivePreviewBitmap(proxy->handle(), bitmap->handle(), &offset, 0);
		}
		return 0;
	}, Message(WM_DWMSENDICONICLIVEPREVIEWBITMAP));

	// indicate to the window manager that it should always use the bitmaps we provide.
	BOOL attrib = TRUE;
	::DwmSetWindowAttribute(proxy->handle(), DWMWA_FORCE_ICONIC_REPRESENTATION, &attrib, sizeof(attrib));
	::DwmSetWindowAttribute(proxy->handle(), DWMWA_HAS_ICONIC_BITMAP, &attrib, sizeof(attrib));

	taskbar->RegisterTab(proxy->handle(), window->handle());
	moveOnTaskbar(tab);
}

void Taskbar::removeFromTaskbar(ContainerPtr tab) {
	auto proxy = tabs[tab];
	taskbar->UnregisterTab(proxy->handle());
	::DestroyWindow(proxy->handle());
	tabs.erase(tab);
}

void Taskbar::moveOnTaskbar(ContainerPtr tab, ContainerPtr rightNeighbor) {
	taskbar->SetTabOrder(tabs[tab]->handle(), rightNeighbor ? tabs[rightNeighbor]->handle() : 0);
}

void Taskbar::setActiveOnTaskbar(ContainerPtr tab) {
	taskbar->SetTabActive(tabs[tab]->handle(), window->handle(), 0);
}

void Taskbar::setTaskbarIcon(ContainerPtr tab, const IconPtr& icon) {
	tabs[tab]->setSmallIcon(icon);
}

void Taskbar::setOverlayIcon(ContainerPtr tab, const IconPtr& icon, const tstring& description) {
	taskbar->SetOverlayIcon(window->handle(), icon->handle(), description.c_str());
}

BitmapPtr Taskbar::getBitmap(ContainerPtr tab, LPARAM thumbnailSize) {
	UpdateCanvas canvas { tab };

	// get the actual size of the tab.
	const Point size_full { tab->getClientSize() };
	if(size_full.x <= 0 || size_full.y <= 0)
		return 0;

	// this DIB will hold a full capture of the tab.
	BITMAPINFO info { { sizeof(BITMAPINFOHEADER), size_full.x, size_full.y, 1, 32, BI_RGB } };
	BitmapPtr bitmap_full { new Bitmap { ::CreateDIBSection(canvas.handle(), &info, DIB_RGB_COLORS, 0, 0, 0) } };

	CompatibleCanvas canvas_full { canvas.handle() };
	auto select_full(canvas_full.select(*bitmap_full));

	tab->sendMessage(WM_PRINT, reinterpret_cast<WPARAM>(canvas_full.handle()), PRF_CLIENT | PRF_NONCLIENT | PRF_CHILDREN | PRF_ERASEBKGND);

	// get rid of some transparent bits.
	::BitBlt(canvas_full.handle(), 0, 0, size_full.x, size_full.y, canvas_full.handle(), 0, 0, MERGECOPY);

	if(!thumbnailSize)
		return bitmap_full;

	// compute the size of the thumbnail, must not exceed the LPARAM values.
	double factor { std::min(static_cast<double>(HIWORD(thumbnailSize)) / static_cast<double>(size_full.x),
		static_cast<double>(LOWORD(thumbnailSize)) / static_cast<double>(size_full.y)) };
	const Point size_thumb { static_cast<long>(size_full.x * factor), static_cast<long>(size_full.y * factor) };
	if(size_thumb.x <= 0 || size_thumb.y <= 0)
		return 0;

	// this DIB will hold a resized view of the tab, to be used as a thumbnail.
	info.bmiHeader.biWidth = size_thumb.x;
	info.bmiHeader.biHeight = size_thumb.y;
	BitmapPtr bitmap_thumb { new Bitmap { ::CreateDIBSection(canvas.handle(), &info, DIB_RGB_COLORS, 0, 0, 0) } };

	CompatibleCanvas canvas_thumb { canvas.handle() };
	auto select_thumb(canvas_thumb.select(*bitmap_thumb));

	::SetStretchBltMode(canvas_thumb.handle(), HALFTONE);
	::SetBrushOrgEx(canvas_thumb.handle(), 0, 0, 0);
	::StretchBlt(canvas_thumb.handle(), 0, 0, size_thumb.x, size_thumb.y,
		canvas_full.handle(), 0, 0, size_full.x, size_full.y, SRCCOPY);

	return bitmap_thumb;
}

}
