#ifndef DISK_IO_H
#define DISK_IO_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

// 磁盘句柄结构
typedef struct {
    int fd;                    // 文件描述符
    uint64_t size;            // 磁盘大小（字节）
    uint32_t sector_size;     // 扇区大小
    char device_path[256];    // 设备路径
} disk_handle_t;

/**
 * 打开磁盘设备
 * @param device_path 设备路径（如 /dev/sda 或磁盘镜像文件）
 * @return 磁盘句柄指针，失败返回 NULL
 */
disk_handle_t* disk_open(const char* device_path);

/**
 * 关闭磁盘设备
 * @param handle 磁盘句柄
 */
void disk_close(disk_handle_t* handle);

/**
 * 从磁盘读取数据
 * @param handle 磁盘句柄
 * @param offset 偏移量（字节）
 * @param buffer 数据缓冲区
 * @param size 读取大小（字节）
 * @return 实际读取的字节数，失败返回 -1
 */
ssize_t disk_read(disk_handle_t* handle, uint64_t offset, void* buffer, size_t size);

/**
 * 读取指定扇区
 * @param handle 磁盘句柄
 * @param sector 扇区号
 * @param count 扇区数量
 * @param buffer 数据缓冲区
 * @return 实际读取的字节数，失败返回 -1
 */
ssize_t disk_read_sectors(disk_handle_t* handle, uint64_t sector, uint32_t count, void* buffer);

/**
 * 获取磁盘大小
 * @param handle 磁盘句柄
 * @return 磁盘大小（字节）
 */
uint64_t disk_get_size(disk_handle_t* handle);

#endif // DISK_IO_H

