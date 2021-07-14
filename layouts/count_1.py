import numpy as np
import random
from numpy import savetxt

data = np.loadtxt('tight_2_64.cfg', skiprows=10)
print(data)
print(np.shape(data), type(data))
print(data[3][3], type(data[3][3]))

l = np.shape(data)[0]
#expanded_layout = np.zeros((2*l,2*l), dtype=int)
count = 0
count_zero = 0
for i in range(l):
	for j in range(l):
		if(data[i][j] == 1):
			count = count+1
		elif(data[i][j] == 0):
			count_zero = count_zero+1

print(count, " ", count_zero, " ", count+count_zero)
