// Serializes and deserializes data to/from std::iostream.
#pragma once

#include "../literal.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>

namespace Rain::Data {
	// Serializes and deserializes data to/from std::iostream.
	//
	// Serialization is performed in a single pass. Each object in the
	// serialization is re-mapped to its in-serialization location, identified by
	// a std::size_t `reference`. Serialization of any addresses must maintain
	// that the object at the address has previously been serialized already.
	// Thus, one must serialize the pointed-to location before serializing a
	// pointer, or an exception shall be thrown.
	//
	// Exceptional addresses are any that equal nullptr, which will be preserved
	// during serialization. nullptr is maintained by a `reference` with value
	// SIZE_MAX.
	//
	// Custom types may be serialize/deserialized by defining free functions for
	// them.
	class Serializer : public std::iostream {
		private:
		// Manages a subclass of iostream in case we need their functionality.
		std::unique_ptr<std::iostream> streambufManager;

		// Maps real addresses to serialization references.
		// address => (reference, size).
		std::map<char const *, std::pair<std::size_t, std::size_t>> sObjects;

		// Maps serialization references to real addresses.
		// reference => (address, size).
		std::map<std::size_t, std::pair<char *, std::size_t>> dObjects;

		// Total size of serialization data.
		std::size_t sSize = 0, dSize = 0;

		public:
		Serializer(std::streambuf *streambuf) : std::iostream(streambuf) {}
		Serializer(std::string const &filename)
				: std::iostream(nullptr),
					streambufManager(new std::fstream(
						filename,
						std::ios::in | std::ios::out | std::ios::binary)) {
			this->rdbuf(this->streambufManager->rdbuf());
		}

		// Serialization and deserialization use the stream operators. Pointer
		// addresses are converted to/from serialization references.
		template <typename Data>
		typename std::enable_if<!std::is_pointer<Data>::value, Serializer &>::type
		operator<<(Data const &data) {
			// Call free function. If this address has been serialized already, the
			// previous serialization is functionally disregarded.
			serialize(*this, data);
			this->addSObject(data);
			return *this;
		}
		template <typename Data>
		typename std::enable_if<std::is_pointer<Data>::value, Serializer &>::type
		operator<<(Data const &data) {
			auto reference = this->resolvePointer(data);
			if (reference) {
				this->write(
					reinterpret_cast<char const *>(&reference.value()),
					sizeof(std::size_t));
				this->addSObject(data);
			} else {
				// Underlying has not been serialized, throw an error.
				throw std::exception();
			}
			return *this;
		}

		private:
		// Given a pointer, resolve it to a serialization reference if possible.
		template <typename Data>
		std::optional<std::size_t> resolvePointer(Data const *data) {
			if (data == nullptr) {
				return {SIZE_MAX};
			}
			auto address = reinterpret_cast<char const *>(data);
			auto it = this->sObjects.upper_bound(address);
			if (it != this->sObjects.begin()) {
				it--;
			}
			if (
				it != this->sObjects.end() && it->first <= address &&
				address + sizeof(Data) <= it->first + it->second.second) {
				return {it->second.first + address - it->first};
			} else {
				return {};
			}
		}

		// Mark an object as serialized.
		template <typename Data>
		void addSObject(Data const &data) {
			this->sObjects[reinterpret_cast<char const *>(&data)] = {
				this->sSize, sizeof(Data)};
			this->sSize += sizeof(Data);
		}

		public:
		// Deserialization stream operators.
		template <typename Data>
		typename std::enable_if<!std::is_pointer<Data>::value, Serializer &>::type
		operator>>(Data &data) {
			deserialize(*this, data);
			this->addDObject(data);
			return *this;
		}
		template <typename Data>
		typename std::enable_if<std::is_pointer<Data>::value, Serializer &>::type
		operator>>(Data &data) {
			std::size_t reference;
			this->read(reinterpret_cast<char *>(&reference), sizeof(std::size_t));
			auto address = this->resolveReference(reference);
			if (address) {
				data = reinterpret_cast<Data>(address.value());
				this->addDObject(data);
			} else {
				// Serialization reference is malformed.
				throw std::exception();
			}
			return *this;
		}

