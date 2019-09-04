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
import arrays
import time
import os


print(os.environ['LD_LIBRARY_PATH'])
comm = MPI.COMM_WORLD
rank,size = (comm.Get_rank(), comm.Get_size())

os.environ["CUDA_DEVICE_ORDER"] = "PCI_BUS_ID"   # see issue #152
if rank in range(4):
 os.environ["CUDA_VISIBLE_DEVICES"] = str(rank)
else:
 os.environ["CUDA_VISIBLE_DEVICES"] = "-1"

if rank == 0:

    errors = open(sys.argv[1]+"_errors.txt","w")
    results = open(sys.argv[1]+"_results.txt","w")
    sanity_check = open(sys.argv[1]+"_success.txt","w")

    # read in params
    param_file = open(sys.argv[1],"r")
    param_string = param_file.read()
    params = {}
    for p in param_string.strip().split("\n"):
        name,val = p.split(":")[0],p.split(":")[1]
        params[name] = val

    learning_rate = float(params["learning_rate"])
    learning_decay = float(params["learning_decay"])
    epochs = int(params["epochs"])
    examples_per_round = int(params["examples_per_round"])


    #Build model
    model = keras.Sequential()
    dims = [int(i) for i in params["model_shape"].split(",")]

    if(len(dims)<2):
        print("Bad model_shape")
        exit()
    elif(len(dims)==2): #perceptron
        model.add(Dense(dims[1],input_dim=dims[0],activation='sigmoid',kernel_initializer='ones',bias_initializer='ones'))
    else:
        #input and first layer
        model.add(Dense(dims[1],input_dim=dims[0],activation='relu',kernel_initializer='ones',bias_initializer='ones'))
        #middle layers
        for i in range(2,len(dims)-1):
            model.add(Dense(dims[i],activation='relu',kernel_initializer='ones',bias_initializer='ones'))
        #output layer
        model.add(Dense(1,activation="sigmoid",kernel_initializer='ones',bias_initializer='ones'))

    opt = keras.optimizers.SGD(lr=learning_rate,clipvalue=0.5)
    model.compile(opt,loss='binary_crossentropy',metrics=['accuracy'])

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
    try:
        for i in range(math.floor(total_examples/examples_per_round)):
            results.write("STARTING ROUND "+str(i)+":\n")
            results.flush()
            x = []
            y = []
            graph_string = arrays.getModelGraphString(model).strip()

            # Farm out tasks to works until completed
            tests_successful = 0 # number of successfully completed tests
            active = 0 # the number of processes currently
            fail = 0
            #prompt worker nodes to communicate if not in first round
            if i != 0:
                for p in range(1,n_workers+1):
                    comm.send(("ex","wakeup"),p)

            start_time = time.time()
            iter_count = 0
            while tests_successful < examples_per_round or active > 0:
                sanity_check.write(">>>>>>> Starting loop iteration " + str(iter_count) +
                                   " tests_successful = " + str(tests_successful) + " active = " + str(active) +
                                   "\n")
                sanity_check.flush()
                iter_count = iter_count + 1
                ready,res,tarstr = comm.recv()
                if res == "init":
                    sanity_check.write("Init from worker "+str(ready)+"\n")
                    sanity_check.flush()
                else:
                    active = active - 1
                    sanity_check.write("Response from worker "+str(ready)+"! Tests successful="+str(tests_successful)+"\nActive="+str(active)+"\n")
                    sanity_check.flush()
                    try:
                        for line in res.strip().split("\n"):
                            x.append([float(i) for i in line.split(":")[0].split(",")])
                            y.append(0.0 if float(line.split(":")[1]) < 0.0 else 1.0)
                        tests_successful = tests_successful + 1
                    except Exception as err:
                        fail += 1
                        #tests_successful -= 1
                        errors.write("Error msg: " + str(err)+"\n")
                        errors.write(str(res.strip().split("\n"))+"\n"+tarstr)
                        errors.flush()
                if tests_successful < examples_per_round:
                    ex = examples.pop(random.randint(0,len(examples)))
                    #ex = examples.pop(0)
                    comm.send((ex,graph_string),dest=ready) # give worker more work
                    active = active + 1
                    sanity_check.write("Giving worker "+str(ready)+" more work.\n")
                    sanity_check.flush()

            generateDataTime = time.time() - start_time
            results.write("Time to generate data in round "+str(i)+": "+str(generateDataTime)+"\n")
            results.flush()
            model.fit(numpy.array(x),numpy.array(y),epochs=epochs,verbose=0)
            fit_time = time.time()- generateDataTime - start_time
            results.write("Time to fit model in round "+str(i)+": "+str(fit_time)+"\n")
            results.write("Round "+str(i)+"weights:\n"+str(model.get_weights())+"\n")
            results.write("Round "+str(i)+"graph string:\n"+str(arrays.getModelGraphString(model))+"\n")
            results.flush()
            learning_rate *= learning_decay
            opt = keras.optimizers.SGD(lr=learning_rate,clipvalue=0.5)
            model.compile(opt,loss='binary_crossentropy',metrics=['accuracy'])
            results.write("Round "+str(i)+" done\n\n")
            results.flush()

    except Exception as err:
        print(err)

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
            p = Popen(["../../bin/tarski","-q", "-t", "60", "+N20000000"],stdout=PIPE,stdin=PIPE,env={'LD_LIBRARY_PATH':os.environ['LD_LIBRARY_PATH']})
            tarski_string = ex+"\n"+"(def D (make-NuCADConjunction "+str(ex.split(" ")[1])+" '(chooser \"nnet\") '(nn-chooser \""+g+"\")))\n(display (msg D 'trial "+str(random.randint(0,10000000))+") \"\\n\")\n(quit)"
            res = p.communicate(tarski_string.encode())[0]
            p.terminate()
            comm.send((rank,res.decode(),tarski_string),dest=0) # send manager result
