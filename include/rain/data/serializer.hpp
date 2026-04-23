// Serializes and deserializes data to/from std::iostream.
#pragma once

#include "../literal.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace Rain::Data {
	class Serializer;
	class Deserializer;

	template <typename Data>
	struct serialize;
	template <typename Data>
	struct deserialize;

	// Serializes and deserializes data to/from std::iostream.
	//
	// Custom types are (de)serialized with free functions
	// (see later).
	//
	// Memory addresses are not remapped during
	// (de)serialization, and thus if your structure contains
	// memory addresses, it must be handled specially.
	class Serializer : public std::ostream {
		private:
		// Manages a subclass of iostream in case we need their
		// functionality.
		std::unique_ptr<std::ostream> streambufManager;

		public:
		Serializer(std::streambuf *streambuf)
				: std::ostream(streambuf) {}
		Serializer(std::string const &filename)
				: std::ostream(nullptr),
					streambufManager(
						new std::ofstream(filename, std::ios::binary)) {
			this->rdbuf(this->streambufManager->rdbuf());
		}

		// Operators redirect to a template type which can be
		// overloaded, and provides a default.
		template <typename Data>
		inline Rain::Data::Serializer &operator<<(
			Data const &data) {
			serialize<Data>()(*this, data);
			return *this;
		}
		template <
			typename CharT,
			typename Traits,
			typename Allocator>
		inline Rain::Data::Serializer &operator<<(
			std::basic_string<CharT, Traits, Allocator> const
				&data) {
			*this << data.size();
			this->write(
				reinterpret_cast<char const *>(data.data()),
				sizeof(CharT) * data.size());
			return *this;
		}
		template <typename T, typename Allocator>
		inline Rain::Data::Serializer &operator<<(
			std::vector<T, Allocator> const &data) {
			*this << data.size();
			for (auto &i : data) {
				*this << i;
			}
			return *this;
		}
		template <
			typename Key,
			typename Compare,
			typename Allocator>
		inline Rain::Data::Serializer &operator<<(
			std::set<Key, Compare, Allocator> const &data) {
			*this << data.size();
			for (auto &i : data) {
				*this << i;
			}
			return *this;
		}
		template <
			typename Key,
			typename Hash,
			typename KeyEqual,
			typename Allocator>
		inline Rain::Data::Serializer &operator<<(
			std::
				unordered_set<Key, Hash, KeyEqual, Allocator> const
					&data) {
			*this << data.size();
			for (auto &i : data) {
				*this << i;
			}
			return *this;
		}
	};

	class Deserializer : public std::istream {
		private:
		// Manages a subclass of iostream in case we need their
		// functionality.
		std::unique_ptr<std::istream> streambufManager;

		public:
		Deserializer(std::streambuf *streambuf)
				: std::istream(streambuf) {}
		Deserializer(std::string const &filename)
				: std::istream(nullptr),
					streambufManager(
						new std::ifstream(filename, std::ios::binary)) {
			this->rdbuf(this->streambufManager->rdbuf());
		}

		// Operators redirect to a template type which can be
		// overloaded, and provides a default.
		template <typename Data>
		inline Rain::Data::Deserializer &operator>>(
			Data &data) {
			deserialize<Data>()(*this, data);
			return *this;
		}
		template <
			typename CharT,
			typename Traits,
			typename Allocator>
		inline Rain::Data::Deserializer &operator>>(
			std::basic_string<CharT, Traits, Allocator> &data) {
			std::size_t size;
			*this >> size;
			data.resize(size);
			this->read(
				reinterpret_cast<char *>(&data[0]),
				sizeof(CharT) * size);
			return *this;
		}
		template <typename T, typename Allocator>
		inline Rain::Data::Deserializer &operator>>(
			std::vector<T, Allocator> &data) {
			std::size_t size;
			*this >> size;
			data.resize(size);
			for (auto &i : data) {
				*this >> i;
			}
			return *this;
		}
		template <
			typename Key,
			typename Compare,
			typename Allocator>
		inline Rain::Data::Deserializer &operator>>(
			std::set<Key, Compare, Allocator> &data) {
			std::size_t size;
			*this >> size;
			Key key;
			for (std::size_t i{0}; i < size; i++) {
				*this >> key;
				data.insert(key);
			}
			return *this;
		}
		template <
			typename Key,
			typename Hash,
			typename KeyEqual,
			typename Allocator>
		inline Rain::Data::Deserializer &operator>>(
			std::unordered_set<Key, Hash, KeyEqual, Allocator>
				&data) {
			std::size_t size;
			*this >> size;
			Key key;
			for (std::size_t i{0}; i < size; i++) {
				*this >> key;
				data.insert(key);
			}
			return *this;
		}
	};

	// Use separate serialize/deserialize types instead of
	// operator overloading. This is because operator
	// overloading fails to respect definition order when
	// using a variety of custom types, but type overloading
	// does, and so supports more flexible overloading.
	template <typename Data>
	struct serialize {
		void operator()(
			Serializer &serializer,
			Data const &data) {
			serializer.write(
				reinterpret_cast<char const *>(&data),
				sizeof(Data));
		}
	};
	template <typename Data>
	struct deserialize {
		void operator()(
			Deserializer &deserializer,
			Data &data) {
			deserializer.read(
				reinterpret_cast<char *>(&data), sizeof(Data));
		}
	};
}
