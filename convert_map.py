reader = open('Paris_1_256.map', 'r')
file_write = open('Paris_1_256_created.cfg', 'w')

first_four = 0

for line in reader:
    first_four = first_four+1
    if(first_four<5):
        continue
    for l in line:
        if(l=='.'): file_write.write('0 ')
        elif(l=='@'): file_write.write('1 ')
    file_write.write("\n")
