import random
import statistics
from matplotlib import pyplot as plt
import fileinput

def analyze(res_file,sizes,num_subsets,metric="wp",pen=2):
    results = eval(open(res_file).read())
    penalty0 = max([i for i in results[0].values() if isinstance(i,int)])*pen
    penalty1 = max([i for i in results[1].values() if isinstance(i,int)])*pen
    res = {}
    for N in sizes:
        w0 = []
        w1 = []
        for k in range(num_subsets):
            win0 = 0
            win1 = 0
            for i in random.sample(results[0].keys(),N):
                if metric == "wp":
                    if(results[0][i]=="penalty" and results[1][i]!="penalty"):
                        win1 += 1
                    elif(results[0][i]!="penalty" and results[1][i]=="penalty"):
                        win0 +=1
                    elif(results[0][i] > results[1][i]):
                        win0 += 1
                    elif(results[0][i] < results[1][i]):
                        win1 += 1
                elif metric == "nodes":
                    win0 += penalty0 if results[0][i] == "penalty" else results[0][i]
                    win1 += penalty1 if results[1][i] == "penalty" else results[1][i]
            w0.append(float(win0)/N)
            w1.append(float(win1)/N)
        avg0_N = sum(w0)/num_subsets
        avg1_N = sum(w1)/num_subsets
        sd0_N = statistics.stdev(w0)
        sd1_N = statistics.stdev(w1)
        res[N] = {"avg0_N":avg0_N,"avg1_N":avg1_N,"sd0_N":sd0_N,"sd1_N":sd1_N}
    return res

def graphData(data):
    x = list(data.keys())
    u0 = [i["avg0_N"] for i in data.values()]
    u1 = [i["avg1_N"] for i in data.values()]
    s0 = [i["sd0_N"] for i in data.values()]
    s1 = [i["sd1_N"] for i in data.values()]
    plt.plot(x,u0,label="avg0_N")
    plt.plot(x,u1,label="avg1_N")
    plt.plot(x,s0,label="sd0_N")
    plt.plot(x,s1,label="sd1_N")
    plt.ylabel("Value")
    plt.xlabel('N')
    plt.legend()
    plt.show()
