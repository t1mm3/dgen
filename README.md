# dgen
dgen is a flexible data generator for testing and benchmarking purpose.
It aims at producing in the desired format quickly and, hence, can be used for generating large amounts of data.

## Features


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

## Column specification
Each row consists of attributes from a set of columns. First for each attribute one specifies its type. Currently dgen supports: ```integer``` and ```string```. 

### Integer
For integer attributes we generate values either as a densely increasing counter (```sequential```) or randomly chosen from a distribution (e.g. ```random```). Using ```min``` and ```max``` one can specify the domain minimum and maximum.

The following fields are required:

Field | Description
------------ | -------------
gen | Source (e.g. sequential, uniform/random, poisson)
min | Domain minimum
max | Domain maximum
mean | (Poisson only) µ distribution parameter

### String
Strings are generated through dictionary lookups whereas the index (```index```) is an integer.
Dictionaries are specific either through a file name (```file``` each line is a word) or by specifying a list of possible all words (```in```).  

For example:

```javascript
{ "string" : {"index" : { "integer" : { "gen" : "random", "min" : 10, "max" : 100 } }, "in" : ["w1", "w2", "w3"]}}
```

Randomly chooses a word with indices between 10 and 100 (modulo dictionary size) from the dictionary consisting of ```w1```, ```w2``` and ```w3```.