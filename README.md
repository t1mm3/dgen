# dgen
dgen is a flexible data generator for testing and benchmarking purpose.
It aims at producing in the desired format quickly and, hence, can be used for generating large amounts of data.

# Configuration
To generate data dgen expects a configuration file in JSON format which describes the columns to be generated, output size, output format etc.

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