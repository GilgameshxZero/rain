// Serializes and deserializes data to/from std::iostream.
#pragma once

#include "../algorithm/bit_manipulators.hpp"

#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace Rain::Data {
	template<typename, typename = void>
	class SerializerSpec;
	template<typename, typename = void>
	class DeserializerSpec;

	// Serializes and deserializes data to/from std::iostream.
	//
	// Define custom behavior via template overloading.
	//
	// Memory addresses are not remapped during
	// (de)serialization, and thus if your structure contains
	// memory addresses, it must be handled specially.
	class Serializer {
		public:
		std::ostream &stream;

		explicit Serializer(std::ostream &stream) :
			stream{stream} {}

		// Const and non-const overloads for "one-time" data.
		template<
			typename Type,
			typename std::enable_if<Functional::TypeTrait<
				Functional::TypeDowngrade<SerializerSpec>>::
					IsSpecifiedBy<Type>::value>::type * = nullptr>
		inline auto &operator<<(Type &data) {
			return SerializerSpec<Type>::operate(*this, data);
		}
		template<
			typename Type,
			typename std::enable_if<Functional::TypeTrait<
				Functional::TypeDowngrade<SerializerSpec>>::
					IsSpecifiedBy<Type>::value>::type * = nullptr>
		inline auto &operator<<(Type const &data) {
			return SerializerSpec<Type>::operate(*this, data);
		}
		template<
			typename Type,
			typename std::enable_if<
				!Functional::TypeTrait<Functional::TypeDowngrade<
					SerializerSpec>>::IsSpecifiedBy<Type>::value &&
				!std::is_array<Type>::value>::type * = nullptr>
		inline auto &operator<<(Type const &data) {
			Algorithm::writeBytes(
				this->stream, data, std::endian::little);
			return *this;
		}
		// Arrays need a special handler esp. if the native
		// endian is not little.
		template<
			typename Type,
			typename std::enable_if<
				!Functional::TypeTrait<Functional::TypeDowngrade<
					SerializerSpec>>::IsSpecifiedBy<Type>::value &&
				std::is_array<Type>::value>::type * = nullptr>
		inline auto &operator<<(Type const &data) {
			if constexpr (
				std::endian::native == std::endian::little) {
				Algorithm::writeBytes(
					this->stream, data, std::endian::little);
			} else {
				for (auto const &i : data) {
					*this << i;
				}
			}
			return *this;
		}
	};
	class Deserializer {
		public:
		template<typename>
		static std::false_type hasConstruct(...);
		template<typename Spec>
		static std::true_type hasConstruct(
			decltype(Spec::construct(
				std::declval<Deserializer &>())) *);
		template<typename Spec>
		using HasConstruct =
			decltype(hasConstruct<Spec>(nullptr));

		std::istream &stream;

		explicit Deserializer(std::istream &stream) :
			stream{stream} {}

		// Standard deserializers require the target object to
		// possibly be in an invalid state before the function
		// is called.
		template<
			typename Type,
			typename std::enable_if<Functional::TypeTrait<
				Functional::TypeDowngrade<DeserializerSpec>>::
					IsSpecifiedBy<Type>::value>::type * = nullptr>
		inline auto &operator>>(Type &data) {
			return DeserializerSpec<Type>::operate(*this, data);
		}
		template<
			typename Type,
			typename std::enable_if<
				!Functional::TypeTrait<Functional::TypeDowngrade<
					DeserializerSpec>>::IsSpecifiedBy<Type>::value &&
				!std::is_array<Type>::value>::type * = nullptr>
		inline auto &operator>>(Type &data) {
			Algorithm::readBytes(
				this->stream, data, std::endian::little);
			return *this;
		}
		// Arrays need a special handler esp. if the native
		// endian is not little.
		template<
			typename Type,
			typename std::enable_if<
				!Functional::TypeTrait<Functional::TypeDowngrade<
					DeserializerSpec>>::IsSpecifiedBy<Type>::value &&
				std::is_array<Type>::value>::type * = nullptr>
		inline auto &operator>>(Type &data) {
			if constexpr (
				std::endian::native == std::endian::little) {
				Algorithm::readBytes(
					this->stream, data, std::endian::little);
			} else {
				for (auto &i : data) {
					*this >> i;
				}
			}
			return *this;
		}

		// Preferred RAII deserializers will construct the
		// object from the stream.
		//
		// Because the type may be abstract and require a
		// derived type's instantiation, this pattern must
		// return a pointer.
		//
		// Only override the default construct if the construct
		// function exists.
		template<
			typename Type,
			typename std::enable_if<
				Functional::TypeTrait<Functional::TypeDowngrade<
					DeserializerSpec>>::IsSpecifiedBy<Type>::value &&
				HasConstruct<DeserializerSpec<Type>>::value>::type
				* = nullptr>
		inline std::unique_ptr<Type> construct() {
			return DeserializerSpec<Type>::construct(*this);
		}
		template<
			typename Type,
			typename std::enable_if<
				!Functional::TypeTrait<Functional::TypeDowngrade<
					DeserializerSpec>>::IsSpecifiedBy<Type>::value ||
				!HasConstruct<DeserializerSpec<Type>>::value>::type
				* = nullptr>
		inline std::unique_ptr<Type> construct() {
			std::unique_ptr<Type> pType(new Type);
			*this >> *pType.get();
			return pType;
		}
	};

	// Joint serializer/deserializer.
	class BiSerializer :
		public Serializer,
		public Deserializer {
		public:
		BiSerializer(std::iostream &stream) :
			Serializer(stream),
			Deserializer(stream) {}
	};

	// std::shared_ptr.
	template<typename Type>
	class SerializerSpec<std::shared_ptr<Type>, void> {
		public:
		static auto &operate(
			Serializer &serializer,
			std::shared_ptr<Type> const &data) {
			return serializer << *data.get();
		}
	};
	template<typename Type>
	class DeserializerSpec<std::shared_ptr<Type>, void> {
		public:
		// std::shared_ptr must already be allocated. We do not
		// allocate since sometimes T is abstract.
		static auto &operate(
			Deserializer &deserializer,
			std::shared_ptr<Type> &data) {
			return deserializer >> *data.get();
		}
	};

	// std::string.
	template<
		typename CharT,
		typename Traits,
		typename Allocator>
	class SerializerSpec<
		std::basic_string<CharT, Traits, Allocator>,
		void> {
		public:
		static auto &operate(
			Serializer &serializer,
			std::basic_string<CharT, Traits, Allocator> const
				&data) {
			serializer << data.size();
			serializer.stream.write(
				reinterpret_cast<char const *>(data.data()),
				sizeof(CharT) * data.size());
			return serializer;
		}
	};
	template<
		typename CharT,
		typename Traits,
		typename Allocator>
	class DeserializerSpec<
		std::basic_string<CharT, Traits, Allocator>,
		void> {
		public:
		static auto &operate(
			Deserializer &deserializer,
			std::basic_string<CharT, Traits, Allocator> &data) {
			std::size_t size;
			deserializer >> size;
			data.resize(size);
			deserializer.stream.read(
				reinterpret_cast<char *>(&data[0]),
				sizeof(CharT) * size);
			return deserializer;
		}
	};

	// std::vector.
	template<typename T, typename Allocator>
	class SerializerSpec<std::vector<T, Allocator>, void> {
		public:
		static auto &operate(
			Serializer &serializer,
			std::vector<T, Allocator> const &data) {
			serializer << data.size();
			for (auto &i : data) {
				serializer << i;
			}
			return serializer;
		}
	};
	template<typename T, typename Allocator>
	class DeserializerSpec<std::vector<T, Allocator>, void> {
		public:
		static auto &operate(
			Deserializer &deserializer,
			std::vector<T, Allocator> &data) {
			std::size_t size;
			deserializer >> size;
			data.resize(size);
			for (auto &i : data) {
				deserializer >> i;
			}
			return deserializer;
		}
	};

	// std::set.
	template<
		typename Key,
		typename Compare,
		typename Allocator>
	class SerializerSpec<
		std::set<Key, Compare, Allocator>,
		void> {
		public:
		static auto &operate(
			Serializer &serializer,
			std::set<Key, Compare, Allocator> const &data) {
			serializer << data.size();
			for (auto &i : data) {
				serializer << i;
			}
			return serializer;
		}
	};
	template<
		typename Key,
		typename Compare,
		typename Allocator>
	class DeserializerSpec<
		std::set<Key, Compare, Allocator>,
		void> {
		public:
		// std::shared_ptr must already be allocated. We do
		// not allocate since sometimes T is abstract.
		static auto &operate(
			Deserializer &deserializer,
			std::set<Key, Compare, Allocator> &data) {
			std::size_t size;
			deserializer >> size;
			Key key;
			for (std::size_t i{0}; i < size; i++) {
				deserializer >> key;
				data.insert(key);
			}
			return deserializer;
		}
	};

	// std::unordered_set.
	template<
		typename Key,
		typename Hash,
		typename KeyEqual,
		typename Allocator>
	class SerializerSpec<
		std::unordered_set<Key, Hash, KeyEqual, Allocator>,
		void> {
		public:
		static auto &operate(
			Serializer &serializer,
			std::
				unordered_set<Key, Hash, KeyEqual, Allocator> const
					&data) {
			serializer << data.size();
			for (auto &i : data) {
				serializer << i;
			}
			return serializer;
		}
	};
	template<
		typename Key,
		typename Hash,
		typename KeyEqual,
		typename Allocator>
	class DeserializerSpec<
		std::unordered_set<Key, Hash, KeyEqual, Allocator>,
		void> {
		public:
		// std::shared_ptr must already be allocated. We do
		// not allocate since sometimes T is abstract.
		static auto &operate(
			Deserializer &deserializer,
			std::unordered_set<Key, Hash, KeyEqual, Allocator>
				&data) {
			std::size_t size;
			deserializer >> size;
			Key key;
			for (std::size_t i{0}; i < size; i++) {
				deserializer >> key;
				data.insert(key);
			}
			return deserializer;
		}
	};
}
