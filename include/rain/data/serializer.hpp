// Serializes and deserializes data to/from std::iostream.
#pragma once

#include "../string.hpp"

#include <iostream>
#include <map>
#include <optional>

namespace Rain::Data {
	// Serializes and deserializes data to/from std::iostream.
	//
	// Each serialized variable is preceded by a header:
	// header {
	// 	char type;
	// 	std::size_t? reference;
	// }
	// Based on the type, reference may or may not exist.
	// switch type {
	// 	case 0x00: Following are raw bytes for this variable.
	// 	case 0x01: reference is a "serialization reference", pointing to the
	// memory location in serialization-order of a variable.
	// }
	// References are computed single-pass during serialization/deserialization.
	//
	// Type is not preserved, and must be specified during deserialization.
	//
	// Pointers and references will have their underlying memory (de)serialized,
	// unless their type is void *, or the pointed-to address is nullptr. No
	// memory frees/allocations will be made during (de)serialization. If an
	// underlying memory location is not explicitly (de)serialized prior, the
	// pointer which will point to it must be exlicitly (de)allocated outside of
	// deserialization.
	//
	// Serializing a pointer first serializes the underlying (which may end up
	// just being a reference to a previously-serialized location), then
	// serializes the pointer itself. The pointer address is serialized with
	// reference {} structures.
	//
	// Custom types may be serialize/deserialized by defining free functions for
	// them.
	class Serializer {
		private:
		std::ostream &stream;

		// {memory beginning, size} => serialization reference.
		// Maps a memory range to the serialization reference.
		// Memory ranges must not overlap, or an exception will be thrown.
		std::map<std::pair<void const *, std::size_t>, std::size_t> objects;

		// Tracks serialization total size.
		std::size_t size;

		public:
		Serializer(std::ostream &stream) : stream(stream), size(0_zu) {}

		// Write to the serialization stream, to be used by free functions.
		void write(char const *s, std::streamsize count) {
			this->stream.write(s, count);
		}

		private:
		// Given a memory range, retrieve its serialization reference if available.
		// Otherwise, returns nothing, or throws an exception if range overlap with
		// a previous non-enclosing range.
		std::optional<std::size_t> getReference(
			void const *location,
			std::size_t size) {
			auto it = this->objects.upper_bound({location, SIZE_MAX});
			if (it != this->objects.begin()) {
				it--;
			}

			// Ensure that we are in range of a previously serialized block.
			if (
				it != this->objects.end() && it->first.first <= location &&
				reinterpret_cast<char const *>(it->first.first) + it->first.second >
					reinterpret_cast<char const *>(location)) {
				// Ensure that ranges don't overlap.
				if (
					reinterpret_cast<char const *>(location) + size <=
					reinterpret_cast<char const *>(it->first.first) + it->first.second) {
					// Compute offset from block begin.
					return {
						it->second + reinterpret_cast<char const *>(location) -
						reinterpret_cast<char const *>(it->first.first)};
				} else {
					throw std::exception();
				}
			} else {
				// No existing block.
				return {};
			}
		}

		// Given a data type, if it is a pointer, interpret the next bytes as a
		// serialization reference; otherwise, interpret the next bytes raw.
		template <typename Data>
		typename std::enable_if<std::is_pointer<Data>::value, void>::type
		serializeBytes(Data const &data) {
			// If Data is a pointer, convert the pointer location to a serialization
			// reference.
			// The underlying is guaranteed to have been serialized already.
			auto reference = this->getReference(
				reinterpret_cast<void const *>(data), sizeof(void const *));
			this->write(
				reinterpret_cast<char const *>(&reference.value()),
				sizeof(std::size_t));
		}
		template <typename Data>
		typename std::enable_if<!std::is_pointer<Data>::value, void>::type
		serializeBytes(Data const &data) {
			// Otherwise, call the free function for this type.
			serialize(*this, data);
		}

		// Helper for serialization functions for both pointers and references.
		template <typename Data>
		void serializeWithHeader(Data const &data) {
			auto reference =
				this->getReference(reinterpret_cast<void const *>(&data), sizeof(Data));

			if (reference) {
				// This is previously serialized memory, so reference it.
				this->write("\x01", 1_zu);
				this->write(
					reinterpret_cast<char const *>(&reference.value()),
					sizeof(std::size_t));
			} else {
				// This is a new memory block.
				this->write("\x00", 1_zu);
				this->objects[{reinterpret_cast<void const *>(&data), sizeof(Data)}] =
					this->size;
				this->size += sizeof(Data);
				this->serializeBytes(data);
			}
		}

