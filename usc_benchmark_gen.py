import numpy as np
import random

data = np.loadtxt('layouts/tight_64.cfg',skiprows=10)
print(data)
print(np.shape(data), type(data))
print(data[3][3], type(data[3][3]))

d = [2,6,10,14,18,22,26,30,34]

file1 = open("layouts/tight_64_2.scen","a")
count = 0
# for i in range(300):
#     x_i = random.randint(0,8)
#     y_i = random.randint(2,34)
#     x_g = random.randint(0,8)
#     y_g = random.randint(2,34) 
#     if(data[d[x_i]][y_i] == 0 and data[d[x_g]][y_g] == 0): 
#         count = count+1
#         L = [str(count), "    ", "usc.map    ", "37    ", "37    ", str(y_i), "    ", str(d[x_i]), "    ", str(y_g), "    ", str(d[x_g]), '\n']   #corresponds to extreme middle cell farthest from obstacles
#         file1.writelines(L)

# soft benchmarks
# for i in range(500):
#     x_i = random.randint(1,254)
#     y_i = random.randint(1,254)
#     x_g = random.randint(1,254)
#     y_g = random.randint(1,254) 
#     if(data[x_i][y_i] == 0 and data[x_g][y_g] == 0 and data[x_g+1][y_g] != 1 and data[x_g][y_g+1] != 1 and data[x_g][y_g-1] != 1 and data[x_g-1][y_g] != 1 and data[x_i+1][y_i] != 1 and data[x_i][y_i+1] != 1 and data[x_i][y_i-1] != 1 and data[x_i-1][y_i] != 1): 
#         count = count+1
#         L = [str(count), "    ", "usc.map    ", "256    ", "256    ", str(y_i), "    ", str(x_i), "    ", str(y_g), "    ", str(x_g), '\n']   #corresponds to extreme middle cell farthest from obstacles
#         file1.writelines(L)


#hard benchmarks
for i in range(500):
    x_i = random.randint(1,62)
    y_i = random.randint(1,62)
    x_g = random.randint(1,62)
    y_g = random.randint(1,62) 
    if(data[x_i][y_i] == 0 and data[x_g][y_g] == 0): # and data[x_g+1][y_g] != 1 and data[x_g][y_g+1] != 1 and data[x_g][y_g-1] != 1 and data[x_g-1][y_g] != 1 and data[x_i+1][y_i] != 1 and data[x_i][y_i+1] != 1 and data[x_i][y_i-1] != 1 and data[x_i-1][y_i] != 1): 
        count = count+1
        L = [str(count), "\t", "usc_64_64.map\t", "64\t", "64\t", str(y_i), "\t", str(x_i), "\t", str(y_g), "\t", str(x_g),"\t", "134.5653", '\n']   #corresponds to extreme middle cell farthest from obstacles
        file1.writelines(L)



print(count)  




# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1
# 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
