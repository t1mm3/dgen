#!/bin/env python2
import time
import os
import sys
import multiprocessing
from subprocess import Popen, PIPE, call

def run_dev0(cmds):
	with open(os.devnull, "w") as f:
			start_time = time.time()
			call(cmds, stdout=f)
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

def run(file, wc, name, cmds):
	if wc:
		times = runN_wc(10, cmds)
	else:
		times = runN_dev0(10, cmds)

	file.write("{0}\t{1:10.2f} \t{2:10.2f}\n".format(name, min(times), sum(times)/len(times)))

def run_runs_batch(f, num, wc):
	curr_dir = "@CMAKE_CURRENT_BINARY_DIR@/"
	s = str(num)
	run(f, wc, s+"\tdgen           ", ["@PROJECT_BINARY_DIR@/src/dgen", curr_dir + "/conf1.json", "-n", s])
	run(f, wc, s+"\tdgen_4threads  ", ["@PROJECT_BINARY_DIR@/src/dgen", curr_dir + "/conf1.json", "-t", "4" ,"-n", s])
	run(f, wc, s+"\tnaive_printf   ", [curr_dir + "csv_naive_printf", s])
	run(f, wc, s+"\tnaive_cppstream", [curr_dir + "csv_naive_cppstream", s])
	run(f, wc, s+"\tlut_printf     ", [curr_dir + "csv_hardcode_lut", s])

def run_all_runs(f, wc):
	f.write("#NUM\tNAME\tMIN\tMEAN\n")
	for order in range(5, 9):
		run_runs_batch(f, 10**order, wc)	

def run_all_scale(f):
	f.write("#THREADS\tMIN\tMEAN\n")
	curr_dir = "@CMAKE_CURRENT_BINARY_DIR@/"
	num = 100000000
	for t in range(1, multiprocessing.cpu_count()+1):
		run(f, True, str(t), ["@PROJECT_BINARY_DIR@/src/dgen", curr_dir + "/conf1.json", "-n", str(num), "-t", str(t)])


with open("@CMAKE_CURRENT_BINARY_DIR@/runs_wc.txt", "w+") as f:
	run_all_runs(f, True)

#with open("@CMAKE_CURRENT_BINARY_DIR@/runs_dev0.txt", "w+") as f:
#	run_all_runs(f, False)


#with open("@CMAKE_CURRENT_BINARY_DIR@/scaling.txt", "w+") as f:
#	run_all_scale(f)