// Includes all /data headers.
#pragma once

#include "data/serializer.hpp"

template <typename Value, std::size_t SIZE>
inline std::ostream &operator<<(
	std::ostream &stream,
	std::array<Value, SIZE> const &values) {
	if (values.empty()) {
		return stream << "[]";
	}
	stream << '[' << std::setw(4) << values[0];
	for (std::size_t i{1}; i < SIZE; i++) {
		stream << ' ' << std::setw(4) << values[i];
	}
	return stream << ']';
}
