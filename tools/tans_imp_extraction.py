import os
import multiprocessing
import matplotlib.pyplot as plt
from scipy.interpolate import interp1d
from statistics import mean

power = 108
period = [1e-6, 10e-6, 100e-6, 1e-3, 10e-3, 100e-3, 1]
duty = [0.01, 0.02, 0.05, 0.1, 0.3, 0.5]

period.reverse()
duty.reverse()

cwd = os.path.dirname(__file__)
exe = cwd + '/../build.release/bin/sic.exe'
workdir = cwd + '/../test/data/simulation/thermal/trans'

def run_job(cmd) :
    os.system(cmd)

def batch_run() :
    jobs = []
    for i in range(len(period)) :
        for j in range(len(duty)) :
            jobdir = workdir + f'/{i}_{j}'
            if not os.path.exists(jobdir) :
                os.makedirs(jobdir)
            job = f'{exe} {jobdir} {period[i]} {duty[j]}'
            # print(job)
            jobs.append(job)
        
    pool = multiprocessing.Pool()
    pool.map(run_job, jobs)

def load_data(filename) :
    results = [[] for i in range(13)] 
    with open(filename) as f :
        for line in f :
            if line := line.strip() :
                args = line.split(',')
                for i in range(13) :
                    results[i].append(float(args[i]))
    return results

def calc_z(times, interp, period) :
    t1 = 0
    t2 = t1 + period
    z = []
    s = 0
    while t2 < times[-1] :
        maxT = 0
        minT = 1e9
        start = s
        for i in range(start, len(times)) :
            s = i
            t = times[i]
            if t < t1 :
                continue
            if t > t2 :
                break
            minT = min(minT, interp(t))
            maxT = max(maxT, interp(t))
        z.append((maxT - minT)/power)
        t1 = t2
        t2 += period

    return mean(z)
        
def analysis(i, j) :

    jobdir = workdir + f'/{i}_{j}'
    results = load_data(jobdir + '/trans.txt')
    interp = [interp1d(results[0], results[k], kind = 'linear') for k in range(1, 13)]

    p = period[i]
    z = []
    for k in range(12) :
        z.append(calc_z(results[0], interp[k], p))
    return z

if __name__ == '__main__' :
    # batch_run()
    # for i in range(len(period)) :
    #     for j in range(len(duty)) :
    f = open(workdir + '/z.txt', 'w')
    for i in range(len(period)) :
        for j in range(len(duty)) :
                print(f'{i}_{j}')
                for z in analysis(i, j) :
                    f.write(f'{z},')
                f.write('\n')

    f.close()