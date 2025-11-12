#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "disk_io.h"
#include "signature.h"
#include "file_system.h"
#include "scanner.h"
#include "recovery.h"
#include "utils.h"

#define VERSION "1.0.0"
#define MAX_SCAN_RESULTS 10000

// 扫描模式
typedef enum {
    SCAN_MODE_QUICK,
    SCAN_MODE_DEEP,
    SCAN_MODE_AUTO
} scan_mode_t;

// 程序配置
typedef struct {
    char device_path[512];
    char output_dir[512];
    scan_mode_t scan_mode;
    int auto_recover;
    int verify;
    int show_info;
    int list_only;
} config_t;

void print_banner(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║          DiskAS - 磁盘文件恢复工具 v%s                ║\n", VERSION);
    printf("║          Disk File Recovery Software                      ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
}

void print_usage(const char* program) {
    printf("用法: %s [选项] <设备路径>\n\n", program);
    printf("选项:\n");
    printf("  -h, --help              显示此帮助信息\n");
    printf("  -v, --version           显示版本信息\n");
    printf("  -i, --info              显示设备和文件系统信息\n");
    printf("  -m, --mode <模式>       扫描模式: quick(快速), deep(深度), auto(自动)\n");
    printf("                          默认: auto\n");
    printf("  -o, --output <目录>     恢复文件的输出目录\n");
    printf("                          默认: ./recovered\n");
    printf("  -l, --list              仅列出可恢复的文件，不执行恢复\n");
    printf("  -r, --recover           自动恢复所有找到的文件\n");
    printf("  -V, --verify            验证恢复的文件完整性\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s -i /dev/sdb1                    # 显示设备信息\n", program);
    printf("  %s -l disk_image.img               # 列出可恢复文件\n", program);
    printf("  %s -m deep -o output /dev/sdb1     # 深度扫描并恢复\n", program);
    printf("  %s -r -V disk_image.img            # 自动恢复并验证\n", program);
    printf("\n");
    printf("警告: 请确保对设备有读取权限，某些操作可能需要 root 权限。\n");
    printf("      建议在磁盘镜像上进行恢复操作，避免修改原始设备。\n");
}

void print_version(void) {
    printf("DiskAS v%s\n", VERSION);
    printf("编译日期: %s %s\n", __DATE__, __TIME__);
}

void show_device_info(disk_handle_t* handle) {
    printf("═══════════════════════════════════════════════════════\n");
    printf("设备信息:\n");
    printf("═══════════════════════════════════════════════════════\n");
    
    char size_buf[32];
    printf("  路径: %s\n", handle->device_path);
    printf("  大小: %s\n", utils_format_size(handle->size, size_buf, sizeof(size_buf)));
    printf("  扇区大小: %u bytes\n", handle->sector_size);
    
    // 检测文件系统
    fs_info_t fs_info;
    if (fs_parse_info(handle, &fs_info) == 0) {
        printf("\n文件系统信息:\n");
        printf("  类型: %s\n", fs_get_type_name(fs_info.type));
        printf("  卷标: %s\n", fs_info.label[0] ? fs_info.label : "(无)");
        printf("  簇大小: %s\n", utils_format_size(fs_info.cluster_size, size_buf, sizeof(size_buf)));
        printf("  总簇数: %llu\n", (unsigned long long)fs_info.total_clusters);
        printf("  总大小: %s\n", utils_format_size(fs_info.total_size, size_buf, sizeof(size_buf)));
    } else {
        printf("\n文件系统: 未知或无法识别\n");
    }
    
    printf("═══════════════════════════════════════════════════════\n\n");
}

void list_scan_results(const scan_result_t* results, int count) {
    if (count == 0) {
        printf("没有找到可恢复的文件。\n");
        return;
    }
    
    printf("\n═══════════════════════════════════════════════════════\n");
    printf("找到 %d 个可恢复的文件:\n", count);
    printf("═══════════════════════════════════════════════════════\n");
    printf("%-6s %-12s %-15s %-30s\n", "序号", "偏移", "大小", "类型");
    printf("───────────────────────────────────────────────────────\n");
    
    char size_buf[32];
    for (int i = 0; i < count; i++) {
        const scan_result_t* result = &results[i];
        printf("%-6d 0x%-10llx %-15s %-30s\n",
               i + 1,
               (unsigned long long)result->offset,
               utils_format_size(result->size, size_buf, sizeof(size_buf)),
               signature_get_description(result->type));
    }
    
    printf("═══════════════════════════════════════════════════════\n\n");
}

int main(int argc, char* argv[]) {
    config_t config = {
        .device_path = "",
        .output_dir = "./recovered",
        .scan_mode = SCAN_MODE_AUTO,
        .auto_recover = 0,
        .verify = 0,
        .show_info = 0,
        .list_only = 0
    };

    // 解析命令行参数
    static struct option long_options[] = {
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0, 'v'},
        {"info",    no_argument,       0, 'i'},
        {"mode",    required_argument, 0, 'm'},
        {"output",  required_argument, 0, 'o'},
        {"list",    no_argument,       0, 'l'},
        {"recover", no_argument,       0, 'r'},
        {"verify",  no_argument,       0, 'V'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "hvim:o:lrV", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_banner();
                print_usage(argv[0]);
                return 0;
            case 'v':
                print_version();
                return 0;
            case 'i':
                config.show_info = 1;
                break;
            case 'm':
                if (strcmp(optarg, "quick") == 0) {
                    config.scan_mode = SCAN_MODE_QUICK;
                } else if (strcmp(optarg, "deep") == 0) {
                    config.scan_mode = SCAN_MODE_DEEP;
                } else if (strcmp(optarg, "auto") == 0) {
                    config.scan_mode = SCAN_MODE_AUTO;
                } else {
                    fprintf(stderr, "错误: 未知的扫描模式 '%s'\n", optarg);
                    return 1;
                }
                break;
            case 'o':
                strncpy(config.output_dir, optarg, sizeof(config.output_dir) - 1);
                break;
            case 'l':
                config.list_only = 1;
                break;
            case 'r':
                config.auto_recover = 1;
                break;
            case 'V':
                config.verify = 1;
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // 检查设备路径参数
    if (optind >= argc) {
        fprintf(stderr, "错误: 未指定设备路径\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    strncpy(config.device_path, argv[optind], sizeof(config.device_path) - 1);

    // 显示横幅
    print_banner();

    // 初始化扫描器
    if (scanner_init() < 0) {
        fprintf(stderr, "错误: 扫描器初始化失败\n");
        return 1;
    }

    // 打开设备
    printf("正在打开设备: %s\n\n", config.device_path);
    disk_handle_t* handle = disk_open(config.device_path);
    if (!handle) {
        fprintf(stderr, "错误: 无法打开设备\n");
        return 1;
    }

    // 显示设备信息
    if (config.show_info) {
        show_device_info(handle);
        if (!config.auto_recover && !config.list_only) {
            disk_close(handle);
            scanner_cleanup();
            return 0;
        }
    }

    // 分配扫描结果数组
    scan_result_t* results = (scan_result_t*)malloc(MAX_SCAN_RESULTS * sizeof(scan_result_t));
    if (!results) {
        fprintf(stderr, "错误: 内存分配失败\n");
        disk_close(handle);
        return 1;
    }

    // 执行扫描
    int found_count = 0;
    printf("\n开始扫描...\n");
    
    switch (config.scan_mode) {
        case SCAN_MODE_QUICK:
            printf("扫描模式: 快速扫描（基于文件系统）\n\n");
            found_count = scanner_quick_scan(handle, results, MAX_SCAN_RESULTS);
            break;
            
        case SCAN_MODE_DEEP:
            printf("扫描模式: 深度扫描（基于文件签名）\n\n");
            found_count = scanner_deep_scan(handle, results, MAX_SCAN_RESULTS);
            break;
            
        case SCAN_MODE_AUTO:
            printf("扫描模式: 自动模式（先快速后深度）\n\n");
            found_count = scanner_quick_scan(handle, results, MAX_SCAN_RESULTS);
            if (found_count == 0) {
                printf("\n快速扫描未找到文件，切换到深度扫描...\n\n");
                found_count = scanner_deep_scan(handle, results, MAX_SCAN_RESULTS);
            }
            break;
    }

    if (found_count < 0) {
        fprintf(stderr, "错误: 扫描失败\n");
        free(results);
        disk_close(handle);
        return 1;
    }

    // 列出扫描结果
    list_scan_results(results, found_count);

    // 执行恢复
    if (config.auto_recover && found_count > 0) {
        printf("开始恢复文件...\n");
        
        recovery_options_t recovery_opts = {
            .output_dir = config.output_dir,
            .overwrite = 0,
            .verify = config.verify
        };
        
        int recovered = recovery_recover_batch(handle, results, found_count, &recovery_opts);
        
        printf("\n恢复完成: %d/%d 文件成功恢复\n", recovered, found_count);
    } else if (config.list_only && found_count > 0) {
        printf("提示: 使用 -r 选项可以恢复这些文件\n");
    } else if (found_count == 0) {
        printf("未找到可恢复的文件。\n");
        printf("提示: 尝试使用 -m deep 进行深度扫描\n");
    }

    // 清理
    free(results);
    disk_close(handle);
    scanner_cleanup();

    return 0;
}
