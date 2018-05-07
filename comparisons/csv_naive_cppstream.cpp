#include "csv_naive.hpp"

int main(int argc, char* argv[])
{
	csv_naive<true, true>(atol(argv[1]));
}
