# dgen
dgen is a flexible data generator for testing and benchmarking purpose.
It aims at producing in the desired format quickly and, hence, can be used for generating large amounts of data.

## Performance
This benchmark compares optimized single-threaded naive implementations with dgen. The naive implementations generate integers in the given domain and directly write a row to stdout.

![Small benchmark](comparisons/runs_wc_stones02.png)

Observe that single-threaded dgen is a around 2x faster than the naive implementations whereas the parallel version of dgen using 4 threads is another 4x faster while still providing a deterministic output.

## Further information
* [Data specification](SPECIFICATION.md)