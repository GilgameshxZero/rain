#pragma once

#include <array>
#include <iomanip>
#include <iostream>

namespace Rain::Math {
	// TODO: Implement efficient views/slicing.
	template <typename U, std::size_t C_ROW, std::size_t C_COL = C_ROW>
	class Matrix : public std::array<std::array<U, C_COL>, C_ROW> {
		private:
		using TypeThis = Matrix<U, C_ROW, C_COL>;
		using TypeSuper = std::array<std::array<U, C_COL>, C_ROW>;

		public:
		template <typename std::enable_if<C_COL == C_ROW>::type * = nullptr>
		static TypeThis I() {
			TypeThis x;
			for (std::size_t i{0}; i < x.size(); i++) {
				x[i][i] = 1;
			}
			return x;
		}

		template <typename... Args>
		Matrix(Args &&...args) : TypeSuper({std::forward<Args>(args)...}) {}

		TypeThis &operator=(TypeThis const &other) {
			TypeSuper::operator=(other);
			return *this;
		}

		TypeThis operator+(TypeThis const &other) const {
			TypeThis x;
			for (std::size_t i{0}; i < x.size(); i++) {
				for (std::size_t j{0}; j < x[i].size(); j++) {
					x[i][j] = this->at(i)[j] + other[i][j];
				}
			}
			return x;
		}
		TypeThis &operator+=(TypeThis const &other) {
			return *this = *this + other;
		}
		TypeThis operator-(TypeThis const &other) const {
			TypeThis x;
			for (std::size_t i{0}; i < x.size(); i++) {
				for (std::size_t j{0}; j < x[i].size(); j++) {
					x[i][j] = this->at(i)[j] - other[i][j];
				}
			}
			return x;
		}
		TypeThis &operator-=(TypeThis const &other) {
			return *this = *this - other;
		}
		TypeThis operator*(U const &other) const {
			TypeThis x;
			for (std::size_t i{0}; i < x.size(); i++) {
				for (std::size_t j{0}; j < x[i].size(); j++) {
					x[i][j] = this->at(i)[j] * other;
				}
			}
			return x;
		}
		TypeThis &operator*=(U const &other) { return *this = *this * other; }
		// TODO: Obviously, Strassen's.
		template <std::size_t C_R_COL>
		auto operator*(Matrix<U, C_COL, C_R_COL> const &other) const {
			Matrix<U, C_ROW, C_R_COL> x;
			for (std::size_t i{0}; i < x.size(); i++) {
				for (std::size_t j{0}; j < x[i].size(); j++) {
					for (std::size_t k{0}; k < this->at(i).size(); k++) {
						x[i][j] += this->at(i)[k] * other[k][j];
					}
				}
			}
			return x;
		}
		template <std::size_t C_R_COL>
		auto &operator*=(Matrix<U, C_COL, C_R_COL> const &other) {
			return *this = *this * other;
		}

		void fill(U const &other) {
			for (auto &i : *this) {
				i.fill(other);
			}
		}
		TypeThis T() {
			TypeThis x{*this};
			for (std::size_t i{0}; i < x.size(); i++) {
				for (std::size_t j{i + 1}; j < x[i].size(); j++) {
					std::swap(x[i][j], x[j][i]);
				}
			}
			return x;
		}
	};
}

template <typename T, std::size_t C>
inline std::ostream &operator<<(
	std::ostream &stream,
	std::array<T, C> const &right) {
	using namespace std;
	if (right.empty()) {
		return stream << "()";
	}
	stream << '(' << setw(4) << right.front();
	for (std::size_t i{1}; i < right.size(); i++) {
		stream << ' ' << setw(4) << right[i];
	}
	return stream << ')';
}

template <typename T, std::size_t C_ROW, std::size_t C_COL = C_ROW>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Math::Matrix<T, C_ROW, C_COL> const &right) {
	using namespace std;
	if (right.empty()) {
		return stream << "[]";
	}
	stream << '[' << right.front();
	for (std::size_t i{1}; i < right.size(); i++) {
		stream << "\n " << right[i];
	}
	return stream << ']';
}
