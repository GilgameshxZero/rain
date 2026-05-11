#pragma once

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

		virtual Tensor<Value, 1> asApply(
			Tensor<Value, 1> const &x) const override final {
			auto yHat{x.copy()};
			for (auto const &i : this->layer) {
				i.get()->apply(yHat);
			}
			return yHat;
		}
		// Artifacts are the layer-by-layer activations.
		std::vector<Tensor<Value, 1>> asApplyWithArtifact(
			Tensor<Value, 1> const &x) const {
			std::vector<Tensor<Value, 1>> artifact;
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
		std::vector<Tensor<Value, 1>> getGradient(
			LossInterface<Value> const &l,
			std::vector<Tensor<Value, 1>> const &artifact) const {
			std::vector<Tensor<Value, 1>> gradient;
			gradient.push_back(l.getGradient(artifact.back()));
			for (std::size_t i{this->layer.size()}; i > 0; i--) {
				gradient.push_back(
					gradient.back().template asMultiply<1>(
						this->layer[i - 1].get()->getGradient(
							artifact[i - 1], artifact[i]),
						{0},
						{0}));
			}
			std::reverse(gradient.begin(), gradient.end());
			return gradient;
		}
		// Backprop gradient updates.
		void stepWithGradient(
			LossInterface<Value> const &l,
			std::vector<Tensor<Value, 1>> const &artifact,
			Value const &stepSize) const {
			auto gradient{
				l.getGradient(artifact.back()) * stepSize};
			for (std::size_t i{this->layer.size()}; i > 0; i--) {
				this->layer[i - 1].get()->stepWithGradient(
					artifact[i - 1], gradient);
				gradient.template multiply<1>(
					this->layer[i - 1].get()->getGradient(
						artifact[i - 1], artifact[i]),
					{0},
					{0});
			}
		}
	};
}
