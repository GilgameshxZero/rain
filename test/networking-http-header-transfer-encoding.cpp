// Tests Networking::Http::Header::TransferEncoding.
#include <rain.hpp>

int main() {
	using namespace Rain::Networking::Http::Header;

	// Test various operators on TransferEncoding.
	{
		TransferEncoding t;
		std::stringstream stream;
		stream << t;
		assert(stream.str() == "identity");

		switch (t) {
			case TransferEncoding::IDENTITY:
				break;
			case TransferEncoding::CHUNKED:
			default:
				throw std::runtime_error("Switch failed.\n");
		}

		assert(t == TransferEncoding::IDENTITY);
		if (t != TransferEncoding::IDENTITY) {
			throw std::runtime_error("Inequality operator failed.\n");
		}

		TransferEncoding t2;
		if (t != t2) {
			throw std::runtime_error("Inequality operator failed.\n");
		}
		assert(t == t2);

		std::string s{t.operator std::string()};
		assert(s == "identity");

		try {
			TransferEncoding t3("invalid");
			throw std::runtime_error("Invalid transfer encodings should throw.");
		} catch (std::exception const &exception) {
			if (
				strcmp(exception.what(), "Invalid transfer encodings should throw.") ==
				0) {
				throw;
			}
		}
	}
}
