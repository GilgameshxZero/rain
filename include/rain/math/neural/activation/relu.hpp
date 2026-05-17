#pragma once

#include "../../../algorithm/algorithm.hpp"
#include "../activation_interface.hpp"

namespace Rain::Math::Neural::Activation {
	template<typename Value>
	class Relu : virtual public ActivationInterface<Value> {
		public:
		virtual Tensor<Value, 1> apply(
			Tensor<Value, 1> &z1) const override {
			return z1.template applyOver<0>([](Value &left) {
				left = std::max(Value{}, left);
			});
		}
		virtual Tensor<Value, 2> getIncrementalGradient(
			Tensor<Value, 1> const &z1,
			Tensor<Value, 1> const &) const override {
			return z1
				.template asApplyOver<0>([](Value &left) {
					left = left > Value{} ? Value{1} : Value{};
				})
				.asDiagonal();
		}
	};
}
