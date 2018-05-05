#!/bin/env python2

from buildpaths import (getProjectBinPath, getTestPath)
from subprocess import Popen, PIPE
import sys

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

	print ("'{num}' == '{lines}'".format(num=num, lines=out))
	assert(num == out)

print("test_card")

test_card(0)

card = 1
for c in range(0, 9):
	test_card(card)
	card = card * 10

print("Success")
exit(0)