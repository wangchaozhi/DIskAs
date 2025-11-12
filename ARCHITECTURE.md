# DiskAS 架构设计文档

## 项目概述

DiskAS 是一个模块化的C语言硬盘文件恢复软件，采用清晰的分层架构设计，易于维护和扩展。

## 架构设计

### 分层架构

```
┌─────────────────────────────────────┐
│        主程序 (main.c)              │
│    命令行解析、流程控制、用户界面     │
└─────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────┐
│         应用层                       │
│  ┌──────────────┐  ┌──────────────┐ │
│  │   Scanner    │  │   Recovery   │ │
│  │  磁盘扫描器   │  │  文件恢复器   │ │
│  └──────────────┘  └──────────────┘ │
└─────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────┐
│         业务层                       │
│  ┌──────────────┐  ┌──────────────┐ │
│  │ File System  │  │  Signature   │ │
│  │ 文件系统分析  │  │  文件签名识别 │ │
│  └──────────────┘  └──────────────┘ │
└─────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────┐
│         基础层                       │
│  ┌──────────────┐  ┌──────────────┐ │
│  │   Disk I/O   │  │    Utils     │ │
│  │  磁盘I/O操作  │  │   工具函数    │ │
│  └──────────────┘  └──────────────┘ │
└─────────────────────────────────────┘
```

## 模块详解

### 1. Disk I/O 模块 (disk_io.c/h)

**职责**: 提供底层磁盘访问接口

**主要功能**:
- 打开/关闭设备（块设备和磁盘镜像）
- 按字节偏移读取数据
- 按扇区读取数据
- 获取设备大小信息
- 跨平台支持（macOS/Linux）

**关键 API**:
```c
disk_handle_t* disk_open(const char* device_path);
void disk_close(disk_handle_t* handle);
ssize_t disk_read(disk_handle_t* handle, uint64_t offset, void* buffer, size_t size);
ssize_t disk_read_sectors(disk_handle_t* handle, uint64_t sector, uint32_t count, void* buffer);
```

**设计特点**:
- 统一的设备抽象（块设备和文件）
- 安全的边界检查
- 详细的错误报告

---

### 2. Signature 模块 (signature.c/h)

**职责**: 文件类型识别和签名匹配

**主要功能**:
- 维护文件签名数据库
- 基于魔数识别文件类型
- 支持20+种常见文件格式
- 可扩展的签名系统

**关键 API**:
```c
void signature_init(void);
file_type_t signature_identify(const uint8_t* data, size_t size);
const char* signature_get_extension(file_type_t type);
const char* signature_get_description(file_type_t type);
```

**支持的文件类型**:
- 图片: JPEG, PNG, GIF, BMP
- 文档: PDF, DOC/DOCX, XLS/XLSX, PPT/PPTX
- 压缩: ZIP, RAR, 7Z
- 音视频: MP3, MP4, AVI, MOV
- 其他: TXT, HTML, XML, EXE, DLL

**设计特点**:
- 数据驱动的签名匹配
- 可配置的偏移量和长度
- 文本文件启发式识别

---

### 3. File System 模块 (file_system.c/h)

**职责**: 文件系统分析和元数据提取

**主要功能**:
- 自动检测文件系统类型
- 解析文件系统结构
- 扫描已删除文件
- 提取文件元数据

**支持的文件系统**:
- FAT12/16/32
- NTFS
- EXT2/3/4

**关键 API**:
```c
fs_type_t fs_detect_type(disk_handle_t* handle);
int fs_parse_info(disk_handle_t* handle, fs_info_t* info);
int fs_scan_deleted_files(disk_handle_t* handle, const fs_info_t* info, 
                         file_entry_t* entries, int max_entries);
```

**设计特点**:
- 可扩展的文件系统支持
- 详细的文件系统信息
- 已删除文件的元数据恢复

---

### 4. Scanner 模块 (scanner.c/h)

**职责**: 磁盘扫描和文件发现

**主要功能**:
- 快速扫描（基于文件系统）
- 深度扫描（基于文件签名）
- 自动模式选择
- 进度报告

**扫描模式**:
1. **快速扫描**: 分析文件系统元数据，速度快
2. **深度扫描**: 全盘搜索文件签名，恢复率高
3. **自动模式**: 智能选择最优扫描策略

**关键 API**:
```c
int scanner_init(void);
int scanner_scan(disk_handle_t* handle, const scan_options_t* options,
                scan_result_t* results, int max_results);
int scanner_quick_scan(disk_handle_t* handle, scan_result_t* results, int max_results);
int scanner_deep_scan(disk_handle_t* handle, scan_result_t* results, int max_results);
```

**设计特点**:
- 灵活的扫描选项
- 回调机制支持
- 智能文件大小估算
- 避免重复扫描

---

### 5. Recovery 模块 (recovery.c/h)

**职责**: 文件恢复和完整性验证

**主要功能**:
- 单文件恢复
- 批量文件恢复
- 进度显示
- 完整性验证
- 唯一文件名生成

**关键 API**:
```c
recovery_status_t recovery_recover_file(disk_handle_t* handle, 
                                       const scan_result_t* result,
                                       const char* output_path);
int recovery_recover_batch(disk_handle_t* handle,
                          const scan_result_t* results,
                          int count,
                          const recovery_options_t* options);
int recovery_verify_file(const char* file_path);
```

**恢复状态**:
- SUCCESS: 完全成功
- FAILED: 恢复失败
- PARTIAL: 部分恢复
- CANCELED: 用户取消