		public:
		// Writes header for data, and conditionally calls serialize free function
		// for the type.
		template <typename Data>
		typename std::enable_if<!std::is_pointer<Data>::value, Serializer &>::type
		operator<<(Data const &data) {
			this->serializeWithHeader(data);
			return *this;
		}

		// Serializing a pointer first serializes the underlying, then serializes
		// the pointer itself.
		template <typename Data>
		Serializer &operator<<(Data const *data) {
			*this << *data;
			this->serializeWithHeader(data);
			return *this;
		}
	};

	class Deserializer {
		private:
		std::istream &stream;

		// serialization reference => {begin, size}.
		// Maps references in the serialization to their in-memory range.
		std::map<std::size_t, std::pair<void *, std::size_t>> objects;

		// Tracks size of deserialization.
		std::size_t size;

		public:
		Deserializer(std::istream &stream) : stream(stream), size(0_zu) {}

		// Read from the serialization stream, to be used by free functions.
		void read(char *s, std::streamsize count) { this->stream.read(s, count); }

		private:
		// Resolve a serialization reference into a deserialized memory location, or
		// returns nothing.
		std::optional<void *> resolveReference(std::size_t reference) {
			auto it = this->objects.upper_bound(reference);
			if (it != this->objects.begin()) {
				it--;
			}
			if (it != this->objects.end()) {
				std::size_t offset = reference - it->first;
				if (it->second.second > reference - it->first) {
					return {reinterpret_cast<void *>(
						reinterpret_cast<char *>(it->second.first) + reference -
						it->first)};
				} else {
					throw std::exception();
				}
			}
			return {};
		}

		// Overloads for pointer/non-pointer Data types.
		template <typename Data>
		typename std::enable_if<std::is_pointer<Data>::value, void>::type
		deserializeBytes(Data &data) {
			// If Data is a pointer, read the next bytes as a serialization
			// reference.
			std::size_t reference;
			this->read(reinterpret_cast<char *>(&reference), sizeof(std::size_t));
			data = reinterpret_cast<Data>(this->resolveReference(reference).value());
		}
		template <typename Data>
		typename std::enable_if<!std::is_pointer<Data>::value, void>::type
		deserializeBytes(Data &data) {
			// Otherwise, call the free function to deserialize into reference.
			deserialize(*this, data);
		}

		template <typename Data>
		typename std::enable_if<std::is_pointer<Data>::value, void>::type
		pointToReference(Data &data, std::size_t reference) {
			// If Data is a pointer, read the next bytes as a serialization
			// reference.
			data = reinterpret_cast<Data>(this->resolveReference(reference).value());
		}
		template <typename Data>
		typename std::enable_if<!std::is_pointer<Data>::value, void>::type
		pointToReference(Data &, std::size_t) {
			// Do nothing if underlying is not pointer.
		}

		// switch type {
		// 	case 0x00: Deserializes into the reference. Returns nullptr.
		// 	case 0x01: Returns the location of the previously deserialized data.
		// }
		template <typename Data>
		void deserializeWithHeader(Data &data) {
			char type;
			this->read(&type, 1_zu);
			switch (type) {
				case '\x00':
				default: {
					// This is a new object.
					this->objects[this->size] = {
						reinterpret_cast<void *>(&data), sizeof(Data)};
					this->size += sizeof(Data);
					this->deserializeBytes(data);
					break;
				}

				case '\x01': {
					// Read reference and set data if Data is a pointer.
					std::size_t reference;
					this->read(reinterpret_cast<char *>(&reference), sizeof(std::size_t));
					this->pointToReference(data, reference);
					break;
				}
			}
		}

		public:
		// Deserializing to a reference depends on the type in the header.
		// switch type {
		// 	case 0x00: The raw bytes will be deserialized into the reference.
		// 	case 0x01: Nothing needs to be done, since underlying bytes are
		// already serialized.
		// }
		template <typename Data>
		Deserializer &operator>>(Data &data) {
			this->deserializeWithHeader(data);
			return *this;
		}

