#pragma once

#include "compression.hpp"

namespace Rain::Networking::Tls {
	class SecurityParameters {
		public:
		CompressionMethod compressionAlgorithm;
	};
}
