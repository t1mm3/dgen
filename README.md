# dgen
dgen is a flexible data generator for testing and benchmarking purpose.
It aims at producing in the desired format quickly and, hence, can be used for generating large amounts of data.

# Configuration

```javascript
{
    "version" : "0.1",
    "tuples" : 10000,
    "threads" : 2,
    "columns" : [
        { "integer" : { "gen" : "random", "min" : 10, "max" : 100 } },
        { "integer" : { "gen" : "random", "min" : -10, "max" : 10 } }
    ],
    "format" : {
        "csv" : { "comma" : "|", "newline" : "\n"}
    },
    "output" : "stdout" 
}
```