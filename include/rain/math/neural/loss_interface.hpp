#pragma once

#include "../tensor.hpp"

namespace Rain::Math::Neural {
	template<typename Value>
	class LossInterface {
		public:
		// Takes y1 = target distribution, y2 = current
		// distribution as arguments.
		virtual Value asApply(
			Tensor<Value, 1> const &,
			Tensor<Value, 1> const &) const = 0;
		Value asApply(
			Tensor<Value, 2> const &Y1,
			Tensor<Value, 2> const &Y2) const {
			return Y1
				.template asContract<>(
					1,
					[this](
						Tensor<Value, 1> const &y1,
						Tensor<Value, 1> const &y2) {
						return this->asApply(y1, y2);
					},
					Y2)
				.mean();
		};
		// Gradient of scalar loss w.r.t. each element of Y2.
		virtual Tensor<Value, 1> getGradient(
			Tensor<Value, 1> const &,
			Tensor<Value, 1> const &) const = 0;
		// G[i][j] = gradient of scalar loss of datapoint [i]
		// w.r.t element [j] of Y2.
		Tensor<Value, 2> getGradient(
			Tensor<Value, 2> const &Y1,
			Tensor<Value, 2> const &Y2) const {
			// TODO: This is sloppy because we don't have vstack.
			return Tensor<Value, 2>(Y2.size())
				.template applyOver<1>(
					[this](
						Tensor<Value, 1> &left,
						Tensor<Value, 1> const &y1,
						Tensor<Value, 1> const &y2) {
						left.deepCopyFrom(this->getGradient(y1, y2));
					},
					Y1,
					Y2);
		}

		virtual ~LossInterface() {}
	};
}
