#pragma once

#include <vector>

namespace Rain::Algorithm {
	// Computes the the minimum prime factor and a list of primes for all integers
	// up to and including N. The minFactor array is 1-indexed; that is,
	// minFactor[1] refers to the minimum prime factor of 1, which we define as 1.
	template <typename Integer>
	std::pair<std::vector<Integer>, std::vector<Integer>> linearSieve(
		Integer const &N) {
		std::vector<Integer> minFactor(N + 1), primes;
		minFactor[0] = minFactor[1] = 1;
		for (Integer i{2}; i <= N; i++) {
			if (minFactor[i] == 0) {
				minFactor[i] = i;
				primes.push_back(i);
			}
			for (Integer j{0}; i * primes[j] <= N; j++) {
				minFactor[i * primes[j]] = primes[j];
				if (primes[j] == minFactor[i]) {
					break;
				}
			}
		}

		// C++17: guaranteed either NRVO or move.
		return {minFactor, primes};
	}
}
