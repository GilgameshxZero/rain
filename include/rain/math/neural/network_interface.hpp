#pragma once

#include "../../data/serializer.hpp"
#include "activation_interface.hpp"

namespace Rain::Math::Neural {
	template<typename Value>
	class NetworkInterface {
		public:
		// Hard to decide what a general network interface
		// should look like.
		//
		// TODO.

		virtual Data::Serializer &serialize(
			Data::Serializer &) const = 0;
		virtual Data::Deserializer &deserialize(
			Data::Deserializer &) = 0;

		virtual ~NetworkInterface() {}
	};
}

namespace Rain::Data {
	template<typename Type>
	class SerializerSpec<
		Type,
		typename std::enable_if<
			Functional::TypeTrait<Functional::TypeDowngrade<
				Math::Neural::NetworkInterface>>::
				IsTemplateBaseOf<Type>::value>::type> {
		public:
		template<typename Value>
		static auto &serialize(
			Data::Serializer &serializer,
			Math::Neural::NetworkInterface<Value> const &data) {
			return data.serialize(serializer);
		}
	};
	template<typename Type>
	class DeserializerSpec<
		Type,
		typename std::enable_if<
			Functional::TypeTrait<Functional::TypeDowngrade<
				Math::Neural::NetworkInterface>>::
				IsTemplateBaseOf<Type>::value>::type> {
		public:
		template<typename Value>
		static auto &deserialize(
			Data::Deserializer &deserializer,
			Math::Neural::NetworkInterface<Value> &data) {
			return data.deserialize(deserializer);
		}
	};
}
