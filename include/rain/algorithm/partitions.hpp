#pragma once

#include <vector>

namespace Rain::Algorithm {
	// Computes up to the N-th partition number in O(N\sqrt{N}) using the
	// pentagonal numbers/generating function. This can be done in O(N\lg N) but
	// may involve polynomial exponentiation or log, which are significantly more
	// complicated.
	template <typename Integer>
	inline std::vector<Integer> partitionNumbers(Integer const &N) {
		std::vector<Integer> partitions(N + 1, 0);
		partitions[0] = 1;
		for (Integer i{1}; i <= N; i++) {
			for (Integer j{1}, k{1}, sign{1};;
					 j++, k = (j * (3 * j - 1)) >> 1, sign = -sign) {
				if (k > i) {
					break;
				}
				partitions[i] += sign * partitions[i - k];
				k += j;
				if (k > i) {
					break;
				}
				partitions[i] += sign * partitions[i - k];
			}
		}

		// C++17: guaranteed either NRVO or move.
		return partitions;
	}
}
