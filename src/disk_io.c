#include "disk_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef __APPLE__
#include <sys/disk.h>
#elif defined(__linux__)
#include <linux/fs.h>
#include <sys/ioctl.h>
#endif

disk_handle_t* disk_open(const char* device_path) {
    if (!device_path) {
        fprintf(stderr, "Error: Invalid device path\n");
        return NULL;
    }

    disk_handle_t* handle = (disk_handle_t*)calloc(1, sizeof(disk_handle_t));
    if (!handle) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    // 复制设备路径
    strncpy(handle->device_path, device_path, sizeof(handle->device_path) - 1);

    // 打开设备（只读模式）
    handle->fd = open(device_path, O_RDONLY);
    if (handle->fd < 0) {
        fprintf(stderr, "Error: Cannot open device '%s': %s\n", 
                device_path, strerror(errno));
        free(handle);
        return NULL;
    }

    // 获取设备大小
    struct stat st;
    if (fstat(handle->fd, &st) < 0) {
        fprintf(stderr, "Error: Cannot stat device: %s\n", strerror(errno));
        close(handle->fd);
        free(handle);
        return NULL;
    }

    if (S_ISREG(st.st_mode)) {
        // 普通文件（磁盘镜像）
        handle->size = st.st_size;
    } else if (S_ISBLK(st.st_mode)) {
        // 块设备
#ifdef __APPLE__
        uint32_t block_size;
        uint64_t block_count;
        if (ioctl(handle->fd, DKIOCGETBLOCKSIZE, &block_size) < 0 ||
            ioctl(handle->fd, DKIOCGETBLOCKCOUNT, &block_count) < 0) {
            fprintf(stderr, "Error: Cannot get device size: %s\n", strerror(errno));
            close(handle->fd);
            free(handle);
            return NULL;
        }
        handle->size = (uint64_t)block_size * block_count;
#elif defined(__linux__)
        uint64_t size;
        if (ioctl(handle->fd, BLKGETSIZE64, &size) < 0) {
            fprintf(stderr, "Error: Cannot get device size: %s\n", strerror(errno));
            close(handle->fd);
            free(handle);
            return NULL;
        }
        handle->size = size;
#else
        fprintf(stderr, "Error: Block device not supported on this platform\n");
        close(handle->fd);
        free(handle);
        return NULL;
#endif
    } else {
        fprintf(stderr, "Error: Invalid device type\n");
        close(handle->fd);
        free(handle);
        return NULL;
    }

    // 设置默认扇区大小
    handle->sector_size = 512;

    printf("Opened device: %s (Size: %llu bytes, Sector size: %u bytes)\n",
           device_path, (unsigned long long)handle->size, handle->sector_size);

    return handle;
}

void disk_close(disk_handle_t* handle) {
    if (!handle) {
        return;
    }

    if (handle->fd >= 0) {
        close(handle->fd);
    }

    free(handle);
}

ssize_t disk_read(disk_handle_t* handle, uint64_t offset, void* buffer, size_t size) {
    if (!handle || !buffer || handle->fd < 0) {
        return -1;
    }

    if (offset >= handle->size) {
        fprintf(stderr, "Error: Read offset beyond device size\n");
        return -1;
    }

    // 调整读取大小，防止超出设备边界
    if (offset + size > handle->size) {
        size = handle->size - offset;
    }

    // 定位到指定偏移
    if (lseek(handle->fd, offset, SEEK_SET) < 0) {
        fprintf(stderr, "Error: Cannot seek to offset %llu: %s\n",
                (unsigned long long)offset, strerror(errno));
        return -1;
    }

    // 读取数据
    ssize_t bytes_read = read(handle->fd, buffer, size);
    if (bytes_read < 0) {
        fprintf(stderr, "Error: Read failed: %s\n", strerror(errno));
        return -1;
    }

    return bytes_read;
}

ssize_t disk_read_sectors(disk_handle_t* handle, uint64_t sector, 
                         uint32_t count, void* buffer) {
    if (!handle || !buffer) {
        return -1;
    }

    uint64_t offset = sector * handle->sector_size;
    size_t size = count * handle->sector_size;

    return disk_read(handle, offset, buffer, size);
}

uint64_t disk_get_size(disk_handle_t* handle) {
    if (!handle) {
        return 0;
    }
    return handle->size;
}

