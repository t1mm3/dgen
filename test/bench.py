#!/bin/env python2
from buildpaths import (getProjectBinPath, getTestPath)
import time
import os
import subprocess
import tempfile

def gen_csv(col_domains, num_tuples, file):
	cols = ""
	first = True
	for c in col_domains:
		col = ""
		cgen_str = ""

		(cmin, cmax, cgen, ctype) = c
		if cgen == ColGenType.RANDOM:
			cgen_str = "random"
		elif cgen == ColGenType.SEQUENTIAL:
			cgen_str = "sequential"
		else:
			assert(False)

		if ctype == ColType.INT:
			col = '{{ "integer" : {{ "gen" : "{gen}", "min" : {min}, "max" : {max} }} }}'.format(gen=cgen_str, min=cmin, max=cmax)
		elif ctype == ColType.STR:
			col = '{{ "string" : {{"index" : {{ "integer" : {{ "gen" : "{gen}", "min" : {min}, "max" : {max} }} }}, "file" : "words.txt" }} }}'.format(gen=cgen_str, min=cmin, max=cmax)
		else:
			assert(False)

		# concat
		if first:
			first = False
			cols = col
		else:
			cols = cols + ", \n" + col

	script = """
{{
    "version" : "0.1",
    "tuples" : {num},
    "columns" : [
{cols}
    ],
    "format" : {{
        "csv" : {{ "comma" : "|", "newline" : "|\\n"}}
    }},
    "output" : "stdout"
}}

""".format(num=num_tuples, cols=cols)

	read, write = os.pipe()
	os.write(write, script)
	os.close(write)
	with open(file,"wb") as out:
		subprocess.Popen("dgen",stdout=out, stdin=read)


def run(script, descr):
	command = getProjectBinPath() + "/dgen"

	with tempfile.NamedTemporaryFile() as temp:
		temp.write(script)
		temp.flush()

		with open(os.devnull, "w") as f:
			start_time = time.time()
			subprocess.call(["perf", "record", "--", command, temp.name], stdout=f)
			print("%s\t%s" % (descr, time.time() - start_time))

run("""
{{
    "version" : "0.1",
    "tuples" : 100000000,
    "threads" : 1,
    "columns" : [
        {{ "integer" : {{ "gen" : "random", "min" : 10, "max" : 10000 }} }}
    ],
    "format" : {{
        "csv" : {{ "comma" : "|", "newline" : "\\n"}}
    }},
    "output" : "stdout"
}}
	""".format(), "1 int")

int1024 = ""
first = True
for i in range(0, 1024):
	sep = "" if first else ",\n"

	int1024 = int1024 + """{sep}{{ "integer" : {{ "gen" : "random", "min" : 10, "max" : 10000 }} }}""".format(sep=sep)
	first = False

run("""
{{
    "version" : "0.1",
    "tuples" : 100000,
    "threads" : 1,
    "columns" : [
        {ints}
    ],
    "format" : {{
        "csv" : {{ "comma" : "|", "newline" : "\\n"}}
    }},
    "output" : "stdout"
}}
	""".format(ints=int1024), "1024 int")
