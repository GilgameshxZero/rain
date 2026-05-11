#pragma once

#include "../activation_interface.hpp"

namespace Rain::Math::Neural::Activation {
	template<typename Value>
	class Identity :
		virtual public ActivationInterface<Value> {
		public:
		virtual Tensor<Value, 1> apply(
			Tensor<Value, 1> &z1) const override {
			return z1;
		}
		virtual Tensor<Value, 2> getGradient(
			Tensor<Value, 1> const &,
			Tensor<Value, 1> const &z2) const override {
			return Tensor<Value, 2>::identity(z2.size()[0]);
		}
	};
}
