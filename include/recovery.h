#ifndef RECOVERY_H
#define RECOVERY_H

#include <stdint.h>
#include "disk_io.h"
#include "scanner.h"

// 恢复状态
typedef enum {
    RECOVERY_SUCCESS = 0,
    RECOVERY_FAILED,
    RECOVERY_PARTIAL,
    RECOVERY_CANCELED
} recovery_status_t;

// 恢复选项
typedef struct {
    const char* output_dir;   // 输出目录
    uint8_t overwrite;        // 是否覆盖已存在文件
    uint8_t verify;           // 是否验证恢复的文件
} recovery_options_t;

/**
 * 恢复单个文件
 * @param handle 磁盘句柄
 * @param result 扫描结果
 * @param output_path 输出文件路径
 * @return 恢复状态
 */
recovery_status_t recovery_recover_file(disk_handle_t* handle, 
                                       const scan_result_t* result,
                                       const char* output_path);

/**
 * 批量恢复文件
 * @param handle 磁盘句柄
 * @param results 扫描结果数组
 * @param count 结果数量
 * @param options 恢复选项
 * @return 成功恢复的文件数量
 */
int recovery_recover_batch(disk_handle_t* handle,
                          const scan_result_t* results,
                          int count,
                          const recovery_options_t* options);

/**
 * 获取恢复状态描述
 * @param status 恢复状态
 * @return 状态描述字符串
 */
const char* recovery_get_status_desc(recovery_status_t status);

/**
 * 验证恢复的文件完整性
 * @param file_path 文件路径
 * @return 1表示完整，0表示损坏
 */
int recovery_verify_file(const char* file_path);

#endif // RECOVERY_H

