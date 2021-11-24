# Virtual Memory Paging Simulator

가상메모리 관리 기법의 하나인 페이지 교체 기법 중 OPT, FIFO, LRU, Second-Chance를 구현하고 동작 과정을 보여주는 시뮬레이터 구현하기

## 0. Quick start

``` bash
$ git clone https://github.com/codejune/c-virtual-memory-simulator.git
$ cd c-virtual-memory-simulator

# 일괄 빌드
$ make

# 개별 빌드
$ make opt 
$ ./opt
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

### 페이징 기법 단축어

- OPT: opt
- FIFO: fifo
- LRU: lru
- Second-Chance: sc

### 입력

- Page frame: min 1, max 4
- Page reference string: min 1, max 30

### 조건

- 실행 초기화면에서 입력 데이터의 파일 이름을 입력 받음
- 입력 데이터에는 Page frame 수, Page reference string 값 포함
- 각 기법간 Paging 과정 및 page fault 발생 출력
- Second-Chance 기법의 경우 참조 기록에 대한 주기적 Refresh는 없다고 가정

### 출력

- Input data

    ``` bash
    $ cat input.txt                        
    3                           # Page frame 수
    2 3 2 1 5 2 4 5 3 2 5 2     # Page reference string
    ```

- Output (OPT)

    ``` bash
    $ ./opt
    Input file path > input.txt
    Used method : OPT
    Page frames: 3
    Page reference string: 2 3 2 1 5 2 4 5 3 2 5 2 

            frame   1       2       3       page fault
    time
    1               2                       F
    2               2       3               F
    3               2       3
    4               2       3       1       F
    5               2       3       5       F
    6               2       3       5
    7               4       3       5       F
    8               4       3       5
    9               4       3       5
    10              2       3       5       F
    11              2       3       5
    12              2       3       5
    Number of page faults: 6 times
    ```

- Output (FIFO)

    ``` bash
    $ ./fifo
    Input file path > input.txt
    Used method : FIFO
    Page frames: 3
    Page reference string: 2 3 2 1 5 2 4 5 3 2 5 2 

            frame   1       2       3       page fault
    time
    1               2                       F
    2               2       3               F
    3               2       3
    4               2       3       1       F
    5               5       3       1       F
    6               5       2       1       F
    7               5       2       4       F
    8               5       2       4
    9               3       2       4       F
    10              3       2       4
    11              3       5       4       F
    12              3       5       2       F
    Number of page faults: 9 times
    ```

- Output (LRU)

    ``` bash
    $ ./lru 
    Input file path > input.txt
    Used method : LRU
    Page frames: 3
    Page reference string: 2 3 2 1 5 2 4 5 3 2 5 2 

            frame   1       2       3       page fault
    time
    1               2                       F
    2               2       3               F
    3               2       3
    4               2       3       1       F
    5               2       5       1       F
    6               2       5       1
    7               2       5       4       F
    8               2       5       4
    9               3       5       4       F
    10              3       5       2       F
    11              3       5       2
    12              3       5       2
    Number of page faults: 7 times
    ```

- Output (Second-Chance)

    ``` bash
    $ ./sc 
    Input file path > input.txt
    Used method : SC
    Page frames: 3
    Page reference string: 2 3 2 1 5 2 4 5 3 2 5 2 

            frame   1       2       3       page fault
    time
    1               2                       F
    2               2       3               F
    3               2       3
    4               2       3       1       F
    5               2       5       1       F
    6               2       5       1
    7               2       5       4       F
    8               2       5       4
    9               2       5       3       F
    10              2       5       3
    11              2       5       3
    12              2       5       3
    Number of page faults: 6 times
    ```
