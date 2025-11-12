#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>
#include <time.h>
#include "disk_io.h"

// 文件系统类型
typedef enum {
    FS_TYPE_UNKNOWN = 0,
    FS_TYPE_FAT12,
    FS_TYPE_FAT16,
    FS_TYPE_FAT32,
    FS_TYPE_NTFS,
    FS_TYPE_EXT2,
    FS_TYPE_EXT3,
    FS_TYPE_EXT4
} fs_type_t;

// 文件条目结构
typedef struct {
    char name[256];           // 文件名
    uint64_t size;            // 文件大小
    uint64_t cluster;         // 起始簇号
    time_t create_time;       // 创建时间
    time_t modify_time;       // 修改时间
    uint8_t is_deleted;       // 是否已删除
    uint8_t is_directory;     // 是否为目录
    fs_type_t fs_type;        // 文件系统类型
} file_entry_t;

// 文件系统信息
typedef struct {
    fs_type_t type;           // 文件系统类型
    uint64_t total_size;      // 总大小
    uint64_t cluster_size;    // 簇大小
    uint64_t total_clusters;  // 总簇数
    uint64_t root_cluster;    // 根目录簇号
    uint64_t fat_offset;      // FAT表偏移
    uint64_t data_offset;     // 数据区偏移
    char label[32];           // 卷标
} fs_info_t;

/**
 * 检测文件系统类型
 * @param handle 磁盘句柄
 * @return 文件系统类型
 */
fs_type_t fs_detect_type(disk_handle_t* handle);

/**
 * 解析文件系统信息
 * @param handle 磁盘句柄
 * @param info 文件系统信息结构（输出）
 * @return 成功返回 0，失败返回 -1
 */
int fs_parse_info(disk_handle_t* handle, fs_info_t* info);

/**
 * 扫描已删除的文件
 * @param handle 磁盘句柄
 * @param info 文件系统信息
 * @param entries 文件条目数组（输出）
 * @param max_entries 最大条目数
 * @return 实际找到的文件数量
 */
int fs_scan_deleted_files(disk_handle_t* handle, const fs_info_t* info, 
                         file_entry_t* entries, int max_entries);

/**
 * 获取文件系统类型名称
 * @param type 文件系统类型
 * @return 类型名称字符串
 */
const char* fs_get_type_name(fs_type_t type);

#endif // FILE_SYSTEM_H

