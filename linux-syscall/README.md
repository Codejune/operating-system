# Calculation System Call

4가지 연산 기능을 (덧셈(+), 뺄셈(-), 곱셈(*), 나머지(%)) 제공하는 신규 시스템 콜을 각각 추가하고, 추가된 시스템 콜을 호출하는 테스트 프로그램 구현

## 0. Quick start

``` bash
git clone https://github.com/codejune/c-linux-syscall.git
cd c-linux-syscall

# 시스템 콜 추가
cp sys_* {linux_kernel_dir}/kernel/
cp kernel_Makefile {linux_kernel_dir}/kernel/Makefile
cp syscalls.h {linux_kernel_dir}/include/linux/
cp syscall_64.tbl {linux_kernel_dir}/arch/x86/entry/syscalls/
# Do kernel build and reboot your system

# 테스트 프로그램 빌드 및 실행
make
./mycalculator
```

## 1. Environment

### Software

- Ubuntu 20.04.3 LTS (x86_64)
- Linux Kernel 5.11.22

### Hardware

- VM Instance
- 4 Core 8 Thread
- 8 GB RAM
- 60 GB Storage

## 2. Requirement

### 조건

- 4가지 이항 연산을 (덧셈(+), 뺄셈(-), 곱셈(*), 나머지(%)) 제공하는 새로운 시스템 콜 함수 4개를 커널에 등록
- 테스트 프로그램을 (4가지 연산만을 수행하는 이항 계산기 프로그램) 작성하여 새롭게 등록된 시스템 콜 호출 확인

### 출력

- 테스트 프로그램

    ``` bash
    $ ./mycalculator 
    >> 1+2
    3
    >> 3+5
    8
    >> 4+2
    6
    ```

- 커널 로그

    ``` bash
    $ dmesg 

    ...

    [260376.527302] TCP: request_sock_TCP: Possible SYN flooding on port 22. Sending cookies.  Check SNMP counters.
    [435056.713901] sys_my_add: a=1, b=2, result=3
    [435059.119871] sys_my_add: a=3, b=5, result=8
    [435061.672088] sys_my_add: a=4, b=2, result=6
    ```
