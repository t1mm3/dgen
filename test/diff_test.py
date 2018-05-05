#!/bin/env python2

from buildpaths import (getProjectBinPath, getTestPath)
from subprocess import Popen, PIPE
import sys
from subprocess import check_output

def diff(file):
	gen = getProjectBinPath() + "/dgen"
	config = getTestPath() + "/" + file
	
	data = Popen([gen, config], stdout=PIPE)
	diff = Popen(['diff', config + ".expect", "-"],stdin=data.stdout, stdout=PIPE)
	data.stdout.close()
	out, err = diff.communicate()

	print(out)
	assert(out.strip == "")

diff("json_conf1.txt")
diff("json_conf2.txt")
diff("json_conf3.txt")

print("Success")
exit(0)