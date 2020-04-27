# 2020-OS-Project-1

** B06902020 唐浩 **

- Spec: https://hackmd.io/@Ue96nvjESj2XsDXw532-qA/ryYqceUrU

## Environment
- OS: Ubuntu 16.04
- GCC version: 5.4.0
- Python version: 3.5.2 (For running all input files, not necessary.)

## Usage
```powershell
# Compile
make

# Execute ("sudo" is for outputing to dmesg.)
## Single case:
## This will generate outputs to stdout and dmesg.
sudo ./make < <INPUT FILE>

## All input files:
python3 run_all_input.py <INPUT FILE DIRECTORY>
```

## Remarks
- Start time: 紀錄子行程被第一次丟上cpu執行時的時間。
- No need to compile the kernel.
