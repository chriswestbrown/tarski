from keras.layers import Dense
import keras
import numpy
import graphtest

#   Creates the arrays necessary for graphtest.generateGraph() based on a keras
#   model passed in as input.

def makeArrays(model):
    weights = []
    bias = []
    functions = []
    #iterate through each layer in the model. Does not include the inputs as a
    #"real layer"
    for l in model.layers:
        functions.append(l.get_config()['activation'])
        temp = l.get_weights();
        weights.append(temp[0].tolist())
        try:
            bias.append(temp[1].tolist())
        except IndexError:
            #the output node will not have a bias, so we give it a bias of zero
            #so that the other programs function the same without having to check
            #for the last node
            bias.append([0.0])
    return (weights,bias,functions)

def getLayerSizes(model):
    return list(map(lambda x: x.input_shape[1],model.layers)) + [model.layers[-1].output_shape[1]]

def getModelGraphString(model):
    x = makeArrays(model)
    return graphtest.generateGraph(getLayerSizes(model),x[0],x[1],x[2])
