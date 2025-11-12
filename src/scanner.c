#include "scanner.h"
#include "signature.h"
#include "file_system.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_BLOCK_SIZE (1024 * 1024)  // 1MB
#define SCAN_BUFFER_SIZE (64 * 1024)      // 64KB

static int initialized = 0;

int scanner_init(void) {
    if (initialized) {
        return 0;
    }

    signature_init();
    initialized = 1;
    
    printf("Scanner initialized\n");
    return 0;
}

// 估算文件大小（基于文件类型）
static uint64_t estimate_file_size(disk_handle_t* handle, uint64_t offset, file_type_t type) {
    // 对于某些文件类型，可以从文件头读取大小
    uint8_t buffer[512];
    if (disk_read(handle, offset, buffer, sizeof(buffer)) < 0) {
        return 0;
    }

    switch (type) {
        case FILE_TYPE_JPEG: {
            // JPEG 文件以 FF D9 结束
            uint64_t max_search = 50 * 1024 * 1024; // 最多搜索 50MB
            uint8_t search_buf[4096];
            for (uint64_t pos = offset + 2; pos < offset + max_search; pos += sizeof(search_buf) - 1) {
                ssize_t read_size = disk_read(handle, pos, search_buf, sizeof(search_buf));
                if (read_size <= 0) break;
                
                for (int i = 0; i < read_size - 1; i++) {
                    if (search_buf[i] == 0xFF && search_buf[i + 1] == 0xD9) {
                        return (pos + i + 2) - offset;
                    }
                }
            }
            return 1024 * 1024; // 默认 1MB
        }
        
        case FILE_TYPE_PNG: {
            // PNG 文件大小在 IHDR 块之后
            if (buffer[0] == 0x89 && buffer[1] == 0x50) {
                // 读取到文件末尾标记 IEND
                uint64_t max_search = 20 * 1024 * 1024;
                uint8_t search_buf[4096];
                for (uint64_t pos = offset + 8; pos < offset + max_search; pos += sizeof(search_buf) - 4) {
                    ssize_t read_size = disk_read(handle, pos, search_buf, sizeof(search_buf));
                    if (read_size <= 0) break;
                    
                    for (int i = 0; i < read_size - 4; i++) {
                        if (memcmp(&search_buf[i], "IEND", 4) == 0) {
                            return (pos + i + 8) - offset;
                        }
                    }
                }
            }
            return 512 * 1024; // 默认 512KB
        }
        
        case FILE_TYPE_PDF: {
            // 搜索 %%EOF 标记
            uint64_t max_search = 100 * 1024 * 1024;
            uint8_t search_buf[4096];
            for (uint64_t pos = offset + 5; pos < offset + max_search; pos += sizeof(search_buf) - 5) {
                ssize_t read_size = disk_read(handle, pos, search_buf, sizeof(search_buf));
                if (read_size <= 0) break;
                
                for (int i = 0; i < read_size - 5; i++) {
                    if (memcmp(&search_buf[i], "%%EOF", 5) == 0) {
                        return (pos + i + 5) - offset;
                    }
                }
            }
            return 2 * 1024 * 1024; // 默认 2MB
        }
        
        case FILE_TYPE_ZIP:
        case FILE_TYPE_DOCX:
        case FILE_TYPE_XLSX:
        case FILE_TYPE_PPTX: {
            // ZIP 格式，查找 End of Central Directory
            uint64_t max_search = 200 * 1024 * 1024;
            uint8_t search_buf[4096];
            for (uint64_t pos = offset + 4; pos < offset + max_search; pos += sizeof(search_buf) - 22) {
                ssize_t read_size = disk_read(handle, pos, search_buf, sizeof(search_buf));
                if (read_size <= 0) break;
                
                for (int i = 0; i < read_size - 22; i++) {
                    if (search_buf[i] == 0x50 && search_buf[i+1] == 0x4B && 
                        search_buf[i+2] == 0x05 && search_buf[i+3] == 0x06) {
                        // 找到 EOCD 标记
                        uint16_t comment_len = *(uint16_t*)(&search_buf[i + 20]);
                        return (pos + i + 22 + comment_len) - offset;
                    }
                }
            }
            return 5 * 1024 * 1024; // 默认 5MB
        }
        
        default:
            // 对于未知类型，返回一个合理的默认大小
            return 1024 * 1024; // 1MB
    }
}

