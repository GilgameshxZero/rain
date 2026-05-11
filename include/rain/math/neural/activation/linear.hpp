#pragma once

#include "../activation_interface.hpp"

namespace Rain::Math::Neural::Activation {
	// Linear operator activation allows for Weight and Bias
	// and may change the size of tensor during `apply`.
	template<typename Value>
	class Linear : virtual public ActivationInterface<Value> {
		public:
		Tensor<Value, 2> weight;
		Tensor<Value, 1> bias;

		// Force deep-copy for W/B.
		Linear(
			Tensor<Value, 2> const &weight,
			Tensor<Value, 1> const &bias) :
			weight{weight.copy()},
			bias{bias.copy()} {}

		virtual Tensor<Value, 1> apply(
			Tensor<Value, 1> &z1) const override final {
			return (
				z1.template multiply<1>(this->weight, {0}, {0}) +
				this->bias)
				.clamp();
		}
		virtual Tensor<Value, 2> getGradient(
			Tensor<Value, 1> const &,
			Tensor<Value, 1> const &) const override final {
			return this->weight.asTranspose({1, 0});
		}
		virtual void stepWithGradient(
			Tensor<Value, 1> const &z1,
			Tensor<Value, 1> const &gradient) override final {
			this->weight =
				(this->weight - z1.asMultiplyOuter(gradient))
					.clamp();
			this->bias = (this->bias - gradient).clamp();
		}
	};
}
