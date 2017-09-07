#include <fstream>
#include <chrono>
#include <vector>
#include <cstdint>
#include <numeric>
#include <random>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <iterator>

template <typename T>
std::vector<T> GenerateData(std::size_t bytes){
	assert(bytes % sizeof(T) == 0);
	std::vector<T> data(bytes / sizeof(T));
	std::iota(data.begin(), data.end(), 40);
	return data;
}

template <typename T>
void write(const char* filename, std::size_t bytes){
	std::vector<T> data = GenerateData<T>(bytes);
	auto startTime = std::chrono::high_resolution_clock::now();
	auto myfile = std::fstream(filename, std::ios::out | std::ios::binary);
	myfile.write(reinterpret_cast<char*>(data.data()), bytes);
	myfile.close();
	auto endTime = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
	std::cout << "write speed=" << bytes * 1000 / (time * 1048576) << " MB/s" << std::endl;
}

template <typename T>
void read(const char* file, std::size_t bytes, size_t start, size_t end){
	auto startTime = std::chrono::high_resolution_clock::now();
	std::ifstream in{file};
	std::vector<T> buf(bytes / sizeof(T));// reserve space for N/8 doubles
	in.read(reinterpret_cast<char*>(buf.data()), buf.size() * sizeof(T));
	auto endTime = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
	std::cout << "read speed=" << bytes * 1000 / (time * 1048576) << " MB/s" << std::endl;
	std::cout << "total elements read=" << buf.size() << std::endl;
	//std::copy(buf.begin() + start, buf.begin() + end, std::ostream_iterator<T>(std::cout, "\n"));

}

int main(){
	const std::size_t bytes = 100000000;
	write<uint32_t>("numbers.dat",bytes);
	read<uint32_t>("numbers.dat", bytes, 1, 10);
	return 0;
}
