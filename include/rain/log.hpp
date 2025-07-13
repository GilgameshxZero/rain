#include <iostream>
#include <mutex>

namespace Rain {
	// Class for multithreaded logging which is turned off at RELEASE.
	class Log {
		private:
		static inline std::mutex mtx;

		public:
		template <typename... Values>
		static void verbose(Values... values) {
			if (!Rain::Platform::isDebug()) {
				return;
			}
			std::lock_guard<std::mutex> lckGuard(Log::mtx);
			Log::writeLine(values...);
		}

		private:
		static void writeLine() { std::cout << std::endl; }

		template <typename Value, typename... Values>
		static void writeLine(Value &&value, Values &&...values) {
			std::cout << std::forward<Value>(value);
			Log::writeLine(std::forward<Values>(values)...);
		}
	};
}
