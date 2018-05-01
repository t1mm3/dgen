#include "outputs.hpp"

#include <iostream>

void CoutOutput::operator()(std::string& data)
{
	std::lock_guard<std::mutex> lg(lock);
	std::cout << data;
}


CheckOutput::CheckOutput(std::function<void(std::string&)> check)
 : check(check)
{
}


void
CheckOutput::operator()(std::string& data)
{
	check(data);
}
