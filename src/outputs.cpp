#include "outputs.hpp"

#include <iostream>

void CoutOutput::operator()(StrBuffer&& data)
{
	std::cout.write(&data.data[0], data.used);
}


CheckOutput::CheckOutput(std::function<void(StrBuffer&&)> check)
 : check(check)
{
}


void
CheckOutput::operator()(StrBuffer&& data)
{
	check(std::move(data));
}
