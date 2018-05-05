#!/bin/env python2

from buildpaths import (getProjectBinPath, getTestPath)
from subprocess import Popen, PIPE
import sys
from multiprocessing import Pool

def test_card(num):
	p = Popen([
			getProjectBinPath() + "/dgen",
			getTestPath() + "/json_conf1.txt",
			"-t", "32",
			"-n", str(num)
		], stdout=PIPE)
	lines = Popen(['wc', "-l"],stdin=p.stdout, stdout=PIPE)
	p.stdout.close()
	out, err = lines.communicate()
	out = int(out.strip())

	return num == out

if __name__ == '__main__':
	tests = [0]

	card = 1
	for c in range(0, 9):
		tests = tests + [card]
		card = card * 10

	pool = Pool(processes=4)
	result = pool.map(test_card, tests)

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


print("Success")
exit(0)