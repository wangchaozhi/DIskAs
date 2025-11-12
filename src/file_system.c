#include "file_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// FAT32 引导扇区结构
typedef struct __attribute__((packed)) {
    uint8_t  jmp[3];
    char     oem[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t sectors_per_fat_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t sectors_per_fat_32;
    uint16_t flags;
    uint16_t version;
    uint32_t root_cluster;
    uint16_t fsinfo_sector;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    char     volume_label[11];
    char     fs_type[8];
} fat32_boot_sector_t;

// FAT 目录项结构
typedef struct __attribute__((packed)) {
    char     name[11];
    uint8_t  attr;
    uint8_t  reserved;
    uint8_t  create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t cluster_high;
    uint16_t modify_time;
    uint16_t modify_date;
    uint16_t cluster_low;
    uint32_t file_size;
} fat_dir_entry_t;

// NTFS 引导扇区结构
typedef struct __attribute__((packed)) {
    uint8_t  jmp[3];
    char     oem[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint8_t  reserved[7];
    uint8_t  media_type;
    uint8_t  unused1[2];
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint8_t  unused2[8];
    uint64_t total_sectors;
    uint64_t mft_cluster;
    uint64_t mft_mirror_cluster;
    int8_t   clusters_per_mft_record;
    uint8_t  unused3[3];
    int8_t   clusters_per_index_buffer;
    uint8_t  unused4[3];
    uint64_t volume_serial;
} ntfs_boot_sector_t;

fs_type_t fs_detect_type(disk_handle_t* handle) {
    if (!handle) {
        return FS_TYPE_UNKNOWN;
    }

    uint8_t buffer[512];
    if (disk_read_sectors(handle, 0, 1, buffer) < 0) {
        return FS_TYPE_UNKNOWN;
    }

    // 检查 FAT 文件系统
    fat32_boot_sector_t* fat = (fat32_boot_sector_t*)buffer;
    if (fat->bytes_per_sector == 512 && 
        fat->sectors_per_cluster > 0 && 
        fat->sectors_per_cluster <= 128) {
        
        // 检查 FAT 类型字符串
        if (memcmp(fat->fs_type, "FAT32   ", 8) == 0) {
            return FS_TYPE_FAT32;
        } else if (memcmp(fat->fs_type, "FAT16   ", 8) == 0) {
            return FS_TYPE_FAT16;
        } else if (memcmp(fat->fs_type, "FAT12   ", 8) == 0) {
            return FS_TYPE_FAT12;
        }
        
        // 通过簇数判断 FAT 类型
        uint32_t total_sectors = fat->total_sectors_16 ? 
                                fat->total_sectors_16 : fat->total_sectors_32;
        uint32_t fat_size = fat->sectors_per_fat_16 ? 
                           fat->sectors_per_fat_16 : fat->sectors_per_fat_32;
        uint32_t root_sectors = ((fat->root_entries * 32) + 511) / 512;
        uint32_t data_sectors = total_sectors - (fat->reserved_sectors + 
                                                 (fat->num_fats * fat_size) + 
                                                 root_sectors);
        uint32_t total_clusters = data_sectors / fat->sectors_per_cluster;
        
        if (total_clusters < 4085) {
            return FS_TYPE_FAT12;
        } else if (total_clusters < 65525) {
            return FS_TYPE_FAT16;
        } else {
            return FS_TYPE_FAT32;
        }
    }

    // 检查 NTFS 文件系统
    ntfs_boot_sector_t* ntfs = (ntfs_boot_sector_t*)buffer;
    if (memcmp(ntfs->oem, "NTFS    ", 8) == 0) {
        return FS_TYPE_NTFS;
    }

    // 检查 EXT 文件系统（超级块在偏移 1024 处）
    uint8_t ext_buffer[1024];
    if (disk_read(handle, 1024, ext_buffer, 1024) > 0) {
        uint16_t* magic = (uint16_t*)(ext_buffer + 56);
        if (*magic == 0xEF53) {
            // 读取版本信息判断 EXT2/3/4
            uint32_t* rev_level = (uint32_t*)(ext_buffer + 76);
            if (*rev_level == 0) {
                return FS_TYPE_EXT2;
            }
            uint32_t* feature_incompat = (uint32_t*)(ext_buffer + 96);
            if (*feature_incompat & 0x0040) {
                return FS_TYPE_EXT4;
            }
            return FS_TYPE_EXT3;
        }
    }

    return FS_TYPE_UNKNOWN;
}

int fs_parse_info(disk_handle_t* handle, fs_info_t* info) {
    if (!handle || !info) {
        return -1;
    }

    memset(info, 0, sizeof(fs_info_t));
    info->type = fs_detect_type(handle);

    if (info->type == FS_TYPE_UNKNOWN) {
        return -1;
    }

    uint8_t buffer[512];
    if (disk_read_sectors(handle, 0, 1, buffer) < 0) {
        return -1;
    }

    if (info->type == FS_TYPE_FAT32 || info->type == FS_TYPE_FAT16 || 
        info->type == FS_TYPE_FAT12) {
        fat32_boot_sector_t* fat = (fat32_boot_sector_t*)buffer;
        
        info->cluster_size = fat->bytes_per_sector * fat->sectors_per_cluster;
        info->total_size = (fat->total_sectors_16 ? 
                           fat->total_sectors_16 : fat->total_sectors_32) * 
                           fat->bytes_per_sector;
        
        uint32_t fat_size = fat->sectors_per_fat_16 ? 
                           fat->sectors_per_fat_16 : fat->sectors_per_fat_32;
        uint32_t root_sectors = ((fat->root_entries * 32) + 511) / 512;
        
        info->fat_offset = fat->reserved_sectors * fat->bytes_per_sector;
        info->data_offset = (fat->reserved_sectors + 
                            (fat->num_fats * fat_size) + 
                            root_sectors) * fat->bytes_per_sector;
        
        if (info->type == FS_TYPE_FAT32) {
            info->root_cluster = fat->root_cluster;
        } else {
            info->root_cluster = 0;
        }
        
        info->total_clusters = (info->total_size - info->data_offset) / 
                              info->cluster_size;
        
        // 提取卷标
        strncpy(info->label, fat->volume_label, 11);
        info->label[11] = '\0';
        
        // 去除尾部空格
        for (int i = 10; i >= 0; i--) {
            if (info->label[i] == ' ') {
                info->label[i] = '\0';
            } else {
                break;
            }
        }
    } else if (info->type == FS_TYPE_NTFS) {
        ntfs_boot_sector_t* ntfs = (ntfs_boot_sector_t*)buffer;
        
        info->cluster_size = ntfs->bytes_per_sector * ntfs->sectors_per_cluster;
        info->total_size = ntfs->total_sectors * ntfs->bytes_per_sector;
        info->total_clusters = ntfs->total_sectors / ntfs->sectors_per_cluster;
        info->root_cluster = ntfs->mft_cluster;
        
        strcpy(info->label, "NTFS Volume");
    }

    return 0;
}

int fs_scan_deleted_files(disk_handle_t* handle, const fs_info_t* info,
                         file_entry_t* entries, int max_entries) {
    if (!handle || !info || !entries || max_entries <= 0) {
        return 0;
    }

    int found_count = 0;

    // 目前仅实现 FAT 文件系统的扫描
    if (info->type != FS_TYPE_FAT32 && info->type != FS_TYPE_FAT16 && 
        info->type != FS_TYPE_FAT12) {
        printf("Warning: Deleted file scanning only supported for FAT file systems\n");
        return 0;
    }

    // 扫描根目录区域（对于 FAT12/16）或数据区（对于 FAT32）
    uint64_t scan_offset;
    uint32_t scan_size;
    
    if (info->type == FS_TYPE_FAT32) {
        scan_offset = info->data_offset + 
                     (info->root_cluster - 2) * info->cluster_size;
        scan_size = info->cluster_size;
    } else {
        scan_offset = info->data_offset - 32 * 512; // 根目录在数据区之前
        scan_size = 32 * 512;
    }

    uint8_t* buffer = (uint8_t*)malloc(scan_size);
    if (!buffer) {
        return 0;
    }

    if (disk_read(handle, scan_offset, buffer, scan_size) < 0) {
        free(buffer);
        return 0;
    }

    // 扫描目录项
    fat_dir_entry_t* dir_entries = (fat_dir_entry_t*)buffer;
    int num_entries = scan_size / sizeof(fat_dir_entry_t);

    for (int i = 0; i < num_entries && found_count < max_entries; i++) {
        fat_dir_entry_t* entry = &dir_entries[i];
        
        // 跳过空项
        if (entry->name[0] == 0x00) {
            break;
        }
        
        // 检查已删除的文件（第一个字节为 0xE5）
        if ((uint8_t)entry->name[0] == 0xE5 && entry->attr != 0x0F) {
            file_entry_t* fe = &entries[found_count];
            memset(fe, 0, sizeof(file_entry_t));
            
            // 恢复文件名的第一个字符为 '?'
            fe->name[0] = '?';
            int name_len = 1;
            for (int j = 1; j < 8 && entry->name[j] != ' '; j++) {
                fe->name[name_len++] = entry->name[j];
            }
            
            // 添加扩展名
            if (entry->name[8] != ' ') {
                fe->name[name_len++] = '.';
                for (int j = 8; j < 11 && entry->name[j] != ' '; j++) {
                    fe->name[name_len++] = entry->name[j];
                }
            }
            fe->name[name_len] = '\0';
            
            fe->size = entry->file_size;
            fe->cluster = ((uint64_t)entry->cluster_high << 16) | entry->cluster_low;
            fe->is_deleted = 1;
            fe->is_directory = (entry->attr & 0x10) != 0;
            fe->fs_type = info->type;
            
            found_count++;
        }
    }

    free(buffer);
    return found_count;
}

const char* fs_get_type_name(fs_type_t type) {
    switch (type) {
        case FS_TYPE_FAT12:  return "FAT12";
        case FS_TYPE_FAT16:  return "FAT16";
        case FS_TYPE_FAT32:  return "FAT32";
        case FS_TYPE_NTFS:   return "NTFS";
        case FS_TYPE_EXT2:   return "EXT2";
        case FS_TYPE_EXT3:   return "EXT3";
        case FS_TYPE_EXT4:   return "EXT4";
        default:             return "Unknown";
    }
}

