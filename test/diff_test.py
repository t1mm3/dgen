#!/bin/env python2

from buildpaths import (getProjectBinPath, getTestPath)
from subprocess import Popen, PIPE
import sys
from multiprocessing import Pool

def diff(file):
	gen = getProjectBinPath() + "/dgen"
	config = getTestPath() + "/" + file

	data = Popen([gen, config], stdout=PIPE)
	diff = Popen(['diff', config + ".expect", "-"],stdin=data.stdout, stdout=PIPE)
	data.stdout.close()
	out, err = diff.communicate()

	out = out.decode().strip()
	return(out == "")

if __name__ == '__main__':
	tests = ["json_conf1.txt", "json_conf2.txt", "json_conf3.txt", "json_conf1large.txt",
		"json_conf2large.txt", "json_conf3large.txt", "json_conf4large.txt"]

	pool = Pool(processes=4)
	result = pool.map(diff, tests)

	both = zip(tests,result)

	all_good = True
	for (test, succ) in both:
		if not succ:
			all_good = False
			print("Test '{}' failed".format(test))

	if all_good:
		print("Success")
		exit(0)
	else:
		exit(1)