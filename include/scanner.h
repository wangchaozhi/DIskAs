#ifndef SCANNER_H
#define SCANNER_H

#include <stdint.h>
#include "disk_io.h"
#include "signature.h"

// 扫描结果结构
typedef struct {
    uint64_t offset;          // 文件在磁盘上的偏移
    uint64_t size;            // 文件大小（估算）
    file_type_t type;         // 文件类型
    uint8_t confidence;       // 置信度（0-100）
} scan_result_t;

// 扫描回调函数类型
typedef void (*scan_callback_t)(const scan_result_t* result, void* user_data);

// 扫描选项
typedef struct {
    uint64_t start_offset;    // 起始偏移
    uint64_t end_offset;      // 结束偏移（0表示扫描到末尾）
    uint32_t block_size;      // 扫描块大小
    uint8_t deep_scan;        // 是否深度扫描
    scan_callback_t callback; // 进度回调
    void* user_data;          // 用户数据
} scan_options_t;

/**
 * 初始化扫描器
 * @return 成功返回 0，失败返回 -1
 */
int scanner_init(void);

/**
 * 扫描磁盘查找可恢复的文件
 * @param handle 磁盘句柄
 * @param options 扫描选项
 * @param results 扫描结果数组（输出）
 * @param max_results 最大结果数
 * @return 实际找到的文件数量，失败返回 -1
 */
int scanner_scan(disk_handle_t* handle, const scan_options_t* options,
                scan_result_t* results, int max_results);

/**
 * 快速扫描（基于文件系统）
 * @param handle 磁盘句柄
 * @param results 扫描结果数组（输出）
 * @param max_results 最大结果数
 * @return 实际找到的文件数量，失败返回 -1
 */
int scanner_quick_scan(disk_handle_t* handle, scan_result_t* results, int max_results);

/**
 * 深度扫描（基于文件签名）
 * @param handle 磁盘句柄
 * @param results 扫描结果数组（输出）
 * @param max_results 最大结果数
 * @return 实际找到的文件数量，失败返回 -1
 */
int scanner_deep_scan(disk_handle_t* handle, scan_result_t* results, int max_results);

/**
 * 清理扫描器
 */
void scanner_cleanup(void);

#endif // SCANNER_H

