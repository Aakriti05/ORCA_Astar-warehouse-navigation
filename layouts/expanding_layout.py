import numpy as np
import random
from numpy import savetxt

data = np.loadtxt('tight_2_128.cfg', skiprows=10)
print(data)
print(np.shape(data), type(data))
print(data[3][3], type(data[3][3]))

l = np.shape(data)[0]
expanded_layout = np.zeros((2*l,2*l), dtype=int)

for i in range(l):
    for j in range(l):
        expanded_layout[2*i][2*j] = data[i][j]
        expanded_layout[2*i+1][2*j] = data[i][j]
        expanded_layout[2*i][2*j+1] = data[i][j]
        expanded_layout[2*i+1][2*j+1] = data[i][j]

print(np.shape(expanded_layout))
print(expanded_layout, type(expanded_layout), type(data[3][3]))
savetxt('tight_2_256.cfg', expanded_layout.astype(int), fmt='%i')

#c-14,5,16 royal ragon
