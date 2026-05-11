#pragma once

#include "../../data/serializer.hpp"
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
			// TODO: This is sloppy because we don't have vstack.
			auto firstResult{this->asApply(X[0])};
			Tensor<Value, 2> result(
				{X.size()[0], firstResult.size()[0]});
			result[0].deepCopyFrom(firstResult);
			for (std::size_t i{1}; i < X.size()[0]; i++) {
				result[i].deepCopyFrom(this->asApply(X[i]));
			}
			return X = result;
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
		auto getGradient(
			Tensor<Value, 2> const &z1,
			Tensor<Value, 2> const &z2) const {
			// TODO: This is sloppy because we don't have vstack.
			return Tensor<Value, 3>(
				{z1.size()[0], z2.size()[1], z1.size()[1]})
				.template applyOver<2>(
					[this](
						Tensor<Value, 2> &left,
						Tensor<Value, 1> const &y1,
						Tensor<Value, 1> const &y2) {
						left.deepCopyFrom(this->getGradient(y1, y2));
					},
					z1,
					z2);
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
			Tensor<Value, 1> const &) {}
		virtual void stepWithGradient(
			Tensor<Value, 2> const &,
			Tensor<Value, 2> const &) {}

		virtual Data::Serializer &serialize(
			Data::Serializer &serializer) const {
			return serializer;
		}
		virtual Data::Deserializer &deserialize(
			Data::Deserializer &deserializer) {
			return deserializer;
		}

		virtual ~ActivationInterface() {}

		// Data::Serializer.
		class Serializer {
			public:
			static auto &serialize(
				Data::Serializer &serializer,
				ActivationInterface const *data) {
				return data->serialize(serializer);
			}
			static auto &deserialize(
				Data::Deserializer &deserializer,
				ActivationInterface *data) {
				return data->deserialize(deserializer);
			}
		};
	};
}
