#!/usr/bin/env python3

######################################################################
#
#
#
#
######################################################################

from keras.layers import Dense
import keras
import numpy
import random
from mpi4py import MPI
from subprocess import Popen,PIPE
import math
import sys
import arrays
import time

tarski_exec = ""
try:
    tarski_exec = sys.argv[2]
except:
    tarski_exec = "/home/m201362/csunix/tarski/bin/tarski"

comm = MPI.COMM_WORLD
rank,size = (comm.Get_rank(), comm.Get_size())

if rank == 0:

    results = open(sys.argv[1]+"_res","w")
    param_file = open(sys.argv[1],"r")
    param_string = param_file.read()
    params = {}
    for p in param_string.strip().split("\n"):
        name,val = p.split(":")[0],p.split(":")[1]
        params[name] = val

    choice1 = params['choice1'].split(",")
    choice2 = params['choice2']
    example_file = params['example_file']
    num_examples = int(params['num_examples'])


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

    for i in range(len(choice1)):
        nums = {0:{},1:{}}
        example_indecies = random.sample(range(len(examples)),num_examples)
        for j in range(2):
            count = 0 #keep track of place in example_indecies
            # Farm out tasks to works until completed
            tests_completed = 0 # last completed test, tests are numbered 1 through N
            active = 0 # the number of processes currently
            fail = 0

            #prompt worker nodes to communicate if not in first round
            if i != 0 or j==1:
                for p in range(1,n_workers+1):
                    comm.send(("ex","wakeup",""),p)

            while tests_completed < num_examples or active > 0:
                ready,res,tarstr,ind = comm.recv()
                if res != "init":
                    active = active -1
                    tests_completed += 1
                    try:
                        nums[j][ind] = int(res)
                    except Exception as err:
                        # print(err)
                        nums[j][ind] = "penalty"
                if tests_completed < num_examples and count<len(example_indecies):
                    index = example_indecies[count]
                    count += 1
                    ex = examples[index]
                    graph_string = choice1[i] if j==0 else choice2
                    comm.send((ex,graph_string,index),dest=ready) # give worker more work
                    active = active + 1
        results.write(str(nums)+"\n")
        results.flush()


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
    comm.send((rank,"init","",""),dest=0)
    while True:
        x = comm.recv(source=0) # receive work tasking from manager
        ex,g,i = x[0],x[1],x[2]
        if g == "die":
            break;
        elif g == "wakeup":
            comm.send((rank,"init","",""),0)
            continue
        else:
            g = g.replace("sigmoid","tanh")
            p = Popen([tarski_exec, "-q", "-t", "60", "+N20000000"],stdout=PIPE,stdin=PIPE)
            tarski_string = ""
            if g == "BPC":
                tarski_string = ex+"\n"+"(def D (make-NuCADConjunction "+str(ex.split(" ")[1])+" '(chooser \"BPC\")))\n(display (msg D 'num-x-cells) \"\\n\")\n(quit)"
            else:
                tarski_string = ex+"\n"+"(def D (make-NuCADConjunction "+str(ex.split(" ")[1])+" '(chooser \"nnet\") '(nn-chooser \""+g+"\")))\n(display (msg D 'num-x-cells) \"\\n\")\n(quit)"
            res = p.communicate(tarski_string.encode())[0]
            p.terminate()
            comm.send((rank,res.decode(),tarski_string,i),dest=0) # send manager result
