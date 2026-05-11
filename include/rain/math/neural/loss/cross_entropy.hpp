#pragma once

#include "../../tensor.hpp"
#include "../loss_interface.hpp"

#include <limits>

namespace Rain::Math::Neural::Loss {
	template<typename Value>
	class CrossEntropy : public LossInterface<Value> {
		public:
		Tensor<Value, 1> left;

		CrossEntropy(Tensor<Value, 1> const &left) :
			left{left.copy()} {}

		virtual Value asApply(
			Tensor<Value, 1> const &right) const override final {
			return clamp(-static_cast<Value>(
				this->left * right.asLog().clamp()));
		}
		virtual Tensor<Value, 1> getGradient(
			Tensor<Value, 1> const &right) const override final {
			// Offset probability distribution `right` by EPS to
			// guarantee no NANs.
			return -this->left
								.asDivideElementWise(
									right + std::numeric_limits<Value>::min())
								.clamp();
		}
	};
}
