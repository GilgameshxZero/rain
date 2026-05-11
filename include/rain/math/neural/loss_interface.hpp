#pragma once

#include "../tensor.hpp"

namespace Rain::Math::Neural {
	// TODO: How to clean up Loss to support batches?
	template<typename Value>
	class LossInterface {
		public:
		virtual Value asApply(
			Tensor<Value, 1> const &) const = 0;
		virtual Tensor<Value, 1> getGradient(
			Tensor<Value, 1> const &) const = 0;

		virtual ~LossInterface() {}
	};
}
