#!/bin/env python2
import time
import os
from subprocess import Popen, PIPE

def run_dev0(cmds):
	with open(os.devnull, "w") as f:
			start_time = time.time()
			subprocess.call(cmds, stdout=f)
			return time.time() - start_time

def runN_dev0(n, cmds):
	times = []
	for i in range(0, n):
		times = times + [run_dev0(cmds)]
	return times

def run_wc(cmds):
	p = Popen(cmds, stdout=PIPE)
	start_time = time.time()
	lines = Popen(['wc', "-l"],stdin=p.stdout, stdout=PIPE)
	p.stdout.close()
	out, err = lines.communicate()
	return time.time() - start_time

def runN_wc(n, cmds):
	times = []
	for i in range(0, n):
		times = times + [run_wc(cmds)]
	return times

def run(name, cmds):
	times = runN_wc(10, cmds)
	print("{name}\t{time}".format(name=name, time=min(times)))

def run_batch(num):
	curr_dir = "@CMAKE_CURRENT_BINARY_DIR@/"
	s = str(num)
	run(s+"\tdgen", ["@PROJECT_BINARY_DIR@/dgen", curr_dir + "/conf1.json", "-n", s])
	run(s+"\tnaive_printf", [curr_dir + "csv_naive_printf", s])
	run(s+"\tnaive_cppstream", [curr_dir + "csv_naive_cppstream", s])


run_batch(10000)
run_batch(100000)
run_batch(1000000)
run_batch(10000000)
