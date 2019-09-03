#   Generates a string representaion of a DAG in a format expected by the Constructor
#   to Graph in node.cpp.
#   a: 1D array of nodes in the DAG per latyer. For example, a model with 8 input
#       nodes, 4 middle nodes, and 1 output node would have an a array of [8,4,1]
#   weight: 3D array of weights, produced by arrays.makeArrays()
#   bias: 2D aray of biases, produced bt arrays.makeArrays()
#   functions: 1D array of each layer's (not the input layer) activation functions.
#       Produced by arrays.makeArrays
def generateGraph(a, weight, bias, functions):
    graphString = ""
    layer={}
    i=1
    while i <= len(a):
        layer[i-1] = list(range(sum(a[0:i])-a[i-1], sum(a[0:i])))
        i = i+1

    graphString=str(sum(a))+" "
    currentLayer = 0;
    while(currentLayer < len(layer)):
        for nodenum in layer[currentLayer]: #for each node in current layer
            if currentLayer==0:
                graphString += "( i  func " + str(nodenum) + " 0.0 " +  " [ ] [ ] ) "
            else:
                graphString+= "( m "+functions[currentLayer-1]+" " + str(nodenum) + " " + str(bias[currentLayer-1][nodenum-max(layer[currentLayer-1]) - 1]) + " " + str(layer[currentLayer-1]).replace(",", " ").replace("[", "[ ").replace("]", " ] ")
                graphString += "[ "
                for i in range(a[currentLayer-1]): #for each input of the node
                    graphString += str(weight[currentLayer-1][i][nodenum-layer[currentLayer][0]]) + " "
                graphString+="] ) "
        currentLayer+=1
    return graphString
