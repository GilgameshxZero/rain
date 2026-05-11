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

		template<
			std::size_t ORDER,
			std::enable_if<ORDER == 1 || ORDER == 2>::type * =
				nullptr>
		Tensor<Value, ORDER> asApply(
			Tensor<Value, ORDER> const &x) const {
			auto yHat{x.copy()};
			for (auto const &i : this->layer) {
				i.get()->apply(yHat);
			}
			return yHat;
		}
		// Artifacts are the layer-by-layer activations.
		template<
			std::size_t ORDER,
			std::enable_if<ORDER == 1 || ORDER == 2>::type * =
				nullptr>
		std::vector<Tensor<Value, ORDER>> asApplyWithArtifact(
			Tensor<Value, ORDER> const &x) const {
			std::vector<Tensor<Value, ORDER>> artifact;
			artifact.push_back(x.copy());
			for (auto const &i : this->layer) {
				artifact.push_back(
					i.get()->asApply(artifact.back()));
			}
			return artifact;
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
		std::vector<Tensor<Value, ORDER>> getGradient(
			LossInterface<Value> const &l,
			Tensor<Value, ORDER> const &y,
			std::vector<Tensor<Value, ORDER>> const &artifact)
			const {
			std::vector<Tensor<Value, ORDER>> gradient;
			gradient.push_back(l.getGradient(y, artifact.back()));
			for (std::size_t i{this->layer.size()}; i > 0; i--) {
				if constexpr (ORDER == 1) {
					gradient.push_back(
						gradient.back().template asMultiply<1>(
							this->layer[i - 1].get()->getGradient(
								artifact[i - 1], artifact[i]),
							{0},
							{0}));
				} else {
					auto layerGradient{
						this->layer[i - 1].get()->getGradient(
							artifact[i - 1], artifact[i])};
					gradient.push_back(
						Tensor<Value, ORDER>(
							{gradient.back().size()[0],
								layerGradient.size()[2]}));
					gradient.back().template applyOver<1>(
						[](
							Tensor<Value, 1> &left,
							Tensor<Value, 1> const &r1,
							Tensor<Value, 2> const &r2) {
							left.deepCopyFrom(
								r1.template asMultiply<1>(r2, {0}, {0}));
						},
						gradient,
						layerGradient);
				}
			}
			std::reverse(gradient.begin(), gradient.end());
			return gradient;
		}
		// Backprop gradient updates.
		template<
			std::size_t ORDER,
			std::enable_if<ORDER == 1 || ORDER == 2>::type * =
				nullptr>
		void stepWithGradient(
			LossInterface<Value> const &l,
			Tensor<Value, ORDER> const &y,
			std::vector<Tensor<Value, ORDER>> const &artifact,
			Value const &stepSize) const {
			auto gradient{
				l.getGradient(y, artifact.back()) * stepSize};
			for (std::size_t i{this->layer.size()}; i > 0; i--) {
				this->layer[i - 1].get()->stepWithGradient(
					artifact[i - 1], gradient);
				if constexpr (ORDER == 1) {
					gradient.template multiply<1>(
						this->layer[i - 1].get()->getGradient(
							artifact[i - 1], artifact[i]),
						{0},
						{0});
				} else {
					auto layerGradient{
						this->layer[i - 1].get()->getGradient(
							artifact[i - 1], artifact[i])};
					Tensor<Value, ORDER> nextGradient(
						{gradient.size()[0], layerGradient.size()[2]});
					nextGradient.template applyOver<1>(
						[](
							Tensor<Value, 1> &left,
							Tensor<Value, 1> const &r1,
							Tensor<Value, 2> const &r2) {
							left.deepCopyFrom(
								r1.template asMultiply<1>(r2, {0}, {0}));
						},
						gradient,
						layerGradient);
					gradient = nextGradient;
				}
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
