#pragma once

#include "../algorithm/geometry.hpp"
#include "exception.hpp"
#include "solid-brush.hpp"
#include "windows.hpp"

#include <stack>

#ifdef RAIN_PLATFORM_WINDOWS

namespace Rain::Windows {
	// Holds a pre-existing HDC, and manages any SelectObject operations on it.
	class DeviceContext {
		private:
		HDC const hDc;
		std::stack<HGDIOBJ> selectedObjs;

		public:
		DeviceContext(HDC hDc) : hDc{hDc} {}
		~DeviceContext() {
			while (!this->selectedObjs.empty()) {
				SelectObject(this->hDc, this->selectedObjs.top());
				this->selectedObjs.pop();
			}
		}
		DeviceContext(DeviceContext const &other) = delete;
		DeviceContext &operator=(DeviceContext const &other) = delete;
		DeviceContext(DeviceContext &&other) = delete;
		DeviceContext &operator=(DeviceContext &&other) = delete;

		inline operator HDC() const { return this->hDc; }

		template <typename ObjectType>
		void select(ObjectType const &object) {
			this->selectedObjs.push(
				SelectObject(this->hDc, static_cast<HGDIOBJ>(object)));
		}
		void deselect() {
			SelectObject(this->hDc, this->selectedObjs.top());
			this->selectedObjs.pop();
		}

		void fillRect(
			Algorithm::Geometry::RectangleL const &rectangle,
			SolidBrush const &brush) {
			RECT rect(rectangle);
			validateSystemCall(FillRect(*this, &rect, brush));
		}
		void moveTo(Algorithm::Geometry::PointL const &point) {
			validateSystemCall(MoveToEx(*this, point.x, point.y, NULL));
		}
		void lineTo(Algorithm::Geometry::PointL const &point) {
			validateSystemCall(LineTo(*this, point.x, point.y));
		}
		void ellipse(Algorithm::Geometry::RectangleL const &rectangle) {
			validateSystemCall(Ellipse(
				*this,
				rectangle.left,
				rectangle.top,
				rectangle.right,
				rectangle.bottom));
		}
	};

	// Creates a memory DC, and deletes it upon destruction.
	class DeviceContextMemory : public DeviceContext {
		public:
		DeviceContextMemory(HDC hDc)
				: DeviceContext(validateSystemCall(CreateCompatibleDC(hDc))) {}
		~DeviceContextMemory() { DeleteDC(*this); }
	};
}

#endif
