#pragma once

#include "../tensor.hpp"

namespace Rain::Math::Neural {
	// Takes Tensor(N_0) to Tensor(N_1).
	template<typename Value>
	class ActivationInterface {
		public:
		// Apply the activation to take Z_1(N_0) to Z_2(N_1).
		// The activated vector is returned, and the original
		// one is also updated.
		virtual Tensor<Value, 1> apply(
			Tensor<Value, 1> &) const = 0;
		// Decay rvalue-reference.
		auto apply(Tensor<Value, 1> &&tensor) const {
			return this->apply(tensor);
		}
		// Apply the activation without side-effects.
		auto asApply(Tensor<Value, 1> const &tensor) const {
			return this->apply(tensor.copy());
		}
		// Return a matrix of gradients G where G[i][j] is the
		// partial derivative of Z_1[i] w.r.t. Z_2[j].
		//
		// The activated values Z_2 must be provided as an
		// argument.
		virtual Tensor<Value, 2> getGradient(
			Tensor<Value, 1> const &) const = 0;
	};
}
