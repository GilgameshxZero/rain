#pragma once

#include "content_type.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class PlaintextFragment {
		public:
		virtual ContentType contentType() const = 0;
		virtual std::uint16_t length() const = 0;
		virtual void sendWith(std::ostream &) const = 0;

		// We need a virtual destructor since we will be
		// deleting derived instances through a base pointer.
		virtual ~PlaintextFragment() {}
	};
}
