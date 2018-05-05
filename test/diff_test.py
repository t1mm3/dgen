#!/bin/env python2

from buildpaths import (getProjectBinPath, getTestPath, getParallelism)
from subprocess import Popen, PIPE
import sys
from multiprocessing import Pool


def diff(opts):
	(file, threads) = opts
	gen = getProjectBinPath() + "/dgen"
	config = getTestPath() + "/" + file

	data = Popen([gen, config, "-t", str(threads)], stdout=PIPE)
	diff = Popen(['diff', config + ".expect", "-"],stdin=data.stdout, stdout=PIPE)
	data.stdout.close()
	out, err = diff.communicate()

	out = out.decode().strip()
	return(out == "")

def run(testnames, parallel):
	# generate tests
	tests = []
	for testname in testnames:
		for threads in range(1, 16):
			t = 16 if parallel else 1
			tests = tests + [(testname, t)]

	# run tests
	pool = Pool(processes=getParallelism())
	result = pool.map(diff, tests)

	both = zip(tests,result)

	all_good = True
	for (test, succ) in both:
		if not succ:
			all_good = False
			print("Test '{}' failed".format(test))

	if not all_good:
		exit(1)

if __name__ == '__main__':
	testnames = ["json_conf1.txt", "json_conf2.txt", "json_conf3.txt", "json_conf1large.txt",
		"json_conf2large.txt", "json_conf3large.txt", "json_conf4large.txt"]

	print("Single threaded")
	run(testnames, False)

	print("Parallel")
	run(testnames, True)

	print("Success")
	exit(0)