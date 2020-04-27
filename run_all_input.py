# RUN AS SUDO #

import os
import sys
import subprocess as sp

path = sys.argv[1]
total = int(sp.getoutput('ls ' + path + ' | wc -l'))

if not os.path.exists("output"):
    os.makedirs("output")

cnt = 1
files = sorted(os.listdir(path))
for f in files:
    print('Now running:','(%s/%d)'%(str(cnt).zfill(2), total), f)
    
    os.system("dmesg -C")
    filename = os.path.join(path, f)
    policy, num = f.split("_")[:2]
    output_file = "%s_%s_stdout.txt"%(policy, num[:-4])
    dmesg_file = "%s_%s_dmesg.txt"%(policy, num[:-4])
    os.system("./main < %s > %s"%(filename, os.path.join("output", output_file)))

    dmesg_out = sp.getoutput("dmesg | grep Project1").split('\n')
    with open (os.path.join('output', dmesg_file), 'w') as df:
        for line in dmesg_out:
            time = line.split('[Project1] ')[1]
            df.write('[Project1] ' + time + '\n')
    cnt += 1
