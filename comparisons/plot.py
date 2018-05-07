#!/bin/env python
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

def plot_runs():
    df = pd.read_csv('@CMAKE_CURRENT_BINARY_DIR@/runs.txt', sep='\t', names=['NUM', 'NAME', 'MIN', 'MEAN'], skiprows=1)

    fig = plt.figure()

    ax1 = fig.add_subplot(111)

    ax1.set_title("CSV generation (2 integer columns)")
    ax1.set_xlabel('#Rows')
    ax1.set_ylabel('Time in s')
    ax1.set_xscale("log")
    # ax1.set_yscale("log")

    # For each type
    dist = []
    for index, row in df.iterrows():
        if row['NAME'] not in dist:
            dist.append(row['NAME'])

    for t in dist:
        dd = df[df['NAME'] == t]
        ax1.plot(dd['NUM'], dd['MIN'], label=t, linestyle='--', marker='o')

    leg = ax1.legend()
    fig.savefig("@CMAKE_CURRENT_BINARY_DIR@/runs.pdf")
    plt.close(fig)


def plot_scalablity():
    df = pd.read_csv('@CMAKE_CURRENT_BINARY_DIR@/scaling.txt', sep='\t', names=['THREADS', 'MIN', 'MEAN'], skiprows=1)

    fig = plt.figure()

    ax1 = fig.add_subplot(111)

    ax1.set_title("Scalability of CSV generation (2 integer columns)")
    ax1.set_xlabel('#Threads')
    ax1.set_ylabel('Time in s')

    ax1.plot(df['THREADS'], df['MIN'], linestyle='--', marker='o')

    leg = ax1.legend()
    fig.savefig("@CMAKE_CURRENT_BINARY_DIR@/scaling.pdf")
    plt.close(fig)

def main():
    plot_runs()
    plot_scalablity()


if __name__ == '__main__':
    main()