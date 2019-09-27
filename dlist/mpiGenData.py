#!/usr/bin/env python3

######################################################################
#
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
# import arrays
import time
import os


comm = MPI.COMM_WORLD
rank,size = (comm.Get_rank(), comm.Get_size())
tarski_path = sys.argv[2]

# os.environ["CUDA_DEVICE_ORDER"] = "PCI_BUS_ID"   # see issue #152
# if rank in range(4):
#  os.environ["CUDA_VISIBLE_DEVICES"] = str(rank)
# else:
#  os.environ["CUDA_VISIBLE_DEVICES"] = "-1"

if rank == 0:

    # read in params
    param_file = open(sys.argv[1],"r")
    param_string = param_file.read()
    params = {}
    for p in param_string.strip().split("\n"):
        name,val = p.split(":")[0],p.split(":")[1]
        params[name] = val

    examples_per_round = int(params["examples_per_round"])


    n_workers = size - 1

    example_file = open(params["example_file"],"r")
    examples = example_file.read().strip().split("\n")
    total_examples = len(examples)


    #### MANAGER ####
    # random.seed(a=2020) # note: same seed every time! Don't really do this!

    # Give every worker its own random seed
    for p in range(1,n_workers+1):
        ready = comm.recv(source=p)
        comm.send(random.randint(0,10000),dest=ready)
    print("starting")
    # try:
    x = []
    y = []
    graph_string = ""
    # Farm out tasks to works until completed
    tests_successful = 0 # number of successfully completed tests
    active = 0 # the number of processes currently
    fail = 0

    while tests_successful < total_examples or active > 0:
        ready,res,tarstr = comm.recv()
        if res != "init":
            active = active - 1
            try:
                for line in res.strip().split("\n"):
                    x.append([float(i) for i in line.split(":")[0].split(",")])
                    y.append(0.0 if float(line.split(":")[1]) < 0.0 else 1.0)
                tests_successful = tests_successful + 1
                feat = open("features.txt","w")
                results = open("results.txt","w")
                feat.write(str(x))
                results.write(str(y))
                feat.close()
                results.close()
            except Exception as err:
                fail += 1
                #tests_successful -= 1
        if tests_successful < examples_per_round:
            ex = examples.pop(random.randint(0,len(examples)))
            #ex = examples.pop(0)
            comm.send((ex,graph_string),dest=ready) # give worker more work
            active = active + 1
    # except Exception as err:
    #     print(err)

    #kill all workers
    for p in range(1,n_workers+1):
        comm.send(("","die"),p)
    exit()

else:
    #### WORKER ####
    comm.send(rank,dest=0)
    seed = comm.recv(source=0)
    if seed < 0:
        sys.exit(0)
    random.seed(a=seed)
    comm.send((rank,"init",""),dest=0)
    while True:
        x = comm.recv(source=0) # receive work tasking from manager
        ex,g = x[0],x[1]
        if g == "die":
            break;
        elif g == "wakeup":
            comm.send((rank,"init",""),0)
            continue
        else:
            g = g.replace("sigmoid","tanh")
            p = Popen([tarski_path,"-q", "-t", "60", "+N20000000"],stdout=PIPE,stdin=PIPE,env={'LD_LIBRARY_PATH':os.environ['LD_LIBRARY_PATH']})
            # tarski_string = ex+"\n"+"(def D (make-NuCADConjunction "+str(ex.split(" ")[1])+" '(chooser \"nnet\") '(nn-chooser \""+g+"\")))\n(display (msg D 'trial "+str(random.randint(0,10000000))+") \"\\n\")\n(quit)"
            tarski_string = ex+"\n"+"(def D (make-NuCADConjunction "+str(ex.split(" ")[1])+" '(chooser \"BPC\")))\n(display (msg D 'trial) \"\\n\")\n(quit)"
            res = p.communicate(tarski_string.encode())[0]
            p.terminate()
            comm.send((rank,res.decode(),tarski_string),dest=0) # send manager result
