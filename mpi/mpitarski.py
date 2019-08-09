#!/usr/bin/env python3

######################################################################
# Example pattern for using mpi with a manager/worker approach.
#
# run for example as: mpirun -n 4 ./mpie02.py
#
# We will run N tests.  Each test produces a value (the 'width') and
# we would like to print out the average 'width'.
# process 0 manages the process, keeping the worker threads busy.
# 1. each worder thread is given a seed value
# 2. worker's repeatedly signal that they are ready for work (also
#    returning a value if they've computed one, and the mangager
#    sends them a new test to run, summing up the results as it goes.
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


comm = MPI.COMM_WORLD
rank,size = (comm.Get_rank(), comm.Get_size())


if rank == 0:
    learning_rate = 0.05
    learning_decay = .7
    examples_per_round = 10
    epochs = 20
    file_name = "../tests/hpc2019/sets/type2-4vars.txt"

    file = open(file_name,"r")
    examples = file.read().strip().split("\n")
    total_examples = len(examples)

    model = keras.Sequential()
    model.add(Dense(1,input_dim=11,activation='tanh',kernel_initializer='ones',bias_initializer='ones'))
    # model.add(Dense(1,activation='tanh',kernel_initializer='ones',use_bias=False))
    opt = keras.optimizers.SGD(lr=learning_rate,clipvalue=0.5)
    model.compile(opt,loss='binary_crossentropy',metrics=['accuracy'])
    n_workers = size - 1
    # number of tests to be run
    # l.total_boards = 1000

    #### MANAGER ####
    # random.seed(a=2020) # note: same seed every time! Don't really do this!

    # Give every worker its own random seed
    for p in range(1,n_workers+1):
        ready = comm.recv(source=p)
        comm.send(random.randint(0,10000),dest=ready)

    for i in range(math.floor(total_examples/examples_per_round)):
        x = []
        y = []
        graph_string = arrays.getModelGraphString(model).strip()

        # Farm out tasks to works until completed
        test_num = 0 # last completed test, tests are numbered 1 through N
        active = 0 # the number of processes currently
        fail = 0
        #prompt worker nodes to communicate if not in first round
        if i != 1:
            for p in range(1,n_workers+1):
                comm.send(("ex","wakeup"),p)

        while test_num < examples_per_round or active > 0:
            ready,res = comm.recv()
            if res != "init":
                active = active -1
                try:
                    for line in res.strip().split("\n"):
                        x.append([float(i) for i in line.split(":")[0].split(",")])
                        y.append(float(line.split(":")[1]))
                        print("success")
                except:
                    fail += 1
                    sys.stderr.write("Fail num: "+str(fail)+"\nWorker"+str(ready)+" could not parse this result:\n\t"+res+"\n")
                    sys.stderr.flush()
            if test_num < examples_per_round:
                test_num = test_num + 1
                ex = examples.pop(random.randint(0,len(examples)))
                comm.send((ex,graph_string),dest=ready) # give worker more work
                active = active + 1
            else:
                active = active-1
        print("Finished round "+str(i)+". Fitting now")
        model.fit(numpy.array(x),numpy.array(y),epochs=epochs,verbose=0)
        learning_rate *= learning_decay
        opt = keras.optimizers.SGD(lr=learning_rate,clipvalue=0.5)
        model.compile(opt,loss='binary_crossentropy',metrics=['accuracy'])
        print(model.get_weights())

    #kill all workers
    for p in range(1,n_workers+1):
        comm.send("die",p)
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
            p = Popen(["../bin/tarski","-q", "-t", "60", "+N20000000"],stdout=PIPE,stdin=PIPE)
            tarski_string = ex+"\n"+"(def D (make-NuCADConjunction "+str(ex.split(" ")[1])+" '(chooser \"nnet\") '(nn-chooser \""+g+"\")))\n(display (msg D 'trial) \"\\n\")\n(quit)"
            res = p.communicate(tarski_string.encode())[0]
            p.terminate()
            comm.send((rank,res.decode()),dest=0) # send manager result
