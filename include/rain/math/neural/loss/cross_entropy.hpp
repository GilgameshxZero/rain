#pragma once

#include "../../tensor.hpp"
#include "../loss_interface.hpp"

#include <limits>

namespace Rain::Math::Neural::Loss {
	template<typename Value>
	class CrossEntropy : public LossInterface<Value> {
		public:
		using Super = LossInterface<Value>;
		using Super::asApply;
		using Super::getGradient;

		virtual Value asApply(
			Tensor<Value, 1> const &y1,
			Tensor<Value, 1> const &y2) const override {
			return clamp(
				-static_cast<Value>(y1 * y2.asLog().clamp()));
		}
		virtual Tensor<Value, 1> getGradient(
			Tensor<Value, 1> const &y1,
			Tensor<Value, 1> const &y2) const override {
			// Offset probability distribution `right` by EPS to
			// guarantee no NANs.
			return -y1.asDivideElementWise(
									y2 + std::numeric_limits<Value>::min())
								.clamp();
		}
	};
}
