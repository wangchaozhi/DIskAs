#include "recovery.h"
#include "utils.h"
#include "signature.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define RECOVERY_BUFFER_SIZE (1024 * 1024)  // 1MB

recovery_status_t recovery_recover_file(disk_handle_t* handle, 
                                       const scan_result_t* result,
                                       const char* output_path) {
    if (!handle || !result || !output_path) {
        return RECOVERY_FAILED;
    }

    if (result->size == 0) {
        fprintf(stderr, "Error: File size is zero\n");
        return RECOVERY_FAILED;
    }

    printf("Recovering file to: %s\n", output_path);
    printf("  Offset: 0x%llx\n", (unsigned long long)result->offset);
    printf("  Size: %llu bytes\n", (unsigned long long)result->size);
    printf("  Type: %s\n", signature_get_description(result->type));

    // 创建输出文件
    int out_fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) {
        fprintf(stderr, "Error: Cannot create output file: %s\n", strerror(errno));
        return RECOVERY_FAILED;
    }

    // 分配缓冲区
    uint8_t* buffer = (uint8_t*)malloc(RECOVERY_BUFFER_SIZE);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        close(out_fd);
        return RECOVERY_FAILED;
    }

    recovery_status_t status = RECOVERY_SUCCESS;
    uint64_t remaining = result->size;
    uint64_t current_offset = result->offset;
    uint64_t total_recovered = 0;

    while (remaining > 0) {
        // 计算本次读取大小
        size_t read_size = (remaining > RECOVERY_BUFFER_SIZE) ? 
                          RECOVERY_BUFFER_SIZE : remaining;

        // 从磁盘读取数据
        ssize_t bytes_read = disk_read(handle, current_offset, buffer, read_size);
        if (bytes_read <= 0) {
            fprintf(stderr, "Error: Failed to read from disk at offset 0x%llx\n", 
                   (unsigned long long)current_offset);
            status = RECOVERY_PARTIAL;
            break;
        }

        // 写入输出文件
        ssize_t bytes_written = write(out_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "Error: Failed to write to output file: %s\n", 
                   strerror(errno));
            status = RECOVERY_PARTIAL;
            break;
        }

        total_recovered += bytes_read;
        remaining -= bytes_read;
        current_offset += bytes_read;

        // 显示进度
        int progress = utils_calculate_progress(total_recovered, result->size);
        utils_show_progress(progress, "Recovering...");
    }

    free(buffer);
    close(out_fd);

    if (status == RECOVERY_SUCCESS) {
        printf("\nFile recovered successfully: %s\n", output_path);
    } else {
        printf("\nFile partially recovered: %s (%llu of %llu bytes)\n", 
               output_path, (unsigned long long)total_recovered, 
               (unsigned long long)result->size);
    }

    return status;
}

int recovery_recover_batch(disk_handle_t* handle,
                          const scan_result_t* results,
                          int count,
                          const recovery_options_t* options) {
    if (!handle || !results || count <= 0 || !options) {
        return 0;
    }

    if (!options->output_dir) {
        fprintf(stderr, "Error: Output directory not specified\n");
        return 0;
    }

    // 创建输出目录
    if (utils_mkdir_recursive(options->output_dir) < 0) {
        fprintf(stderr, "Error: Cannot create output directory: %s\n", 
               options->output_dir);
        return 0;
    }

    printf("Batch recovery starting...\n");
    printf("Output directory: %s\n", options->output_dir);
    printf("Total files: %d\n", count);

    int success_count = 0;
    char output_path[1024];
    char base_name[256];

    for (int i = 0; i < count; i++) {
        const scan_result_t* result = &results[i];
        
        // 生成输出文件名
        const char* ext = signature_get_extension(result->type);
        snprintf(base_name, sizeof(base_name), "%s/recovered_%04d", 
                options->output_dir, i + 1);
        
        utils_generate_unique_filename(base_name, ext, 
                                       output_path, sizeof(output_path));

        // 检查是否覆盖已存在的文件
        if (!options->overwrite && utils_file_exists(output_path)) {
            printf("Skipping existing file: %s\n", output_path);
            continue;
        }

        // 恢复文件
        printf("\n[%d/%d] ", i + 1, count);
        recovery_status_t status = recovery_recover_file(handle, result, output_path);
        
        if (status == RECOVERY_SUCCESS) {
            success_count++;
            
            // 如果需要验证
            if (options->verify) {
                if (recovery_verify_file(output_path)) {
                    printf("Verification: OK\n");
                } else {
                    printf("Verification: FAILED (file may be corrupted)\n");
                }
            }
        }
    }

    printf("\n=== Batch Recovery Complete ===\n");
    printf("Total files: %d\n", count);
    printf("Successfully recovered: %d\n", success_count);
    printf("Failed: %d\n", count - success_count);

    return success_count;
}

const char* recovery_get_status_desc(recovery_status_t status) {
    switch (status) {
        case RECOVERY_SUCCESS:  return "Success";
        case RECOVERY_FAILED:   return "Failed";
        case RECOVERY_PARTIAL:  return "Partial";
        case RECOVERY_CANCELED: return "Canceled";
        default:                return "Unknown";
    }
}

int recovery_verify_file(const char* file_path) {
    if (!file_path || !utils_file_exists(file_path)) {
        return 0;
    }

    // 打开文件
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        return 0;
    }

    // 读取文件头
    uint8_t header[512];
    ssize_t bytes_read = read(fd, header, sizeof(header));
    close(fd);

    if (bytes_read < 16) {
        return 0;
    }

    // 识别文件类型
    file_type_t detected_type = signature_identify(header, bytes_read);
    
    // 如果能识别出有效的文件类型，认为文件是完整的
    if (detected_type != FILE_TYPE_UNKNOWN) {
        return 1;
    }

    // 对于文本文件，检查是否包含可读字符
    int text_chars = 0;
    for (int i = 0; i < bytes_read; i++) {
        if ((header[i] >= 32 && header[i] <= 126) || 
            header[i] == '\n' || header[i] == '\r' || header[i] == '\t') {
            text_chars++;
        }
    }

    // 如果80%以上是可读字符，认为是有效的文本文件
    if (text_chars * 100 / bytes_read >= 80) {
        return 1;
    }

    return 0;
}

