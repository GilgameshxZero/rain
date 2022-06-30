// Serializes and deserializes data to/from std::iostream.
#pragma once

#include "../literal.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>

// Forward-declared prototypes.
namespace Rain::Data {
	class Serializer;
	class Deserializer;
}

template <typename Data>
inline void serialize(Rain::Data::Serializer &, Data const &);
template <typename Data>
inline void deserialize(Rain::Data::Deserializer &, Data &);
template <
	class CharT,
	class Traits = std::char_traits<CharT>,
	class Allocator = std::allocator<CharT> >
inline void serialize(
	Rain::Data::Serializer &,
	std::basic_string<CharT, Traits, Allocator> const &);
template <
	class CharT,
	class Traits = std::char_traits<CharT>,
	class Allocator = std::allocator<CharT> >
inline void deserialize(
	Rain::Data::Deserializer &,
	std::basic_string<CharT, Traits, Allocator> &);
template <class Data>
inline void serialize(Rain::Data::Serializer &, std::vector<Data> const &);
template <class Data>
inline void deserialize(Rain::Data::Deserializer &, std::vector<Data> &);

namespace Rain::Data {
	// Serializes and deserializes data to/from std::iostream.
	//
	// Custom types are (de)serialized with free functions (see later).
	//
	// Memory addresses are not remapped during (de)serialization, and thus if
	// your structure contains memory addresses, it must be handled specially.
	class Serializer : public std::ostream {
		private:
		// Manages a subclass of iostream in case we need their functionality.
		std::unique_ptr<std::ostream> streambufManager;

		public:
		Serializer(std::streambuf *streambuf) : std::ostream(streambuf) {}
		Serializer(std::string const &filename)
				: std::ostream(nullptr),
					streambufManager(new std::ofstream(filename, std::ios::binary)) {
			this->rdbuf(this->streambufManager->rdbuf());
		}

		// Serialization and deserialization use the stream operators.
		template <typename Data>
		Serializer &operator<<(Data const &data) {
			// Call free function.
			::serialize(*this, data);
			return *this;
		}
	};

	class Deserializer : public std::istream {
		private:
		// Manages a subclass of iostream in case we need their functionality.
		std::unique_ptr<std::istream> streambufManager;

		public:
		Deserializer(std::streambuf *streambuf) : std::istream(streambuf) {}
		Deserializer(std::string const &filename)
				: std::istream(nullptr),
					streambufManager(new std::ifstream(filename, std::ios::binary)) {
			this->rdbuf(this->streambufManager->rdbuf());
		}

		// Deserialization stream operators.
		template <typename Data>
		Deserializer &operator>>(Data &data) {
			// Call free function.
			::deserialize(*this, data);
			return *this;
		}
	};
}

// Free functions for types for (de)serialization.

// Fixed-size types and native arrays.
template <typename Data>
inline void serialize(Rain::Data::Serializer &serializer, Data const &data) {
	serializer.write(reinterpret_cast<char const *>(&data), sizeof(Data));
}
template <typename Data>
inline void deserialize(Rain::Data::Deserializer &deserializer, Data &data) {
	deserializer.read(reinterpret_cast<char *>(&data), sizeof(Data));
}

// std::string.
template <class CharT, class Traits, class Allocator>
inline void serialize(
	Rain::Data::Serializer &serializer,
	std::basic_string<CharT, Traits, Allocator> const &data) {
	serializer << data.size();
	serializer.write(
		reinterpret_cast<char const *>(data.data()), sizeof(CharT) * data.size());
}
template <class CharT, class Traits, class Allocator>
inline void deserialize(
	Rain::Data::Deserializer &deserializer,
	std::basic_string<CharT, Traits, Allocator> &data) {
	std::size_t size;
	deserializer >> size;
	data.resize(size);
	deserializer.read(reinterpret_cast<char *>(&data[0]), sizeof(CharT) * size);
}

// std::vector.
template <class Data>
inline void serialize(
	Rain::Data::Serializer &serializer,
	std::vector<Data> const &data) {
	serializer << data.size();
	for (auto &i : data) {
		serializer << i;
	}
}
template <class Data>
inline void deserialize(
	Rain::Data::Deserializer &deserializer,
	std::vector<Data> &data) {
	std::size_t size;
	deserializer >> size;
	data.resize(size);
	for (auto &i : data) {
		deserializer >> i;
	}
}
