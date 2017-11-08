import numpy as np
import matplotlib.pyplot as plt
from itertools import combinations


class Fiber():
    def __init__(self):
        self.data = []
        self.maxofList = 0
        self.minofList = 0
        self.numbofElem = 0
        self.average = 0 
        self.variance = 0
        self.geoMean = 0
        
    def add(self, elem):
        self.data.append(elem)
        self.numbofElem += 1
    
    def stats(self):
        self.maxofList = max(self.data)
        self.minofList = min(self.data)
        if self.numbofElem not in [0,1]:
            self.average = self.numbofElem /(self.maxofList -self.minofList)
            self.variance = np.var(self.data)



def createFrames(frames,k,l,index):
    try:
        frames[(k,l)][(index[k]//100,index[l]//100)] += 1
    except KeyError:
        frames[(k,l)][(index[k]//100,index[l]//100)] = 1

def createFibers(axis, fibIndex, index,fiberObj):
    try:
        fiber = axis[fibIndex][tuple(index[i] for i in fibIndex)]
    except KeyError:
        axis[fibIndex][tuple(index[i] for i in fibIndex)] = Fiber()
    axis[fibIndex][tuple(index[i] for i in fibIndex)].add(index[fiberObj])


dim = 3
numbofFrames = dim * (dim)/2
totalElem = 0
genofFiber = combinations(range(dim),dim-1)
axis = dict()

#initilize frames dictionary
frames = dict()
for k, l  in combinations(range(dim),2):
    frames[(k,l)] = dict()
for fibIndex in combinations(range(dim),dim-1):
    axis[fibIndex] = dict()
#open file and read and keep frames inside dict

f = open('rand_tensor.tns', 'r')
for line in f:
    count += 1
    index = [int(x) for x in line.strip('\n').split(' ')[0:-1]]
    
    totalElem += 1
    #for every frame dictionary create a dictionary
    for k, l  in combinations(range(dim),2):
        #inside that dictionary every tuple(x,y) has a FramePoint
        createFrames(frames,k,l,index)    
    for fibIndex in combinations(range(dim),dim-1):
        fiberObj = [i for i in range(dim) if i not in fibIndex][0]
        createFibers(axis, fibIndex, index,fiberObj)



for fibIndex in combinations(range(dim),dim-1):
    numElem = []
    varofFibers = []
    for fiber in axis[fibIndex].values():
        fiberAxis = [i for i in range(dim) if i not in fibIndex][0]
         #discard fibers with 0 and 1 elements
        fiber.stats()
        numElem.append(fiber.numbofElem)
        if fiber.numbofElem not in [0,1]:
            varofFibers.append(int(fiber.variance))
    #plt.figure(figsize=(30,20))
    n = plt.hist(varofFibers,bins=1000)
    plt.savefig('./' + str(fiberAxis) + ' - Variance Histogram' + ".png")
    #plt.figure(figsize=(30,20))
    n = plt.hist(numElem,bins=max(numElem)+1)
    plt.savefig('./' + str(fiberAxis) + ' - #elements Fiber' + ".png")




for k, l  in combinations(range(dim),2):
    listofElem = []
    for t,v in frames[(k,l)].items():
        listofElem.append([t[0],t[1],v])
    a = np.array(listofElem)
    x = a[:,0]
    y = a[:,1]
    z = a[:,2]
    plt.figure()
    plt.title('{}{} Axes'.format(k,l))
    plt.scatter(x, y, c=z, cmap='jet',marker='.')
    plt.colorbar()
    plt.savefig('./' + str(k) + str(l) + ".png")

plt.show()


