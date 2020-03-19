/*
Standard
*/

/*
Implements RainWindow class which adds functionality on typical HWNDs.

Windows-specific.
*/

#pragma once

#pragma comment(lib,"user32.lib") 

#include "windows-lam-include.hpp"
#include "utility-libraries.hpp"

#include <tchar.h>
#include <unordered_map>
#include <vector>

//first available value for custom messages
#define WM_RAINAVAILABLE	WM_APP + 100

namespace Rain {
	//does not interfere with GWLP_USERDATA
	//(!!) call destructor instead of sending WM_DESTROY
	class RainWindow {
	public:
		typedef LRESULT(*MSGFC)(HWND, UINT, WPARAM, LPARAM);

		static const LPCTSTR NULLCLASSNAME;

		HWND hwnd;

		RainWindow();
		~RainWindow();

		int create(
			std::unordered_map<UINT, MSGFC> *msgm, //pointer to a map which maps messages to functions to be called when the message is received
			MSGFC	  *intfc = NULL, //interceptor function: if the application wants to define a custom message handler, here is where to do it
			UINT      style = 0,
			int       cbClsExtra = 0,
			int	      cbWndExtra = 0,
			HINSTANCE hInstance = GetModuleHandle(NULL),
			HICON     hIcon = NULL,
			HCURSOR   hCursor = NULL,
			HBRUSH    hbrBackground = NULL,
			LPCTSTR   lpszMenuName = _T(""),
			HICON     hIconSm = NULL,
			DWORD     dwExStyle = 0,
			LPCTSTR   lpWindowName = _T(""),
			DWORD     dwStyle = WS_OVERLAPPEDWINDOW,
			int       x = 0,
			int       y = 0,
			int       nWidth = 0,
			int       nHeight = 0,
			HWND      hWndParent = NULL,
			HMENU     hMenu = NULL,
			LPCTSTR   lpszClassName = NULLCLASSNAME);
		LPCTSTR getWndClassName();

		static RainWindow *getWndObj(HWND hwndid);

		//can be called by anything; enter message loop for all windows; returns when WM_QUIT is received
		static WPARAM enterMessageLoop();

	private:
		static int class_id;
		static std::unordered_map<HWND, RainWindow *> objmap;

		std::unordered_map<UINT, MSGFC> *msgm;
		MSGFC *intfc;
		LPTSTR classname;

		friend LRESULT CALLBACK rainWindowProc( //the windowproc is controlled by this class; process messages through the map in the initializer; if intfc != NULL, this function is not used
			HWND   hwnd,
			UINT   uMsg,
			WPARAM wParam,
			LPARAM lParam
		);
	};

	LRESULT CALLBACK rainWindowProc(
		HWND   hwnd,
		UINT   uMsg,
		WPARAM wParam,
		LPARAM lParam
	);
}

namespace Rain {
	const LPCTSTR RainWindow::NULLCLASSNAME = _T("");
	int RainWindow::class_id;
	std::unordered_map<HWND, RainWindow *> RainWindow::objmap;

	RainWindow::RainWindow() {
		hwnd = NULL;
		classname = NULL;
	}
	RainWindow::~RainWindow() {
		if (hwnd) {
			int k = DestroyWindow(hwnd);
			if (k == 0) {
				int p = GetLastError();
				k += p;
			}
			hwnd = NULL;
		}
		if (classname) {
			delete[] classname;
			classname = NULL;
		}
	}
	int RainWindow::create(
		std::unordered_map<UINT, MSGFC> *msgm,
		MSGFC		*intfc,
		UINT		style,
		int			cbClsExtra,
		int			cbWndExtra,
		HINSTANCE	hInstance,
		HICON		hIcon,
		HCURSOR		hCursor,
		HBRUSH		hbrBackground,
		LPCTSTR		lpszMenuName,
		HICON		hIconSm,
		DWORD		dwExStyle,
		LPCTSTR		lpWindowName,
		DWORD		dwStyle,
		int			x,
		int			y,
		int			nWidth,
		int			nHeight,
		HWND		hWndParent,
		HMENU		hMenu,
		LPCTSTR		lpszClassName) {
		this->msgm = msgm;
		this->intfc = intfc;

		if (lpszClassName == NULLCLASSNAME) {
			static LPCTSTR prefix = _T("Rain::Mono5::RainWindow ");
			static const size_t prelen = _tcslen(prefix);
			LPTSTR format = new TCHAR[prelen + 3]; //2 for the "%d", and 1 for the "\0"
			int idlen = static_cast<int>(Rain::tToStr(class_id).length()); //length of class_id
			classname = new TCHAR[idlen + prelen + 1]; //1 for the "\0", memory freed later
			_tcscpy_s(format, prelen + 3, prefix);
			_tcscat_s(format, prelen + 3, _T("%d"));
			_stprintf_s(classname, idlen + prelen + 1, format, class_id);
			lpszClassName = classname;
			class_id++;
			delete[] format;
		} else {
			size_t prelen = _tcslen(lpszClassName);
			classname = new TCHAR[prelen + 1];
			_tcscpy_s(classname, prelen + 1, lpszClassName);
		}

		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = style;
		wcex.lpfnWndProc = rainWindowProc;
		wcex.cbClsExtra = cbClsExtra;
		wcex.cbWndExtra = cbWndExtra;
		wcex.hInstance = hInstance;
		wcex.hIcon = hIcon;
		wcex.hCursor = hCursor;
		wcex.hbrBackground = hbrBackground;
		wcex.lpszMenuName = lpszMenuName;
		wcex.lpszClassName = lpszClassName;
		wcex.hIconSm = hIconSm;

		if (!RegisterClassEx(&wcex)) return GetLastError();

		hwnd = CreateWindowEx(
			dwExStyle,
			lpszClassName,
			lpWindowName,
			dwStyle,
			x, y, nWidth, nHeight,
			hWndParent,
			hMenu,
			hInstance,
			this); //pass pointer to this class, so that we can access message funcs

		if (hwnd == NULL) return GetLastError();

		objmap.insert(std::make_pair(hwnd, this));

		return 0;
	}
	LPCTSTR RainWindow::getWndClassName() {
		return classname;
	}

	RainWindow *RainWindow::getWndObj(HWND hwndid) {
		auto it = objmap.find(hwndid);
		if (it == objmap.end())
			return NULL;
		return it->second;
	}
	WPARAM RainWindow::enterMessageLoop() {
		MSG msg;
		BOOL bRet;

		while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
			if (bRet == -1)
				return -1; //serious error
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		return msg.wParam;
	}

	LRESULT CALLBACK rainWindowProc(
		HWND   hwnd,
		UINT   uMsg,
		WPARAM wParam,
		LPARAM lParam) {
		UNALIGNED RainWindow *wndobj;

		if (uMsg == WM_CREATE || uMsg == WM_NCCREATE)
			wndobj = reinterpret_cast<UNALIGNED RainWindow *>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
		else
			wndobj = RainWindow::getWndObj(hwnd);

		if (wndobj == NULL)
			return DefWindowProc(hwnd, uMsg, wParam, lParam);

		if (wndobj->intfc != NULL) {
			LRESULT rt = (*(wndobj->intfc)) (hwnd, uMsg, wParam, lParam);
			if (rt != 0)
				return rt;
		}

		auto it = wndobj->msgm->find(uMsg);
		if (it != wndobj->msgm->end())
			return it->second(hwnd, uMsg, wParam, lParam);
		else
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