**设计特点**:
- 大缓冲区提高性能
- 实时进度反馈
- 自动处理文件名冲突
- 可选的完整性验证

---

### 6. Utils 模块 (utils.c/h)

**职责**: 通用工具函数

**主要功能**:
- 文件大小格式化
- 目录递归创建
- 文件存在性检查
- 唯一文件名生成
- 进度百分比计算
- 进度条显示
- 十六进制转储

**关键 API**:
```c
char* utils_format_size(uint64_t bytes, char* buffer, size_t size);
int utils_mkdir_recursive(const char* path);
int utils_file_exists(const char* path);
char* utils_generate_unique_filename(const char* base_path, 
                                     const char* extension,
                                     char* buffer, 
                                     size_t size);
void utils_show_progress(int progress, const char* message);
void utils_hex_dump(const uint8_t* data, size_t size, uint64_t offset);
```

**设计特点**:
- 纯函数式设计
- 无全局状态
- 可复用性强

---

### 7. Main 主程序 (main.c)

**职责**: 命令行界面和流程控制

**主要功能**:
- 命令行参数解析
- 用户交互
- 工作流程编排
- 结果展示

**命令行选项**:
```
-h, --help      显示帮助
-v, --version   显示版本
-i, --info      显示设备信息
-m, --mode      扫描模式 (quick/deep/auto)
-o, --output    输出目录
-l, --list      仅列出文件
-r, --recover   自动恢复
-V, --verify    验证完整性
```

**工作流程**:
1. 解析命令行参数
2. 初始化模块
3. 打开设备
4. 显示设备信息（可选）
5. 执行扫描
6. 列出结果
7. 执行恢复（可选）
8. 清理资源

---

## 数据结构设计

### disk_handle_t - 磁盘句柄
```c
typedef struct {
    int fd;                    // 文件描述符
    uint64_t size;            // 磁盘大小
    uint32_t sector_size;     // 扇区大小
    char device_path[256];    // 设备路径
} disk_handle_t;
```

### file_signature_t - 文件签名
```c
typedef struct {
    file_type_t type;        // 文件类型
    const char* extension;   // 文件扩展名
    const uint8_t* magic;    // 魔数
    size_t magic_len;        // 魔数长度
    size_t offset;           // 魔数偏移
    const char* description; // 描述
} file_signature_t;
```

### fs_info_t - 文件系统信息
```c
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
```

### scan_result_t - 扫描结果
```c
typedef struct {
    uint64_t offset;          // 文件偏移
    uint64_t size;            // 文件大小
    file_type_t type;         // 文件类型
    uint8_t confidence;       // 置信度
} scan_result_t;
```

---

## 错误处理策略

### 分级错误处理
1. **致命错误**: 直接退出程序（如内存分配失败）
2. **操作失败**: 返回错误码，继续处理其他任务
3. **警告信息**: 记录日志，不影响主流程

### 错误码规范
- 成功: 返回 0 或非负数
- 失败: 返回 -1 或负数
- 指针函数: 返回 NULL 表示失败

---

## 性能优化

### 1. I/O 优化
- 大缓冲区读取（1MB）
- 减少系统调用次数
- 顺序读取优化

### 2. 内存管理
- 预分配大数组
- 避免频繁 malloc/free
- 栈上分配小对象

### 3. 算法优化
- 快速签名匹配
- 跳过已识别区域
- 智能扫描范围选择

---

## 可扩展性设计

### 1. 新文件类型支持
只需在 `signatures` 数组中添加新签名：
```c
{FILE_TYPE_NEW, "ext", (const uint8_t*)"\xAB\xCD", 2, 0, "New Type"}
```

### 2. 新文件系统支持
实现以下接口：
- 检测函数
- 解析函数
- 扫描函数

### 3. 新扫描模式
扩展 `scan_mode_t` 枚举和相应的处理逻辑

---

## 安全考虑

### 1. 只读访问
- 仅读取设备，不写入
- 防止意外修改原始数据

### 2. 边界检查
- 所有磁盘读取都有边界验证
- 防止缓冲区溢出

### 3. 权限控制
- 提示用户权限要求
- 建议使用磁盘镜像

---

## 测试策略

### 1. 单元测试
每个模块独立测试其功能

### 2. 集成测试
测试模块间的交互

### 3. 系统测试
端到端的恢复场景测试

### 4. 测试用例
- 不同文件系统
- 不同文件类型
- 不同损坏程度

---

## 构建系统

### Makefile
- 简单易用
- 支持 clean, install, uninstall
- 跨平台兼容

### CMake
- 现代构建系统
- IDE 集成友好
- 更好的依赖管理

---

## 未来改进方向

### 短期目标
- [ ] 增加更多文件类型支持
- [ ] 优化深度扫描性能
- [ ] 添加日志系统
- [ ] 单元测试覆盖

### 中期目标
- [ ] GUI 界面
- [ ] 文件预览功能
- [ ] 多线程扫描
- [ ] 更多文件系统支持

### 长期目标
- [ ] RAID 支持
- [ ] 分区表恢复
- [ ] 网络远程恢复
- [ ] 云存储集成

---

## 贡献指南

### 代码规范
- 使用 C11 标准
- 遵循一致的命名规范
- 添加必要的注释
- 保持模块独立性

### 提交流程
1. Fork 项目
2. 创建特性分支
3. 编写代码和测试
4. 提交 Pull Request

---

## 许可证

MIT License - 详见 LICENSE 文件

---

## 联系方式

项目主页: https://github.com/yourusername/DiskAS
问题反馈: Issues 页面