int scanner_scan(disk_handle_t* handle, const scan_options_t* options,
                scan_result_t* results, int max_results) {
    if (!handle || !options || !results || max_results <= 0) {
        return -1;
    }

    if (!initialized) {
        scanner_init();
    }

    uint64_t start = options->start_offset;
    uint64_t end = options->end_offset ? options->end_offset : disk_get_size(handle);
    uint32_t block_size = options->block_size ? options->block_size : DEFAULT_BLOCK_SIZE;
    
    uint8_t* buffer = (uint8_t*)malloc(block_size);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return -1;
    }

    int found_count = 0;
    uint64_t current_offset = start;

    printf("Scanning from offset 0x%llx to 0x%llx...\n", 
           (unsigned long long)start, (unsigned long long)end);

    while (current_offset < end && found_count < max_results) {
        // 读取数据块
        size_t read_size = (current_offset + block_size <= end) ? 
                          block_size : (end - current_offset);
        
        ssize_t bytes_read = disk_read(handle, current_offset, buffer, read_size);
        if (bytes_read <= 0) {
            break;
        }

        // 扫描数据块中的文件签名
        for (size_t i = 0; i < (size_t)bytes_read - 16 && found_count < max_results; i++) {
            file_type_t type = signature_identify(&buffer[i], bytes_read - i);
            
            if (type != FILE_TYPE_UNKNOWN) {
                // 找到一个潜在的文件
                scan_result_t* result = &results[found_count];
                result->offset = current_offset + i;
                result->type = type;
                result->confidence = 80; // 基本置信度
                
                // 估算文件大小
                result->size = estimate_file_size(handle, result->offset, type);
                
                found_count++;
                
                // 跳过已识别的文件内容
                i += (result->size < block_size) ? result->size : block_size;
                
                // 如果设置了回调，调用它
                if (options->callback) {
                    options->callback(result, options->user_data);
                }
            }
        }

        // 更新进度
        int progress = utils_calculate_progress(current_offset - start, end - start);
        utils_show_progress(progress, "Scanning...");

        current_offset += bytes_read;
    }

    utils_show_progress(100, "Scan complete");
    
    free(buffer);
    printf("\nFound %d potential files\n", found_count);
    
    return found_count;
}

int scanner_quick_scan(disk_handle_t* handle, scan_result_t* results, int max_results) {
    if (!handle || !results || max_results <= 0) {
        return -1;
    }

    printf("Performing quick scan (file system analysis)...\n");

    // 检测文件系统类型
    fs_info_t fs_info;
    if (fs_parse_info(handle, &fs_info) < 0) {
        fprintf(stderr, "Error: Cannot parse file system information\n");
        return -1;
    }

    printf("Detected file system: %s\n", fs_get_type_name(fs_info.type));
    
    char size_buf[32];
    printf("Total size: %s\n", utils_format_size(fs_info.total_size, size_buf, sizeof(size_buf)));
    printf("Cluster size: %s\n", utils_format_size(fs_info.cluster_size, size_buf, sizeof(size_buf)));

    // 扫描已删除的文件
    file_entry_t* entries = (file_entry_t*)malloc(max_results * sizeof(file_entry_t));
    if (!entries) {
        return -1;
    }

    int found = fs_scan_deleted_files(handle, &fs_info, entries, max_results);
    
    // 转换为扫描结果
    for (int i = 0; i < found; i++) {
        results[i].offset = fs_info.data_offset + 
                           (entries[i].cluster - 2) * fs_info.cluster_size;
        results[i].size = entries[i].size;
        results[i].type = FILE_TYPE_UNKNOWN; // 需要进一步识别
        results[i].confidence = 90; // 文件系统级别的信息更可靠
    }

    free(entries);
    printf("Quick scan found %d deleted files\n", found);
    
    return found;
}

int scanner_deep_scan(disk_handle_t* handle, scan_result_t* results, int max_results) {
    if (!handle || !results || max_results <= 0) {
        return -1;
    }

    printf("Performing deep scan (signature-based)...\n");

    scan_options_t options = {
        .start_offset = 0,
        .end_offset = 0,  // 扫描整个设备
        .block_size = DEFAULT_BLOCK_SIZE,
        .deep_scan = 1,
        .callback = NULL,
        .user_data = NULL
    };

    return scanner_scan(handle, &options, results, max_results);
}

void scanner_cleanup(void) {
    if (initialized) {
        initialized = 0;
        printf("Scanner cleaned up\n");
    }
}

