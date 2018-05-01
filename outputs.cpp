#include "outputs.hpp"


#include <iostream>
void CoutCons::operator()(std::string& data) {
	std::lock_guard<std::mutex> lg(lock);
	std::cout << data;
}