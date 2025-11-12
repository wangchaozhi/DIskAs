#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <stdint.h>
#include <stddef.h>

// 文件类型枚举
typedef enum {
    FILE_TYPE_UNKNOWN = 0,
    FILE_TYPE_JPEG,
    FILE_TYPE_PNG,
    FILE_TYPE_GIF,
    FILE_TYPE_BMP,
    FILE_TYPE_PDF,
    FILE_TYPE_ZIP,
    FILE_TYPE_RAR,
    FILE_TYPE_7Z,
    FILE_TYPE_DOC,
    FILE_TYPE_DOCX,
    FILE_TYPE_XLS,
    FILE_TYPE_XLSX,
    FILE_TYPE_PPT,
    FILE_TYPE_PPTX,
    FILE_TYPE_MP3,
    FILE_TYPE_MP4,
    FILE_TYPE_AVI,
    FILE_TYPE_MOV,
    FILE_TYPE_TXT,
    FILE_TYPE_HTML,
    FILE_TYPE_XML,
    FILE_TYPE_EXE,
    FILE_TYPE_DLL,
    FILE_TYPE_MAX
} file_type_t;

// 文件签名结构
typedef struct {
    file_type_t type;        // 文件类型
    const char* extension;   // 文件扩展名
    const uint8_t* magic;    // 魔数（文件头特征）
    size_t magic_len;        // 魔数长度
    size_t offset;           // 魔数在文件中的偏移
    const char* description; // 描述
} file_signature_t;

/**
 * 初始化文件签名数据库
 */
void signature_init(void);

/**
 * 识别数据的文件类型
 * @param data 数据缓冲区
 * @param size 数据大小
 * @return 文件类型
 */
file_type_t signature_identify(const uint8_t* data, size_t size);

/**
 * 获取文件类型的扩展名
 * @param type 文件类型
 * @return 扩展名字符串
 */
const char* signature_get_extension(file_type_t type);

/**
 * 获取文件类型的描述
 * @param type 文件类型
 * @return 描述字符串
 */
const char* signature_get_description(file_type_t type);

/**
 * 获取所有支持的文件签名
 * @param count 输出签名数量
 * @return 文件签名数组
 */
const file_signature_t* signature_get_all(size_t* count);

#endif // SIGNATURE_H

