#pragma once

#include "../algorithm/geometry.hpp"
#include "../string/string.hpp"
#include "exception.hpp"
#include "windows.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

namespace Rain::Windows {
	class Window {
		public:
		// Aggregate class for designated initialization (C++20).
		class CreateParameters {
			public:
			// A one-time class name will be generated if this is kept NULL.
			// Otherwise, no call to RegisterClassEx will be made, and an existing
			// window class will be used instead.
			LPCTSTR lpClassName{NULL};

			// Window class parameters.
			UINT style{CS_HREDRAW | CS_VREDRAW};
			HICON hIcon{NULL};
			HCURSOR hCursor{LoadCursor(NULL, IDC_ARROW)};
			HBRUSH hBrBackground{reinterpret_cast<HBRUSH>(COLOR_WINDOWFRAME)};
			LPCTSTR lpMenuName{NULL};
			HICON hIconSm{NULL};

			// Window parameters.
			DWORD dwExStyle{NULL};
			LPCTSTR lpWindowName{""};
			DWORD dwStyle{WS_OVERLAPPEDWINDOW | WS_VISIBLE};
			int x{CW_USEDEFAULT};
			int y{CW_USEDEFAULT};
			int nWidth{CW_USEDEFAULT};
			int nHeight{CW_USEDEFAULT};
			HWND hWndParent{NULL};
			HMENU hMenu{NULL};
		};

		private:
		static LRESULT CALLBACK
		windowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
			if (uMsg == WM_CREATE) {
				// When window is created, set its long pointer to the object.
				CREATESTRUCT *createStruct{reinterpret_cast<CREATESTRUCT *>(lParam)};
				SetWindowLongPtr(
					hWnd,
					GWLP_USERDATA,
					reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
				return reinterpret_cast<Window *>(createStruct->lpCreateParams)
					->onCreate(wParam, lParam);
			}
			Window *that{
				reinterpret_cast<Window *>(GetWindowLongPtr(hWnd, GWLP_USERDATA))};
			if (that == NULL) {
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}
			switch (uMsg) {
				case WM_CLOSE:
					return that->onClose(wParam, lParam);
				case WM_CONTEXTMENU:
					return that->onContextMenu(wParam, lParam);
				case WM_CREATE:
					return that->onCreate(wParam, lParam);
				case WM_DESTROY:
					return that->onDestroy(wParam, lParam);
				case WM_KEYDOWN:
					return that->onKeyDown(wParam, lParam);
				case WM_KEYUP:
					return that->onKeyUp(wParam, lParam);
				case WM_MOVE:
					return that->onMove(wParam, lParam);
				case WM_PAINT:
					return that->onPaint(wParam, lParam);
				case WM_POINTERDOWN:
					return that->onPointerDown(wParam, lParam);
				case WM_POINTERENTER:
					return that->onPointerEnter(wParam, lParam);
				case WM_POINTERHWHEEL:
					return that->onPointerHWheel(wParam, lParam);
				case WM_POINTERLEAVE:
					return that->onPointerLeave(wParam, lParam);
				case WM_POINTERUP:
					return that->onPointerUp(wParam, lParam);
				case WM_POINTERUPDATE:
					return that->onPointerUpdate(wParam, lParam);
				case WM_POINTERWHEEL:
					return that->onPointerWheel(wParam, lParam);
				case WM_POWERBROADCAST:
					return that->onPowerBroadcast(wParam, lParam);
				case WM_SIZE:
					return that->onSize(wParam, lParam);
				case WM_SYSKEYDOWN:
					return that->onSysKeyDown(wParam, lParam);
				case WM_SYSKEYUP:
					return that->onSysKeyUp(wParam, lParam);
				case WM_WINDOWPOSCHANGED:
					return that->onWindowPosChanged(wParam, lParam);
				default:
					return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}
		}