		private:
		template <typename Data>
		Deserializer &deserializePointer(Data *&data) {
			*this >> *data;
			this->deserializeWithHeader(data);
			return *this;
		}

		public:
		// Deserializing to a pointer depends on the type in the header.
		// switch type {
		// 	case 0x00: The pointer must point to valid allocated memory, and the raw
		// bytes will be deserialized into that location.
		// 	case 0x01: The pointer will be pointed to the previously deserialized
		// block.
		// }
		// In any case, the pointer must be deserialized in reverse order of its
		// serialization. Deserialize the underlying first, then deserialize the
		// pointer itself.
		template <typename Data>
		typename std::enable_if<!std::is_pointer<Data>::value, Deserializer &>::type
		operator>>(Data *&data) {
			return this->deserializePointer(data);
		}

		template <typename Data>
		typename std::enable_if<std::is_pointer<Data>::value, Deserializer &>::type
		operator>>(Data *&data) {
			// data is at least a 2-layer pointer. To enable recursive unwrapping,
			// temporarily allocate the pointer here, then delete later.
			std::unique_ptr<Data> tmp(new Data);
			data = tmp.get();
			return this->deserializePointer(data);
		}
	};

	// Free functions for types for (de)serialization only need to be defined for
	// reference types.

	// Fixed-size types and native arrays.
	template <typename Data>
	void serialize(Serializer &serializer, Data const &data) {
		serializer.write(reinterpret_cast<char const *>(&data), sizeof(Data));
	}
	template <typename Data>
	void deserialize(Deserializer &deserializer, Data &data) {
		deserializer.read(reinterpret_cast<char *>(&data), sizeof(Data));
	}

	// std::vector.
	template <typename Data>
	typename std::enable_if<!std::is_pointer<Data>::value, void>::type serialize(
		Serializer &serializer,
		std::vector<Data> const &data) {
		std::size_t size = data.size();
		serializer.write(
			reinterpret_cast<char const *>(&size), sizeof(std::size_t));
		serializer.write(
			reinterpret_cast<char const *>(&data[0]), sizeof(Data) * size);
	}
	template <typename Data>
	typename std::enable_if<!std::is_pointer<Data>::value, void>::type
	deserialize(Deserializer &deserializer, std::vector<Data> &data) {
		std::size_t size;
		deserializer.read(reinterpret_cast<char *>(&size), sizeof(std::size_t));
		data.resize(size);
		deserializer.read(reinterpret_cast<char *>(&data[0]), sizeof(Data) * size);
	}
	template <typename Data>
	typename std::enable_if<std::is_pointer<Data>::value, void>::type serialize(
		Serializer &serializer,
		std::vector<Data> const &data) {
		std::size_t size = data.size();
		serializer.write(
			reinterpret_cast<char const *>(&size), sizeof(std::size_t));
		for (std::size_t i = 0; i < size; i++) {
			serializer << data[i];
		}
	}
	template <typename Data>
	typename std::enable_if<std::is_pointer<Data>::value, void>::type deserialize(
		Deserializer &deserializer,
		std::vector<Data> &data) {
		std::size_t size;
		deserializer.read(reinterpret_cast<char *>(&size), sizeof(std::size_t));
		data.resize(size);
		for (std::size_t i = 0; i < size; i++) {
			deserializer >> data[i];
		}
	}

	// std::string.
	template <
		typename CharT,
		typename Traits = std::char_traits<CharT>,
		typename Allocator = std::allocator<CharT>>
	void serialize(
		Serializer &serializer,
		std::basic_string<CharT, Traits, Allocator> const &data) {
		std::size_t size = data.size();
		serializer.write(
			reinterpret_cast<char const *>(&size), sizeof(std::size_t));
		serializer.write(
			reinterpret_cast<char const *>(&data[0]), sizeof(CharT) * size);
	}
	template <
		typename CharT,
		typename Traits = std::char_traits<CharT>,
		typename Allocator = std::allocator<CharT>>
	void deserialize(
		Deserializer &deserializer,
		std::basic_string<CharT, Traits, Allocator> &data) {
		std::size_t size;
		deserializer.read(reinterpret_cast<char *>(&size), sizeof(std::size_t));
		data.resize(size);
		deserializer.read(reinterpret_cast<char *>(&data[0]), sizeof(CharT) * size);
	}
}
