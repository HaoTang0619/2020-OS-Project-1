# 2020-OS-Project-1

**B06902020 唐浩**

- Spec: https://hackmd.io/@Ue96nvjESj2XsDXw532-qA/ryYqceUrU

## Environment
- OS: Ubuntu 18.04
- GCC version: 7.5.0
- Python version: 3.6.9 (For running all input files, not necessary.)

## Folders
- compare: Theoretical time units vs. Practical time units
- demo: Demo video
- kernel_files: All the files I modified when compiling linux 5.4 kernel. (with 2 new system calls)
- (Reference: https://fenghe.us/compile-5-4-2-linux-kernel-with-a-new-syscall-for-ubuntu-18-04-in-vmware/)
- OS_PJ1_Test: Input files
- output: Output files
- python_files: For running all input files and comparing time units.

## Usage
```powershell
# Compile
make

# Execute ("sudo" is for outputing to dmesg.)
## Single case: (This will generate: (1) outputs to stdout and dmesg, (2) theoretical time units to a file "./Theo_unit.txt".)
sudo ./make < <INPUT FILE>

## All input files:
sudo python3 run_all_input.py <INPUT FILE DIRECTORY> <OUTPUT FILE DIRECTORY> <MAIN>
ex: sudo python3 run_all_input.py ../OS_PJ1_Test ../output ../main

## Compare theoretical & practical time:
python3 time_compare.py <THEO_UNIT FILE> <OUTPUT FILE DIRECTORY> <COMPARED FILE DIRECTORY>
ex: sudo python3 time_compare.py Theo_unit.txt ../output ../compare
```

## Results
```powershell
Time per unit: 0.00146s
Difference between theoretical & practical time:
- Max: 7.56016%
- Min: 0.0047%
- Avg: 1.3636%
```

## Remarks
- Start time: 紀錄子行程被第一次丟上cpu執行時的時間。
