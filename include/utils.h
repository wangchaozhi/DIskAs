#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>

/**
 * 格式化字节大小为人类可读格式
 * @param bytes 字节数
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @return 格式化后的字符串
 */
char* utils_format_size(uint64_t bytes, char* buffer, size_t size);

/**
 * 创建目录（递归）
 * @param path 目录路径
 * @return 成功返回 0，失败返回 -1
 */
int utils_mkdir_recursive(const char* path);

/**
 * 检查文件是否存在
 * @param path 文件路径
 * @return 存在返回 1，否则返回 0
 */
int utils_file_exists(const char* path);

/**
 * 生成唯一的文件名
 * @param base_path 基础路径
 * @param extension 扩展名
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @return 生成的文件名
 */
char* utils_generate_unique_filename(const char* base_path, 
                                     const char* extension,
                                     char* buffer, 
                                     size_t size);

/**
 * 计算进度百分比
 * @param current 当前值
 * @param total 总值
 * @return 百分比（0-100）
 */
int utils_calculate_progress(uint64_t current, uint64_t total);

/**
 * 显示进度条
 * @param progress 进度百分比（0-100）
 * @param message 消息
 */
void utils_show_progress(int progress, const char* message);

/**
 * 十六进制转储
 * @param data 数据
 * @param size 大小
 * @param offset 起始偏移
 */
void utils_hex_dump(const uint8_t* data, size_t size, uint64_t offset);

/**
 * 比较内存（忽略大小写）
 * @param s1 内存块1
 * @param s2 内存块2
 * @param n 字节数
 * @return 相等返回 0
 */
int utils_memcasecmp(const void* s1, const void* s2, size_t n);

#endif // UTILS_H

