#include <string>
#include <unordered_map>

namespace Rain::Networking::Http {
	class QueryParams : public std::unordered_map<std::string, std::string> {
		public:
		// Unwraps from a string (empty, or beginning with ?).
		QueryParams(std::string const &value) {
			for (std::size_t i{0}; i < value.length();) {
				std::size_t j{value.find('=', i + 1)}, k{value.find('&', j + 1)};
				this->insert(
					{value.substr(i + 1, j - i - 1), value.substr(j + 1, k - j - 1)});
				i = k;
			}
		}

		// Wraps into a string.
		operator std::string() const {
			std::string value{"?"};
			for (auto &it : *this) {
				value += it.first + "=" + it.second;
			}
			return value;
		}
	};
}
