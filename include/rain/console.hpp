#pragma once

#include "algorithm/geometry.hpp"
#include "literal.hpp"
#include "platform.hpp"
#include "windows.hpp"

#include <iostream>
#include <mutex>

#ifdef RAIN_PLATFORM_WINDOWS
#include <conio.h>
#else
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace Rain {
	class Console {
		private:
#ifdef RAIN_PLATFORM_WINDOWS
		// Ensures that virtual console mode is enabled, so that escape sequences
		// work as expected.
		class Initializer {
			private:
			HANDLE const hStdOut;
			DWORD const origMode;

			static DWORD getConsoleMode(HANDLE hStdOut) {
				DWORD mode;
				Windows::validateSystemCall(GetConsoleMode(hStdOut, &mode));
				return mode;
			}

			public:
			Initializer()
					: hStdOut{GetStdHandle(STD_OUTPUT_HANDLE)},
						origMode{Initializer::getConsoleMode(this->hStdOut)} {
				DWORD mode{this->origMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING};
				Windows::validateSystemCall(SetConsoleMode(this->hStdOut, mode));
			}
			~Initializer() { SetConsoleMode(this->hStdOut, this->origMode); }
		};
		static inline Initializer _Initializer;
#endif

		static int getchInner() {
#ifdef RAIN_PLATFORM_WINDOWS
			return _getch();
#else
			static struct termios old, current;
			tcgetattr(0, &old);
			current = old;
			current.c_lflag &= ~ICANON;
			current.c_lflag &= ~ECHO;
			tcsetattr(0, TCSANOW, &current);
			int c{getchar()};
			tcsetattr(0, TCSANOW, &old);
			return c;
#endif
		}

		public:
		enum class Code {
			CTRL_C = 0x03,

			BACKSPACE = 0x08,

			ENTER = 0x0a,

			DEL = 0x7f,

			ARROW_LEFT = 0x80,
			ARROW_UP,
			ARROW_RIGHT,
			ARROW_DOWN,

			F1 = 0x84,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,

			ERR = 0xff,
		};

		// Returns [0x80, 0x84) for the arrow keys, [0x84, 0x8f) for the f-keys,
		// 0xff for an error where multiple keys may have been consumed.
		static Code getch() {
			int i{Console::getchInner()};
			switch (i) {
#ifdef RAIN_PLATFORM_WINDOWS
				// ENTER.
				case 13:
					return Code::ENTER;
				case 224: {
					int j{Console::getchInner()};
					switch (j) {
						// ARROW.
						case 75:
							return Code::ARROW_LEFT;
						case 72:
							return Code::ARROW_UP;
						case 77:
							return Code::ARROW_RIGHT;
						case 80:
							return Code::ARROW_DOWN;
							// DEL.
						case 83:
							return Code::DEL;
						// F11-F12.
						case 133:
							return Code::F11;
						case 134:
							return Code::F12;
						default:
							return Code::ERR;
					}
				}
				// BACKSPACE.
				// F1-F10.
				case 0: {
					int j{Console::getchInner()};
					switch (j) {
						case 59:
							return Code::F1;
						case 60:
							return Code::F2;
						case 61:
							return Code::F3;
						case 62:
							return Code::F4;
						case 63:
							return Code::F5;
						case 64:
							return Code::F6;
						case 65:
							return Code::F7;
						case 66:
							return Code::F8;
						case 67:
							return Code::F9;
						case 68:
							return Code::F10;
						default:
							return Code::ERR;
					}
				}
#else
					// ENTER.
				case 27: {
					int j{Console::getchInner()};
					switch (j) {
						case 79: {
							int k{Console::getchInner()};
							switch (k) {
								// F1-F4
								case 80:
									return Code::F1;
								case 81:
									return Code::F2;
								case 82:
									return Code::F3;
								case 83:
									return Code::F4;
							}
						}
						case 91: {
							int k{Console::getchInner()};
							switch (k) {
								// F5-F8.
								case 49: {
									int l{Console::getchInner()};
									switch (l) {
										case 53:
											Console::getchInner();
											return Code::F5;
										case 55:
											Console::getchInner();
											return Code::F6;
										case 56:
											Console::getchInner();
											return Code::F7;
										case 57:
											Console::getchInner();
											return Code::F8;
										default:
											return Code::ERR;
									}
								}
								// F9-F12.
								case 50: {
									int l{Console::getchInner()};
									switch (l) {
										case 48:
											Console::getchInner();
											return Code::F9;
										case 49:
											Console::getchInner();
											return Code::F10;
										case 51:
											Console::getchInner();
											return Code::F11;
										case 52:
											Console::getchInner();
											return Code::F12;
										default:
											return Code::ERR;
									}
								}
								// ARROW.
								case 68:
									return Code::ARROW_LEFT;
								case 65:
									return Code::ARROW_UP;
								case 67:
									return Code::ARROW_RIGHT;
								case 66:
									return Code::ARROW_DOWN;
									// DEL.
								case 51:
									Console::getchInner();
									return Code::DEL;
								default:
									return Code::ERR;
							}
						}
						default:
							return Code::ERR;
					}
				}
				// BACKSPACE.
				case 127:
					return Code::BACKSPACE;
#endif
				default:
					return static_cast<Code>(i);
			}
		}

		enum class Color {
			BLACK = 0,
			RED,
			GREEN,
			YELLOW,
			BLUE,
			MAGENTA,
			CYAN,
			WHITE,
			DEFAULT = 9,
		};

		private:
#ifdef RAIN_PLATFORM_WINDOWS
		static WORD colorToWindows(Color const &color) {
			switch (color) {
				case Color::BLACK:
					return 0;
				case Color::RED:
					return FOREGROUND_RED;
				case Color::GREEN:
					return FOREGROUND_GREEN;
				case Color::YELLOW:
					return FOREGROUND_RED | FOREGROUND_GREEN;
				case Color::BLUE:
					return FOREGROUND_BLUE;
				case Color::MAGENTA:
					return FOREGROUND_RED | FOREGROUND_BLUE;
				case Color::CYAN:
					return FOREGROUND_GREEN | FOREGROUND_BLUE;
				case Color::WHITE:
					return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
				default:
					return 9;
			}
		}
#endif

		static void coutInner() {}
		template <typename Value>
		static void coutInner(Value &&value, auto &&...values) {
			std::cout << std::forward<Value>(value);
			Console::coutInner(std::forward<decltype(values)>(values)...);
		}

		static inline std::mutex coutMtx;

		public:
		class ConsoleParameters {
			public:
			Color fgColor{Color::DEFAULT};
			bool fgIntense{false};
			Color bgColor{Color::DEFAULT};
			bool bgIntense{false};

			// Position is 0-indexed.
			long row{-1}, col{-1};
			bool restore{true};
		};

		// Background colors are inconsistent with newlines, position instead.
		static void cout(ConsoleParameters const &params, auto &&...values) {
			std::lock_guard<std::mutex> lckGuard(Console::coutMtx);
			std::cout << "\x1b[0;"
								<< 40 + static_cast<int>(params.bgColor) +
					(params.bgIntense ? 60 : 0)
								<< ";"
								<< 30 + static_cast<int>(params.fgColor) +
					(params.fgIntense ? 60 : 0)
								<< "m";

			if (params.row >= 0 && params.col >= 0) {
				std::cout << "\x1b[" << params.row + 1 << ";" << params.col + 1 << "f";
			}

			Console::coutInner(std::forward<decltype(values)>(values)...);

			if (params.restore) {
				std::cout << "\x1b[0m";
			}
		}
		// Fills console with spaces, and positions back to 1, 1.
		static void clear() {
			std::lock_guard<std::mutex> lckGuard(Console::coutMtx);
			std::cout << "\x1b[2J\x1b[3J\x1b[1;1H";
		}
		static void hideCursor() {
			std::lock_guard<std::mutex> lckGuard(Console::coutMtx);
			std::cout << "\x1b[?25l";
		}
		static void showCursor() {
			std::lock_guard<std::mutex> lckGuard(Console::coutMtx);
			std::cout << "\x1b[?25h";
		}
		static Algorithm::Geometry::PointL getSize() {
#ifdef RAIN_PLATFORM_WINDOWS
			static HANDLE hStdOut{
				Windows::validateSystemCall(GetStdHandle(STD_OUTPUT_HANDLE))};
			static CONSOLE_SCREEN_BUFFER_INFO csbi;
			Windows::validateSystemCall(GetConsoleScreenBufferInfo(hStdOut, &csbi));
			return {
				static_cast<long>(csbi.srWindow.Right - csbi.srWindow.Left + 1),
				static_cast<long>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1)};
#else
			struct winsize w;
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
			return {static_cast<long>(w.ws_col), static_cast<long>(w.ws_row)};
#endif
		}
		void flush() {
			std::lock_guard<std::mutex> lckGuard(Console::coutMtx);
			std::cout.flush();
		}

		static void log(auto &&...values) {
			if (!Rain::Platform::isDebug()) {
				return;
			}
			std::lock_guard<std::mutex> lckGuard(Console::coutMtx);
			Console::coutInner(std::forward<decltype(values)>(values)...);
			std::cout << std::endl;
		}
	};
}
