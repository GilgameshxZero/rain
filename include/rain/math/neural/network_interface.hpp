#pragma once

#include "../../data/serializer.hpp"
#include "activation_interface.hpp"

namespace Rain::Math::Neural {
	template<typename Value>
	class NetworkInterface {
		public:
		virtual Tensor<Value, 1> asApply(
			Tensor<Value, 1> const &) const = 0;

		virtual Data::Serializer &serialize(
			Data::Serializer &) const = 0;
		virtual Data::Deserializer &deserialize(
			Data::Deserializer &) = 0;

		virtual ~NetworkInterface() {}

		// Data::Serializer.
		class Serializer {
			public:
			static auto &serialize(
				Data::Serializer &serializer,
				NetworkInterface const *data) {
				return data->serialize(serializer);
			}
			static auto &deserialize(
				Data::Deserializer &deserializer,
				NetworkInterface *data) {
				return data->deserialize(deserializer);
			}
		};
	};
}
