#pragma once

#include "../../../data/serializer.hpp"
#include "../activation_interface.hpp"
#include "../loss_interface.hpp"
#include "../network_interface.hpp"

namespace Rain::Math::Neural::Network {
	template<typename Value>
	class FeedForward : public NetworkInterface<Value> {
		public:
		std::vector<std::shared_ptr<ActivationInterface<Value>>>
			layer;

		FeedForward(
			std::vector<
				std::shared_ptr<ActivationInterface<Value>>> const
				&layer) :
			layer{layer} {}

		// Returns the layer-by-layer activations.
		template<
			std::size_t ORDER,
			std::enable_if<ORDER == 1 || ORDER == 2>::type * =
				nullptr>
		std::vector<Tensor<Value, ORDER>> asApply(
			Tensor<Value, ORDER> const &x) const {
			std::vector<Tensor<Value, ORDER>> activation;
			activation.push_back(x.copy());
			for (auto const &i : this->layer) {
				activation.push_back(
					i.get()->asApply(activation.back()));
			}
			return activation;
		}

		// Backprop gradient given loss function (including
		// target labels) and artifacts.
		//
		// Returns gradient, where gradient[0] is d(loss) /
		// d(x), and gradient[layers.size()] = d(loss) /
		// d(yHat).
		template<
			std::size_t ORDER,
			std::enable_if<ORDER == 1 || ORDER == 2>::type * =
				nullptr>
		std::vector<Tensor<Value, ORDER>> getActivationGradient(
			LossInterface<Value> const &l,
			Tensor<Value, ORDER> const &y,
			std::vector<Tensor<Value, ORDER>> const &activation)
			const {
			std::vector<Tensor<Value, ORDER>> activationGradient;
			activationGradient.push_back(
				l.getGradient(y, activation.back()));
			for (std::size_t i{this->layer.size()}; i > 0; i--) {
				if constexpr (ORDER == 1) {
					activationGradient.push_back(
						activationGradient.back()
							.template asMultiply<1>(
								this->layer[i - 1]
									.get()
									->getIncrementalGradient(
										activation[i - 1], activation[i]),
								{0},
								{0}));
				} else {
					auto incrementalGradient{this->layer[i - 1]
							.get()
							->getIncrementalGradient(
								activation[i - 1], activation[i])};
					activationGradient.push_back(
						Tensor<Value, ORDER>(
							{activationGradient.back().size()[0],
								incrementalGradient.size()[2]}));
					activationGradient.back().template applyOver<1>(
						[](
							Tensor<Value, 1> &left,
							Tensor<Value, 1> const &r1,
							Tensor<Value, 2> const &r2) {
							left.deepCopyFrom(
								r1.template asMultiply<1>(r2, {0}, {0}));
						},
						activationGradient
							[activationGradient.size() - 2],
						incrementalGradient);
				}
			}
			std::reverse(
				activationGradient.begin(),
				activationGradient.end());
			return activationGradient;
		}
		// Backprop gradient updates.
		template<
			std::size_t ORDER,
			std::enable_if<ORDER == 1 || ORDER == 2>::type * =
				nullptr>
		void stepWithActivationGradient(
			std::vector<Tensor<Value, ORDER>> const &activation,
			std::vector<Tensor<Value, ORDER>> const
				&activationGradient,
			Value const &stepSize) const {
			for (std::size_t i{0}; i < this->layer.size(); i++) {
				this->layer[i].get()->stepWithGradient(
					activation[i],
					activationGradient[i + 1] * stepSize);
			}
		}

		virtual Data::Serializer &serialize(
			Data::Serializer &serializer) const override {
			for (auto const &i : this->layer) {
				serializer << i;
			}
			return serializer;
		}
		virtual Data::Deserializer &deserialize(
			Data::Deserializer &deserializer) override {
			for (auto &i : this->layer) {
				deserializer >> i;
			}
			return deserializer;
		}
	};
}
