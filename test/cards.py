#!/bin/env python2
from buildpaths import (getProjectBinPath, getTestPath)
from subprocess import Popen, PIPE

def test_card(num):
	p = Popen([
			getProjectBinPath() + "/dgen",
			getTestPath() + "/json_conf1.txt",
			"-t", "32",
			"-n", str(num)
		], stdout=PIPE)
	lines = sum(1 for _ in p.stdout)
	print ("{num} == {lines}".format(num=num, lines=lines))
	assert(num == lines)

print("test_card")

card = 1
for c in range(0, 8):
	test_card(card)
	card = card * 10



# test_card(0)