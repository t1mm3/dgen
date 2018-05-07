#ifndef H_COMPARE_CSV_NAIVE
#define H_COMPARE_CSV_NAIVE

#include <iostream>
#include <random>
#include <cstdlib>

template<bool cpp_stream, bool sequential>
void csv_naive(size_t card)
{
	std::ios_base::sync_with_stdio(false);
	int64_t seed = 1;

	std::mt19937 rng(seed);
	std::uniform_int_distribution<int16_t> uni(10,10000);

	std::mt19937 rng2(seed);
	std::uniform_int_distribution<int16_t> uni2(-10,10000);

	const int64_t dom1 = 10000-10;
	const int64_t dom2 = 10000+10;

	for (size_t i=0; i<card; i++) {
		int16_t col1, col2;
		if (sequential) {
			col1 = (i % dom1) + 10;
			col2 = (i % dom2) - 10;
		} else {
			col1 = uni(rng);
			col2 = uni2(rng2);
		}
		if (cpp_stream) {
			std::cout << col1 << '|' << col2 << "\n";	
		} else {
			printf("%d|%d\n", col1, col2);
		}
	}
}

#endif