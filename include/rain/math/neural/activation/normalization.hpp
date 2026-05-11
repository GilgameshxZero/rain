#pragma once

#include "../activation_interface.hpp"

namespace Rain::Math::Neural::Activation {
	template<typename Value>
	class Normalization :
		virtual public ActivationInterface<Value> {
		public:
		virtual Tensor<Value, 1> apply(
			Tensor<Value, 1> &z1) const override final {
			// f(x_i) = (x_i - mean(x)) / stddev(x).
			// Should not need to clamp usually.
			return z1 = (z1 - z1.mean()) /
				(z1.standardDeviation() +
					std::numeric_limits<Value>::min());
		}
		virtual Tensor<Value, 2> getGradient(
			Tensor<Value, 1> const &z1,
			Tensor<Value, 1> const &z2) const override final {
			// Compute matrix where g[i][j] = d(f(v_i)) / d(v_j).
			return (Tensor<Value, 2>::identity(z2.size()[0]) *
								 z2.size()[0] -
							 1 - z2.asMultiplyOuter(z2)) /
				z2.size()[0] / z1.standardDeviation();
		}
		virtual void stepWithGradient(
			Tensor<Value, 1> const &,
			Tensor<Value, 1> const &) override final {}
	};
}
