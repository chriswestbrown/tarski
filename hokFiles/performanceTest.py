#!/usr/bin/env python3

######################################################################
#
#
#
#
######################################################################

import numpy
import random
from mpi4py import MPI
from subprocess import Popen,PIPE
import math
import sys
import time
import os

tarski_exec = ""
try:
    tarski_exec = sys.argv[2]
except:
    tarski_exec = "/home/m201362/csunix/tarski/bin/tarski"

comm = MPI.COMM_WORLD
rank,size = (comm.Get_rank(), comm.Get_size())

if rank == 0:
    param_file = open(sys.argv[1],"r")
    param_string = param_file.read()
    params = {}
    for p in param_string.strip().split("\n"):
        name,val = p.split(":")[0],p.split(":")[1]
        params[name] = val

    choice1 = params['choice1']
    example_file = params['example_file']
    num_examples = int(params['num_examples'])
    fname = params['fname']
    seed = int(params['seed'])


    n_workers = size - 1
    example_file = open(example_file,"r")
    examples = example_file.read().strip().split("\n")
    total_examples = len(examples)

    #### MANAGER ####

    # Give every worker its own random seed
    for p in range(1,n_workers+1):
        ready = comm.recv(source=p)
        comm.send(random.randint(0,10000),dest=ready)

    evals = []

    random.seed(seed)
    example_indecies = random.sample(range(len(examples)),num_examples)

    nums = []
    # Farm out tasks to works until completed
    tests_completed = 0 # last completed test, tests are numbered 1 through N
    active = 0 # the number of processes currently
    fail = 0

    while tests_completed < num_examples or active > 0:
        # print(comm.recv())
        ready,res = comm.recv()
        if res != "init":
            print(str(ready)+" finished")
            active = active -1
            tests_completed += 1
            print(tests_completed)
            print(res)
            try:
                nums.append(int(res))
            except Exception as err:
                nums.append("timeout")
        if tests_completed < num_examples and len(example_indecies)>0:
            index = example_indecies.pop()
            ex = examples[index]
            graph_string = choice1
            comm.send((ex,graph_string,index),dest=ready) # give worker more work
            active = active + 1
            print("Giving "+str(ready)+" more work")
    ##test here!!
    results = open(fname,"w")
    results.write(str(nums)+"\n")
    results.flush()
    results.close()


    #kill all workers
    for p in range(1,n_workers+1):
        comm.send(("ex","die",""),p)
    results.close()
    exit()

else:
    #### WORKER ####
    comm.send(rank,dest=0)
    seed = comm.recv(source=0)
    if seed < 0:
        sys.exit(0)
    random.seed(a=seed)
    comm.send((rank,"init"),dest=0)
    while True:
        x = comm.recv(source=0) # receive work tasking from manager
        ex,g = x[0],x[1]
        if g == "die":
            break;
        elif g == "wakeup":
            comm.send((rank,"init"),0)
            continue
        else:
            g = g.replace("sigmoid","tanh")
            p = Popen([tarski_exec, "-q", "-t", "60", "+N20000000"],stdout=PIPE,stdin=PIPE,env={'LD_LIBRARY_PATH':os.environ['LD_LIBRARY_PATH']})
            tarski_string = ""
            if g == "BPC":
                tarski_string = ex+"\n"+"(def D (make-NuCADConjunction "+str(ex.split(" ")[1])+" '(chooser \"BPC\")))\n(display (msg D 'num-x-cells) \"\\n\")\n(quit)"
            else:
                tarski_string = ex+"\n"+"(def D (make-NuCADConjunction "+str(ex.split(" ")[1])+" '(chooser \"dlist\") '(dl-chooser \""+g+"\")))\n(display (msg D 'num-x-cells) \"\\n\")\n(quit)"
            res = p.communicate(tarski_string.encode())[0]
            p.terminate()
            comm.send((rank,res.decode()),dest=0) # send manager result
