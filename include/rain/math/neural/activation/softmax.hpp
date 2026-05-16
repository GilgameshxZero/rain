#pragma once

#include "../activation_interface.hpp"

namespace Rain::Math::Neural::Activation {
	template<typename Value>
	class Softmax :
		virtual public ActivationInterface<Value> {
		public:
		virtual Tensor<Value, 1> apply(
			Tensor<Value, 1> &z1) const override {
			// Easy to overflow during exp!
			// Range: [+0, +MAX].
			z1.exp();
			// z1 is finite, z1.sum is [+0, +INF] but is always
			// at least z1. So, range is finite and needn't be
			// clamped.
			return z1 /=
				z1.sum() + std::numeric_limits<Value>::min();
		}
		virtual Tensor<Value, 2> getIncrementalGradient(
			Tensor<Value, 1> const &,
			Tensor<Value, 1> const &z2) const override {
			return z2.asMultiplyElementWise(1 - z2).asDiagonal();
		}
	};
}
