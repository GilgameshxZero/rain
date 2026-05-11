#pragma once

#include "../tensor.hpp"

namespace Rain::Math::Neural {
	// Takes Tensor(N_0) to Tensor(N_1).
	template<typename Value>
	class ActivationInterface {
		public:
		// All functions provide an ORDER + 1 overload to take
		// batches. Apply the activation to take Z_1(N_1) to
		// Z_2(N_2). The activated vector is returned, and the
		// original one is also updated.
		virtual Tensor<Value, 1> apply(
			Tensor<Value, 1> &) const = 0;
		auto apply(Tensor<Value, 2> &X) const {
			return X.applyOver<1>([this](Tensor<Value, 1> &left) {
				this->apply(left);
			});
		}
		// Decay rvalue-reference.
		auto apply(Tensor<Value, 1> &&tensor) const {
			return this->apply(tensor);
		}
		auto apply(Tensor<Value, 2> &&tensor) const {
			return this->apply(tensor);
		}
		// Apply the activation without side-effects.
		auto asApply(Tensor<Value, 1> const &tensor) const {
			return this->apply(tensor.copy());
		}
		auto asApply(Tensor<Value, 2> const &tensor) const {
			return this->apply(tensor.copy());
		}
		// Given Z_2, return a matrix of gradients G where
		// G[i][j] is the partial derivative of Z_2[i] w.r.t.
		// Z_1[j].
		//
		// The activated values Z_2 must be provided as an
		// argument to avoid re-computation (in some cases).
		virtual Tensor<Value, 2> getGradient(
			Tensor<Value, 1> const &,
			Tensor<Value, 1> const &) const = 0;
		// Since gradient is a linear operator, batch version
		// will take the mean across the batch and give the same
		// size as the non-batch version.
		auto getGradient(
			Tensor<Value, 2> const &z1,
			Tensor<Value, 2> const &z2) const {
			// No easy way to write this since we don't have a
			// free way to find the size of the non-batch variant.
			auto result{this->getGradient(z1[0], z2[0])};
			for (std::size_t i{1}; i < z1.size()[0]; i++) {
				result += this->getGradient(z1[i], z2[i]);
			}
			return result / z1.size()[0];
		}
		// Update internal constants based on gradient d(loss) /
		// d(Z_2) \in R^{N_2}. Thus, for each internal constant
		// C, we must have d(Z_2) / d(C). For ORDER 2 internal
		// matrix W, then we must have d(Z_2) / d(W) \in R^{N_2
		// * N_1 * N_2}.
		//
		// Parameters are artifact, gradient.
		//
		// No step size is provided, gradient should be scaled
		// accordingly.
		virtual void stepWithGradient(
			Tensor<Value, 1> const &,
			Tensor<Value, 1> const &) = 0;
		// No batch version necessary; artifact and gradient are
		// both linear and can be averaged.

		virtual ~ActivationInterface() {}
	};
}
