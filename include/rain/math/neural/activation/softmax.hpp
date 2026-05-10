#pragma once

#include "../activation_interface.hpp"

namespace Rain::Math::Neural::Activation {
	template<typename Value>
	class Softmax :
		virtual public ActivationInterface<Value> {
		public:
		virtual Tensor<Value, 1> apply(
			Tensor<Value, 1> &z1) const override final {
			z1 = z1.exp().clamp();
			return z1 /= z1.sum();
		}
		virtual Tensor<Value, 2> getGradient(
			Tensor<Value, 1> const &z2) const override final {
			return z2.asMultiplyElementWise(1 - z2).asDiagonal();
		}
	};
}
