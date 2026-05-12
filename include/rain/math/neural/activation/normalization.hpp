#pragma once

#include "../../clamp.hpp"
#include "../activation_interface.hpp"

namespace Rain::Math::Neural::Activation {
	template<typename Value>
	class Normalization :
		virtual public ActivationInterface<Value> {
		public:
		virtual Tensor<Value, 1> apply(
			Tensor<Value, 1> &z1) const override {
			// f(x_i) = (x_i - mean(x)) / stddev(x).
			// Failing to clamp where necessary can block later
			// Tensor computations and cause high loss!
			return z1 =
							 ((z1 - z1.mean()) /
								 Math::clamp(
									 z1.standardDeviation() +
									 std::numeric_limits<Value>::min()))
								 .clamp();
		}
		virtual Tensor<Value, 2> getIncrementalGradient(
			Tensor<Value, 1> const &z1,
			Tensor<Value, 1> const &z2) const override {
			// Compute matrix where g[i][j] = d(f(v_i)) / d(v_j).
			// Failing to clamp where necessary can block later
			// Tensor computations and cause high loss!
			return (
				(Tensor<Value, 2>::identity(z2.size()[0]) *
						z2.size()[0] -
					1 - z2.asMultiplyOuter(z2)) /
				Math::clamp(
					z2.size()[0] * z1.standardDeviation() +
					std::numeric_limits<Value>::min()))
				.clamp();
		}
	};
}
