#include "csv_naive.hpp"

int main(int argc, char* argv[])
{
	csv_naive<false, true>(atol(argv[1]));	
}
