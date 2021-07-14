import numpy as np
import random

data = np.loadtxt('env_sorter.cfg',skiprows=10)
print(data)
print(np.shape(data), type(data))
print(data[3][3], type(data[3][3]))

# print(data[73][69], data[88][38])

file1 = open("addverb2.scen","a")
count = 0
for i in range(1000):
    x_i = random.randint(1,87)
    y_i = random.randint(1,121)
    x_g = random.randint(1,87)
    y_g = random.randint(1,121)
    if(data[x_i][y_i] == 0 and data[x_g][y_g] == 0): 
        count = count+1
        L = [str(count), "    ", "Addverb.map    ", "88    ", "122    ", str(y_i), "    ", str(x_i), "    ", str(y_g), "    ", str(x_g), '\n']
        file1.writelines(L)

print(count)  