		static HWND createWindow(
			Window *that,
			CreateParameters const &createParams) {
			static std::size_t CLASS_ID{0};
			HINSTANCE hInstance{GetModuleHandle(NULL)};
			// className must be kept in scope for the duration of the function.
			std::string className{
				"Rain::Windows::Window: " + std::to_string(CLASS_ID++)};
			LPCTSTR lpClassName;
			if (createParams.lpClassName == NULL) {
				lpClassName = className.c_str();
				WNDCLASSEX wndClassEx{
					sizeof(WNDCLASSEX),
					createParams.style,
					Window::windowProcedure,
					0,
					0,
					hInstance,
					createParams.hIcon,
					createParams.hCursor,
					createParams.hBrBackground,
					createParams.lpMenuName,
					lpClassName,
					createParams.hIconSm};
				validateSystemCall(RegisterClassEx(&wndClassEx));
			} else {
				lpClassName = createParams.lpClassName;
			}
			return validateSystemCall(CreateWindowEx(
				createParams.dwExStyle,
				lpClassName,
				createParams.lpWindowName,
				createParams.dwStyle,
				createParams.x,
				createParams.y,
				createParams.nWidth,
				createParams.nHeight,
				createParams.hWndParent,
				createParams.hMenu,
				hInstance,
				reinterpret_cast<LPVOID>(that)));
		}

		HWND const hWnd;

		protected:
		virtual LRESULT onClose(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_CLOSE, wParam, lParam);
		}
		virtual LRESULT onContextMenu(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_CONTEXTMENU, wParam, lParam);
		}
		virtual LRESULT onCreate(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_CREATE, wParam, lParam);
		}
		virtual LRESULT onDestroy(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_DESTROY, wParam, lParam);
		}
		virtual LRESULT onKeyDown(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_KEYDOWN, wParam, lParam);
		}
		virtual LRESULT onKeyUp(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_KEYUP, wParam, lParam);
		}
		virtual LRESULT onMove(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_MOVE, wParam, lParam);
		}
		virtual LRESULT onSize(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_SIZE, wParam, lParam);
		}
		virtual LRESULT onPaint(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_PAINT, wParam, lParam);
		}
		virtual LRESULT onPointerDown(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_POINTERDOWN, wParam, lParam);
		}
		virtual LRESULT onPointerEnter(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_POINTERENTER, wParam, lParam);
		}
		virtual LRESULT onPointerHWheel(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_POINTERHWHEEL, wParam, lParam);
		}
		virtual LRESULT onPointerLeave(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_POINTERLEAVE, wParam, lParam);
		}
		virtual LRESULT onPointerUp(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_POINTERUP, wParam, lParam);
		}
		virtual LRESULT onPointerUpdate(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_POINTERUPDATE, wParam, lParam);
		}
		virtual LRESULT onPointerWheel(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_POINTERWHEEL, wParam, lParam);
		}
		virtual LRESULT onPowerBroadcast(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_POWERBROADCAST, wParam, lParam);
		}
		virtual LRESULT onSysKeyDown(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_SYSKEYDOWN, wParam, lParam);
		}
		virtual LRESULT onSysKeyUp(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_SYSKEYUP, wParam, lParam);
		}
		virtual LRESULT onWindowPosChanged(WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, WM_WINDOWPOSCHANGED, wParam, lParam);
		}

		public:
		Window(CreateParameters const &createParams)
				: hWnd{this->createWindow(this, createParams)} {}
		// Window can also wrap an existing HWND, to enable utility functions on it.
		Window(HWND hWnd) : hWnd{hWnd} {}
		~Window() {}

		// Window is a RAII class and cannot be moved nor copied. To do so, use
		// shared_ptr or unique_ptr. Inherited classes will automatically have their
		// move/copy disabled, unless it is written explicitly, without calling
		// this base class move/copy.
		Window(Window const &) = delete;
		Window &operator=(Window const &) = delete;
		Window(Window &&) = delete;
		Window &operator=(Window &&) = delete;

		operator HWND() const { return this->hWnd; }

		Algorithm::Geometry::RectangleL getClientRect() const {
			RECT rect;
			validateSystemCall(GetClientRect(*this, &rect));
			return {rect};
		}
		Algorithm::Geometry::RectangleL getWindowRect() const {
			RECT rect;
			validateSystemCall(GetWindowRect(*this, &rect));
			return {rect};
		}
		UINT getDpi() const { return validateSystemCall(GetDpiForWindow(*this)); }
		void invalidateClient() {
			validateSystemCall(InvalidateRect(*this, NULL, false));
		}
	};
}

#endif
