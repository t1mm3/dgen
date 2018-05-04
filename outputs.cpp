#include "outputs.hpp"

#include <iostream>

void CoutOutput::operator()(std::string&& data)
{
	std::cout << std::move(data);
}


CheckOutput::CheckOutput(std::function<void(std::string&&)> check)
 : check(check)
{
}


void
CheckOutput::operator()(std::string&& data)
{
	check(std::move(data));
}
