/**
 * @file mylscpu.h
 * @author 김병준 (kbj9704@gmail.com)
 * @brief mylscpu 명령어 구현에 필요한 헤더, 함수 및 매크로 정의
 */
#ifndef MYLSCPU_H
#define MYLSCPU_H

// header
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <sys/utsname.h>
#include <inttypes.h>

// Macro
#define MIN_BUFFER_SIZE 64
#define BUFFER_SIZE 256
#define MAX_BUFFER_SIZE 1024

#define CPU_INFO_PATH "/proc/cpuinfo"
#define CPU_MAX_FREQUENCY_PATH "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"
#define CPU_MIN_FREQUENCY_PATH "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq"
#define CPU_ONLINE_PATH "/sys/devices/system/cpu/online"

#define NUMA_NODE_PATH "/sys/devices/system/node"

#define VULNERABILITY_ITLB_MULTIHIT_PATH "/sys/devices/system/cpu/vulnerabilities/itlb_multihit";
#define VULNERABILITY_L1TF_PATH "/sys/devices/system/cpu/vulnerabilities/l1tf";
#define VULNERABILITY_MDS_PATH "/sys/devices/system/cpu/vulnerabilities/mds";
#define VULNERABILITY_MELTDOWN_PATH "/sys/devices/system/cpu/vulnerabilities/meltdown";
#define VULNERABILITY_SEPC_STORE_BYPASS_PATH "/sys/devices/system/cpu/vulnerabilities/spec_store_bypass";
#define VULNERABILITY_SPECTRE_V1_PATH "/sys/devices/system/cpu/vulnerabilities/spectre_v1";
#define VULNERABILITY_SPECTRE_V2_PATH "/sys/devices/system/cpu/vulnerabilities/spectre_v2";
#define VULNERABILITY_SRBDS_PATH "/sys/devices/system/cpu/vulnerabilities/srbds";
#define VULNERABILITY_TSX_ASYNC_ABORT_PATH "/sys/devices/system/cpu/vulnerabilities/tsx_async_abort";

#define L1D_CACHE_TYPE_PATH "/sys/devices/system/cpu/cpu0/cache/index0/type";
#define L1D_CACHE_LEVEL_PATH "/sys/devices/system/cpu/cpu0/cache/index0/level";
#define L1I_CACHE_TYPE_PATH "/sys/devices/system/cpu/cpu0/cache/index1/type";
#define L1I_CACHE_LEVEL_PATH "/sys/devices/system/cpu/cpu0/cache/index1/level";
#define L2_CACHE_TYPE_PATH "/sys/devices/system/cpu/cpu0/cache/index2/type";
#define L2_CACHE_LEVEL_PATH "/sys/devices/system/cpu/cpu0/cache/index2/level";
#define L3_CACHE_TYPE_PATH "/sys/devices/system/cpu/cpu0/cache/index3/type";
#define L3_CACHE_LEVEL_PATH "/sys/devices/system/cpu/cpu0/cache/index3/level";

// Struct
typedef struct Lcache {
	uint64_t one_size;
	uint64_t all_size;
	uint8_t ways;
	char type[BUFFER_SIZE];
	uint8_t level;
} cache;

typedef struct Numa {
    char cpulist[BUFFER_SIZE];
} numa;

typedef struct Cpuinfo
{
    char architecture[BUFFER_SIZE];
    char op_mode[BUFFER_SIZE];
    char byte_order[BUFFER_SIZE];
    char address_size[BUFFER_SIZE];
    uint8_t cpu_count;
    char online_cpu[BUFFER_SIZE];
    uint8_t thread_per_core;
    uint8_t core_per_socket;
    uint8_t socket_count;
    uint8_t numa_node_count;
    char vendor_id[BUFFER_SIZE];
    uint8_t cpu_family;
    uint8_t model_number;
    char model_name[BUFFER_SIZE];
    uint8_t stepping;
    float cpu_clock;
    float cpu_clock_min;
    float cpu_clock_max;
    float bogo_mips;
    char virtualization[BUFFER_SIZE];
    cache l1d_cache;
    cache l1i_cache;
    cache l2_cache;
    cache l3_cache;
    numa *numa_node;
    char vulnerability_itlb_multihit[BUFFER_SIZE];
    char vulnerability_l1tf[BUFFER_SIZE];
    char vulnerability_mds[BUFFER_SIZE];
    char vulnerability_meltdown[BUFFER_SIZE];
    char vulnerability_spec_store_bypass[BUFFER_SIZE];
    char vulnerability_spectre_v1[BUFFER_SIZE];
    char vulnerability_spectre_v2[BUFFER_SIZE];
    char vulnerability_srbds[BUFFER_SIZE];
    char vulnerability_tsx_async_abort[BUFFER_SIZE];
    char flags[MAX_BUFFER_SIZE];
    uint16_t flag_count;
} cpuinfo;

// Glabal variable
struct winsize WINDOW_SIZE;
cpuinfo g_cpuinfo;

// Function prototype
void get_cpu_info(void);
void trim_string(char *buffer);
char **get_tokenization(char *buffer, char divider);
char *get_serialization(char **tokens, uint32_t token_count);
void get_cpu_online(void);
void get_cpu_clock(void);
void get_vulnerability(void);
void get_lcache_info(void);
void get_numa_node(void);
void get_architecture(void);
void get_byte_order(void);
void print_lscpu(void);
char *byte_to_human(uint64_t bytes);
#endif