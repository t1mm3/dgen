#!/bin/env python
from buildpaths import (getProjectBinPath, getTestPath)
import time
import os
import subprocess

def run(config):
	command = getProjectBinPath() + "/dgen"
	config = getTestPath() + config

	with open(os.devnull, "w") as f:
		start_time = time.time()
		subprocess.call([command, config], stdout=f)
		print("--- %s seconds ---" % (time.time() - start_time))


run("bench_conf1.txt")