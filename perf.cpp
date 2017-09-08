#include "stdafx.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <iterator>
#include <chrono>
#define LOOPS 50
#define N 8000000

template <typename T, typename E>
void add(const std::vector<T>& a, const std::vector<T>& b, size_t elements){
	auto start = std::chrono::high_resolution_clock::now();
	E sum = 0;
	for (int index = 0; index < elements; ++index)
		sum += (sin(a[index]) * cos(b[index]));

	auto end = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	std::cout << "N="<<N <<" time(us) =" << time << " sum=" << sum << std::endl;

}

int main(){
	using namespace std;
	vector<int> a(N);
	vector<int> b(N);

	std::iota(a.begin(), a.end(), 400);
	std::iota(b.begin(), b.end(), 500);
	for (int index = 0; index < LOOPS; ++index)
		add<int, double>(a, b, N);
	return 0;
}
