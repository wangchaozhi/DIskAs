#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#define _POSIX_C_SOURCE 200809L

char* utils_format_size(uint64_t bytes, char* buffer, size_t size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int unit_index = 0;
    double size_d = (double)bytes;

    while (size_d >= 1024.0 && unit_index < 5) {
        size_d /= 1024.0;
        unit_index++;
    }

    if (unit_index == 0) {
        snprintf(buffer, size, "%llu %s", (unsigned long long)bytes, units[unit_index]);
    } else {
        snprintf(buffer, size, "%.2f %s", size_d, units[unit_index]);
    }

    return buffer;
}

int utils_mkdir_recursive(const char* path) {
    if (!path || path[0] == '\0') {
        return -1;
    }

    char* path_copy = strdup(path);
    if (!path_copy) {
        return -1;
    }

    char* p = path_copy;
    
    // 跳过根目录斜杠
    if (*p == '/') {
        p++;
    }

    for (; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            
            // 创建目录
            if (mkdir(path_copy, 0755) < 0 && errno != EEXIST) {
                free(path_copy);
                return -1;
            }
            
            *p = '/';
        }
    }

    // 创建最后一级目录
    if (mkdir(path_copy, 0755) < 0 && errno != EEXIST) {
        free(path_copy);
        return -1;
    }

    free(path_copy);
    return 0;
}

int utils_file_exists(const char* path) {
    if (!path) {
        return 0;
    }
    return access(path, F_OK) == 0;
}

char* utils_generate_unique_filename(const char* base_path, 
                                     const char* extension,
                                     char* buffer, 
                                     size_t size) {
    if (!base_path || !extension || !buffer || size == 0) {
        return NULL;
    }

    // 首先尝试不带编号的文件名
    snprintf(buffer, size, "%s.%s", base_path, extension);
    if (!utils_file_exists(buffer)) {
        return buffer;
    }

    // 如果文件存在，添加编号
    for (int i = 1; i < 10000; i++) {
        snprintf(buffer, size, "%s_%d.%s", base_path, i, extension);
        if (!utils_file_exists(buffer)) {
            return buffer;
        }
    }

    // 如果还是找不到可用名称，使用时间戳
    time_t now = time(NULL);
    snprintf(buffer, size, "%s_%ld.%s", base_path, (long)now, extension);
    
    return buffer;
}

int utils_calculate_progress(uint64_t current, uint64_t total) {
    if (total == 0) {
        return 0;
    }
    
    uint64_t progress = (current * 100) / total;
    if (progress > 100) {
        progress = 100;
    }
    
    return (int)progress;
}

void utils_show_progress(int progress, const char* message) {
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;

    const int bar_width = 50;
    int filled = (progress * bar_width) / 100;

    printf("\r[");
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) {
            printf("=");
        } else if (i == filled) {
            printf(">");
        } else {
            printf(" ");
        }
    }
    printf("] %3d%% %s", progress, message ? message : "");
    
    if (progress >= 100) {
        printf("\n");
    }
    
    fflush(stdout);
}

void utils_hex_dump(const uint8_t* data, size_t size, uint64_t offset) {
    if (!data || size == 0) {
        return;
    }

    const int bytes_per_line = 16;
    
    for (size_t i = 0; i < size; i += bytes_per_line) {
        // 打印偏移
        printf("%08llx  ", (unsigned long long)(offset + i));
        
        // 打印十六进制
        for (int j = 0; j < bytes_per_line; j++) {
            if (i + j < size) {
                printf("%02x ", data[i + j]);
            } else {
                printf("   ");
            }
            
            if (j == 7) {
                printf(" ");
            }
        }
        
        printf(" |");
        
        // 打印 ASCII
        for (int j = 0; j < bytes_per_line && i + j < size; j++) {
            uint8_t c = data[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        
        printf("|\n");
    }
}

int utils_memcasecmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;

    for (size_t i = 0; i < n; i++) {
        int diff = tolower(p1[i]) - tolower(p2[i]);
        if (diff != 0) {
            return diff;
        }
    }

    return 0;
}

