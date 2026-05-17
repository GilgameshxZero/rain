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

		// Force deep-copy for W/B. Default constructor is
		// invalid but helps with late initialization when
		// required (e.g. serializer).
		Linear(
			Tensor<Value, 2> const &weight = Tensor<Value, 2>(),
			Tensor<Value, 1> const &bias = Tensor<Value, 1>()) :
			weight{weight.copy()},
			bias{bias.copy()} {}

		virtual Tensor<Value, 1> apply(
			Tensor<Value, 1> &z1) const override {
			return z1.template multiply<1>(
							 this->weight, {0}, {0}) +
				this->bias;
		}
		virtual Tensor<Value, 2> getIncrementalGradient(
			Tensor<Value, 1> const &,
			Tensor<Value, 1> const &) const override {
			return this->weight.asTranspose({1, 0});
		}
		virtual void stepWithGradient(
			Tensor<Value, 2> const &z1,
			Tensor<Value, 2> const &gradient) override {
			this->weight = this->weight -
				z1.template asMultiply<1>(gradient, {0}, {0}) /
					z1.size()[0];
			this->bias = this->bias -
				gradient.asContract(
					0, [](Tensor<Value, 1> const &right) {
						return right.mean();
					});
		}

		virtual Data::Serializer &serialize(
			Data::Serializer &serializer) const override {
			return serializer << this->weight << this->bias;
		}
		virtual Data::Deserializer &deserialize(
			Data::Deserializer &deserializer) override {
			return deserializer >> this->weight >> this->bias;
		}
	};
}
