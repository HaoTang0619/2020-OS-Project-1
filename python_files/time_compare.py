# RUN AS SUDO #

import os
import sys
import subprocess as sp

path = sys.argv[1] # Theo_unit.txt
output = sys.argv[2] # dmesg_files
compare = sys.argv[3] # compared result
total = int(sp.getoutput('ls ' + output + ' | wc -l')) // 2 - 1

if not os.path.exists(compare):
    os.makedirs(compare)

# Theoretical unit
ref = os.path.join(output, 'TIME_MEASUREMENT_dmesg.txt')
Unit = []
with open(ref) as rf:
    for line in rf.readlines():
        start, end = line.split()[2:4]
        Unit.append(float(end) - float(start))
Unit = sum(Unit) / len(Unit) / 500

cnt = 1
max_diff = 0
min_diff = 1e20
tot_diff = 0
tot_cnt = 0

for f in sorted(os.listdir(output)):
    if f[:16] == 'TIME_MEASUREMENT' or f[-10:] == 'stdout.txt':
        continue
    f = f.split('_dmesg')[0]

    print('Now running:','(%s/%d)'%(str(cnt).zfill(2), total), f)

    with open(os.path.join(path)) as pf:
        with open(os.path.join(output, f + '_dmesg.txt')) as outf:
            with open(os.path.join(compare, f + '_compare.txt'), 'w') as cpf:
                cpf.write('|'.join(['Name', 'Theo. start', 'Theo. end', 'Theo. running', 'Practical start', 'Practical end', 'Practical running']) + '\n')
                cpf.write('|'.join(['-', '-', '-', '-', '-', '-', '-']) + '\n')
                
                plines = pf.readlines()
                Theo_unit = {}
                for pl in plines:
                    pl = pl.split()
                    Theo_unit[pl[0]] = pl[1:]

                olines = outf.readlines()
                # Set start_time as 0
                BASE = 1e20 
                BASE_T = 1e20
                for ol in olines:
                    BASE = min(BASE, float(ol.split()[2]))
                    BASE_T = min(BASE_T, round(Unit * float(Theo_unit[ol.split()[1]][1]), 5))
                
                for ol in olines:
                    ol = ol.split()
                    pid = ol[1]
                    prac_start = str(round(float(ol[2]) - BASE, 5))
                    prac_end = str(round(float(ol[3]) - BASE, 5))
                    PRAC = str(round(float(prac_end) - float(prac_start), 5))
                    prac_start += '0' * (8 - len(prac_start))
                    prac_end += '0' * (8 - len(prac_end))
                    PRAC += '0' * (8 - len(PRAC))

                    theo_start = str(round(Unit * float(Theo_unit[pid][1]) - BASE_T, 5))
                    theo_end = str(round(Unit * float(Theo_unit[pid][2]) - BASE_T, 5))
                    THEO = str(round(float(theo_end) - float(theo_start), 5))
                    theo_start += '0' * (8 - len(theo_start))
                    theo_end += '0' * (8 - len(theo_end))
                    THEO += '0' * (8 - len(THEO))

                    diff = abs(float(PRAC) - float(THEO)) / float(THEO) * 100
                    tot_diff += diff
                    tot_cnt += 1
                    max_diff = max(max_diff, diff)
                    min_diff = min(min_diff, diff) 
                    
                    cpf.write('|'.join([Theo_unit[pid][0], theo_start, theo_end, THEO, prac_start, prac_end, PRAC]) + '\n')
    
    cnt += 1

print("Unit:", round(Unit, 5))
print("Max_diff:", str(round(max_diff, 5)) + '%')
print("Min_diff:", str(round(min_diff, 5)) + '%')
print("Avg_diff:", str(round(tot_diff / tot_cnt, 5)) + '%')

