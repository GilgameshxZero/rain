#pragma once

#include "activation_interface.hpp"

namespace Rain::Math::Neural {
	template<typename Value>
	class NetworkInterface {
		public:
		virtual Tensor<Value, 1> asApply(
			Tensor<Value, 1> const &) const = 0;

		virtual ~NetworkInterface() {}
	};
}
