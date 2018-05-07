#!/bin/env python2
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

def main():
    df = pd.read_csv('runs.txt', sep='\t', names=['NUM', 'NAME', 'MIN', 'MEAN'], skiprows=1)
    print(df)

    fig = plt.figure()

    ax1 = fig.add_subplot(111)

    # ax1.set_title("Times")    
    ax1.set_xlabel('#Rows')
    ax1.set_ylabel('Time')
    ax1.set_xscale("log")

    # For each type
    dist = []
    for index, row in df.iterrows():
        if row['NAME'] not in dist:
            dist.append(row['NAME'])

    for t in dist:
        dd = df[df['NAME'] == t]
        ax1.plot(dd['NUM'], dd['MIN'], label=t, linestyle='--', marker='o')

    leg = ax1.legend()

    plt.show()

if __name__ == '__main__':
    main()