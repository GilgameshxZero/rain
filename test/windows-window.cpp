#include <rain/algorithm/geometry.hpp>
#include <rain/literal.hpp>
#include <rain/log.hpp>
#include <rain/windows.hpp>

#include <iostream>
#include <thread>
#include <vector>

#ifdef RAIN_PLATFORM_WINDOWS

class Window : public Rain::Windows::Window {
	using Rain::Windows::Window::Window;

	private:
	Rain::Windows::SolidPen pen{8, 0x00000000};
	Rain::Windows::SolidBrush brush{0x00ffffff};

	std::vector<Rain::Algorithm::Geometry::PointL> clickedPoints;

	protected:
	LRESULT onCreate(WPARAM, LPARAM) override {
		Rain::Windows::validateSystemCall(RegisterTouchWindow(*this, NULL));
		std::cout << "Window::onCreate.\n";
		return 0;
	}
	LRESULT onDestroy(WPARAM wParam, LPARAM lParam) override {
		PostQuitMessage(0);
		std::cout << "Window::onDestroy.\n";
		return 0;
	}
	LRESULT onPaint(WPARAM wParam, LPARAM lParam) override {
		Rain::Windows::PaintStruct ps(*this);
		auto dc{ps.hDc()};
		// Fails compilation as expected since copy is deleted for inherited
		// classes.
		// Rain::Windows::DeviceContextMemory dcm(dc);
		// Rain::Windows::DeviceContextMemory dcm2(dc);
		// dcm2 = dcm;
		auto rcPaint{ps.rcPaint()}, rectangle{ps.rcPaint()};
		rectangle.right = (rcPaint.left * 2 + rcPaint.right) / 3;
		dc.fillRect(rectangle, {0x00ff0000});
		rectangle.left = rectangle.right;
		rectangle.right = (rcPaint.left + rcPaint.right * 2) / 3;
		dc.fillRect(rectangle, {0x0000ff00});
		rectangle.left = rectangle.right;
		rectangle.right = rcPaint.right;
		dc.fillRect(rectangle, {0x000000ff});

		dc.select(this->pen);
		dc.select(this->brush);
		static int const SIZE{20};
		for (auto const &i : this->clickedPoints) {
			dc.ellipse({i.x - SIZE, i.y - SIZE, i.x + SIZE, i.y + SIZE});
		}
		std::cout << "Window::onPaint.\n";
		return 0;
	}
	LRESULT onPointerUp(WPARAM wParam, LPARAM lParam) override {
		POINTER_INFO pointerInfo;
		Rain::Windows::validateSystemCall(
			GetPointerInfo(GET_POINTERID_WPARAM(wParam), &pointerInfo));
		Rain::Windows::validateSystemCall(
			ScreenToClient(*this, &pointerInfo.ptPixelLocation));
		this->clickedPoints.emplace_back(
			pointerInfo.ptPixelLocation.x, pointerInfo.ptPixelLocation.y);
		this->invalidateClient();
		std::cout << "Window::onPointerUp.\n";
		return 0;
	}
};

int main() {
	using namespace Rain::Literal;

	Rain::Windows::validateSystemCall(EnableMouseInPointer(TRUE));
	Rain::Windows::validateSystemCall(
		SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));

	Window window(
		Window::CreateParameters{
			.lpWindowName = "test/windows-window.cpp window name"});
	DWORD mainThreadId{GetCurrentThreadId()};
	std::thread([&]() {
		std::this_thread::sleep_for(1s);
		auto rect{window.getBoundingRect()};
		INPUT input;
		input.type = INPUT_MOUSE;
		input.mi.dx = MulDiv(rect.left + 100, 65536, GetSystemMetrics(SM_CXSCREEN));
		input.mi.dy = MulDiv(rect.top + 100, 65536, GetSystemMetrics(SM_CYSCREEN));
		input.mi.time = 0;
		input.mi.dwExtraInfo = NULL;
		input.mi.mouseData = NULL;
		input.mi.dwFlags =
			MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
		SendInput(1, &input, sizeof(INPUT));
		std::cout << "Moved mouse to " << rect.left + 100 << ", " << rect.top + 100
							<< ".\n";

		std::this_thread::sleep_for(1s);
		input.mi.dwFlags =
			MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
		SendInput(1, &input, sizeof(INPUT));

		std::this_thread::sleep_for(1s);
		input.mi.dwFlags =
			MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
		SendInput(1, &input, sizeof(INPUT));
		std::cout << "Clicked mouse.\n";

		std::this_thread::sleep_for(1s);
		PostThreadMessage(mainThreadId, WM_QUIT, NULL, NULL);
	}).detach();
	Rain::Windows::runMessageLoop();

	return 0;
}

#else

int main() {
	std::cout << "This is a Windows-only test.\n";
	return 0;
}

#endif