		private:
		// Resolve a serialization reference into an optional address.
		std::optional<char *> resolveReference(std::size_t reference) {
			if (reference == SIZE_MAX) {
				return {nullptr};
			}
			auto it = this->dObjects.upper_bound(reference);
			if (it != this->dObjects.begin()) {
				it--;
			}
			if (
				it != this->dObjects.end() && it->first <= reference &&
				reference < it->first + it->second.second) {
				return {it->second.first + reference - it->first};
			} else {
				return {};
			}
		}

		// Mark an object as deserialized.
		template <typename Data>
		void addDObject(Data &data) {
			this->dObjects[this->dSize] = {
				reinterpret_cast<char *>(&data), sizeof(Data)};
			this->dSize += sizeof(Data);
		}
	};

	// Free functions for types for (de)serialization.

	// Fixed-size types and native arrays.
	template <typename Data>
	void serialize(Serializer &serializer, Data const &data) {
		serializer.write(reinterpret_cast<char const *>(&data), sizeof(Data));
	}
	template <typename Data>
	void deserialize(Serializer &serializer, Data &data) {
		serializer.read(reinterpret_cast<char *>(&data), sizeof(Data));
	}

	// // std::vector.
	// template <typename Data>
	// typename std::enable_if<!std::is_pointer<Data>::value, void>::type
	// serialize( 	Serializer &serializer, 	std::vector<Data> const &data) {
	// 	std::size_t size = data.size();
	// 	serializer.write(
	// 		reinterpret_cast<char const *>(&size), sizeof(std::size_t));
	// 	serializer.write(
	// 		reinterpret_cast<char const *>(&data[0]), sizeof(Data) * size);
	// }
	// template <typename Data>
	// typename std::enable_if<!std::is_pointer<Data>::value, void>::type
	// deserialize(Deserializer &deserializer, std::vector<Data> &data) {
	// 	std::size_t size;
	// 	deserializer.read(reinterpret_cast<char *>(&size), sizeof(std::size_t));
	// 	data.resize(size);
	// 	deserializer.read(reinterpret_cast<char *>(&data[0]), sizeof(Data) *
	// size);
	// }
	// template <typename Data>
	// typename std::enable_if<std::is_pointer<Data>::value, void>::type
	// serialize( 	Serializer &serializer, 	std::vector<Data> const &data) {
	// 	std::size_t size = data.size();
	// 	serializer.write(
	// 		reinterpret_cast<char const *>(&size), sizeof(std::size_t));
	// 	for (std::size_t i = 0; i < size; i++) {
	// 		serializer << data[i];
	// 	}
	// }
	// template <typename Data>
	// typename std::enable_if<std::is_pointer<Data>::value, void>::type
	// deserialize( 	Deserializer &deserializer, 	std::vector<Data> &data) {
	// 	std::size_t size;
	// 	deserializer.read(reinterpret_cast<char *>(&size), sizeof(std::size_t));
	// 	data.resize(size);
	// 	for (std::size_t i = 0; i < size; i++) {
	// 		deserializer >> data[i];
	// 	}
	// }

	// // std::string.
	// template <
	// 	typename CharT,
	// 	typename Traits = std::char_traits<CharT>,
	// 	typename Allocator = std::allocator<CharT>>
	// void serialize(
	// 	Serializer &serializer,
	// 	std::basic_string<CharT, Traits, Allocator> const &data) {
	// 	std::size_t size = data.size();
	// 	serializer.write(
	// 		reinterpret_cast<char const *>(&size), sizeof(std::size_t));
	// 	serializer.write(
	// 		reinterpret_cast<char const *>(&data[0]), sizeof(CharT) * size);
	// }
	// template <
	// 	typename CharT,
	// 	typename Traits = std::char_traits<CharT>,
	// 	typename Allocator = std::allocator<CharT>>
	// void deserialize(
	// 	Deserializer &deserializer,
	// 	std::basic_string<CharT, Traits, Allocator> &data) {
	// 	std::size_t size;
	// 	deserializer.read(reinterpret_cast<char *>(&size), sizeof(std::size_t));
	// 	data.resize(size);
	// 	deserializer.read(reinterpret_cast<char *>(&data[0]), sizeof(CharT) *
	// size);
	// }
}
