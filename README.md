# Scheduler Test

리눅스 커널의 스케줄링 정책을 (기본 CFS, 조정된 NICE 값을 적용한 CFS) 확인할 수 있는 프로그램 작성

## 0. Quick start

``` bash
git clone https://github.com/codejune/c-scheduler-test.git
cd c-scheduler-test

# 일괄 빌드
make

# 개별 빌드
make cfs
./cfs
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

### 입력

- fork()를 통해 총 21개의 자식 프로세스를 생성

### 조건

- 그룹으로 나누어 Nice 값을 적용하는 CFS의 경우 Nice 값이 낮은 프로세스 그룹부터 우선적으로 수행
- 정책에 뚜렷한 차이가 나타나도록 프로세스는 별도의 단순 연산을 수행
- 생성되는 21개의 프로세스들의 PID들을 프로세스의 생성 및 종료 시점에 출력

### 출력

- 생성되는 프로세스들에게 Nice 값을 조정하지 않고 적용되는 기본 CFS 정책을 적용하는 프로그램
- 생성되는 프로세스들에게 3개의 그룹(높은 Nice 값, 기본 Nice 값, 낮은 Nice 값)으로 나누어 적용하는 CFS 정책을 적용하는 프로그램

### 예시

- 기본 CFS 정책 적용

  ``` bash
  $ ./cfs
  Parent running,  PID: 274471
  ++  1 Child created, PID: 274472
  ++  2 Child created, PID: 274473
  ++  3 Child created, PID: 274474
  ++  6 Child created, PID: 274477
  ++  5 Child created, PID: 274476
  ++ 11 Child created, PID: 274482
  ++  4 Child created, PID: 274475
  ++ 14 Child created, PID: 274485
  ++  9 Child created, PID: 274480
  ++ 10 Child created, PID: 274481
  ++ 16 Child created, PID: 274487
  ++ 12 Child created, PID: 274483
  ++ 13 Child created, PID: 274484
  ++ 17 Child created, PID: 274488
  ++ 15 Child created, PID: 274486
  ++ 18 Child created, PID: 274489
  ++  7 Child created, PID: 274478
  ++ 19 Child created, PID: 274490
  ++  8 Child created, PID: 274479
  ++ 20 Child created, PID: 274491
  ++ 21 Child created, PID: 274492
  -- 16 Child terminated, PID: 274487
  -- 11 Child terminated, PID: 274482
  --  4 Child terminated, PID: 274475
  --  7 Child terminated, PID: 274478
  -- 10 Child terminated, PID: 274481
  --  3 Child terminated, PID: 274474
  --  9 Child terminated, PID: 274480
  -- 14 Child terminated, PID: 274485
  -- 18 Child terminated, PID: 274489
  -- 20 Child terminated, PID: 274491
  --  1 Child terminated, PID: 274472
  --  2 Child terminated, PID: 274473
  --  6 Child terminated, PID: 274477
  -- 12 Child terminated, PID: 274483
  -- 15 Child terminated, PID: 274486
  --  8 Child terminated, PID: 274479
  -- 17 Child terminated, PID: 274488
  -- 19 Child terminated, PID: 274490
  --  5 Child terminated, PID: 274476
  -- 21 Child terminated, PID: 274492
  -- 13 Child terminated, PID: 274484
  Runtime: 1:068653(sec:usec)
  ```

- 그룹별 NICE 조정 CFS 정책 적용

  ``` bash
  $ ./cfs_nice
  Parent running,  PID: 274619
  ++ NICE( 19) Child created, PID: 274620
  ++ NICE( 19) Child created, PID: 274621
  ++ NICE( 19) Child created, PID: 274623
  ++ NICE( 19) Child created, PID: 274622
  ++ NICE(  0) Child created, PID: 274627
  ++ NICE(  0) Child created, PID: 274630
  ++ NICE(  0) Child created, PID: 274632
  ++ NICE(  0) Child created, PID: 274633
  ++ NICE(  0) Child created, PID: 274631
  ++ NICE(-20) Child created, PID: 274637
  ++ NICE(-20) Child created, PID: 274634
  ++ NICE( 19) Child created, PID: 274626
  ++ NICE( 19) Child created, PID: 274625
  ++ NICE(-20) Child created, PID: 274635
  ++ NICE(  0) Child created, PID: 274628
  ++ NICE(-20) Child created, PID: 274636
  ++ NICE(-20) Child created, PID: 274638
  ++ NICE(  0) Child created, PID: 274629
  ++ NICE(-20) Child created, PID: 274639
  ++ NICE( 19) Child created, PID: 274624
  ++ NICE(-20) Child created, PID: 274640
  -- NICE(-20) Child terminated, PID: 274638
  -- NICE(-20) Child terminated, PID: 274637
  -- NICE(-20) Child terminated, PID: 274634
  -- NICE(-20) Child terminated, PID: 274635
  -- NICE(-20) Child terminated, PID: 274639
  -- NICE(-20) Child terminated, PID: 274636
  -- NICE(-20) Child terminated, PID: 274640
  -- NICE(  0) Child terminated, PID: 274629
  -- NICE(  0) Child terminated, PID: 274627
  -- NICE(  0) Child terminated, PID: 274628
  -- NICE(  0) Child terminated, PID: 274630
  -- NICE(  0) Child terminated, PID: 274633
  -- NICE( 19) Child terminated, PID: 274623
  -- NICE(  0) Child terminated, PID: 274632
  -- NICE(  0) Child terminated, PID: 274631
  -- NICE( 19) Child terminated, PID: 274621
  -- NICE( 19) Child terminated, PID: 274620
  -- NICE( 19) Child terminated, PID: 274626
  -- NICE( 19) Child terminated, PID: 274624
  -- NICE( 19) Child terminated, PID: 274625
  -- NICE( 19) Child terminated, PID: 274622
  Runtime: 3:102885(sec:usec)
  ```
