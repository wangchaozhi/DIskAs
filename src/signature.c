#include "signature.h"
#include <string.h>
#include <stdio.h>

// 文件签名数据库
static file_signature_t signatures[] = {
    // 图片文件
    {FILE_TYPE_JPEG, "jpg", (const uint8_t*)"\xFF\xD8\xFF", 3, 0, "JPEG Image"},
    {FILE_TYPE_PNG, "png", (const uint8_t*)"\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8, 0, "PNG Image"},
    {FILE_TYPE_GIF, "gif", (const uint8_t*)"GIF89a", 6, 0, "GIF Image (89a)"},
    {FILE_TYPE_GIF, "gif", (const uint8_t*)"GIF87a", 6, 0, "GIF Image (87a)"},
    {FILE_TYPE_BMP, "bmp", (const uint8_t*)"BM", 2, 0, "BMP Image"},
    
    // 文档文件
    {FILE_TYPE_PDF, "pdf", (const uint8_t*)"%PDF-", 5, 0, "PDF Document"},
    {FILE_TYPE_DOC, "doc", (const uint8_t*)"\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1", 8, 0, "MS Word Document"},
    {FILE_TYPE_DOCX, "docx", (const uint8_t*)"\x50\x4B\x03\x04", 4, 0, "MS Word Document (DOCX)"},
    {FILE_TYPE_XLS, "xls", (const uint8_t*)"\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1", 8, 0, "MS Excel Spreadsheet"},
    {FILE_TYPE_XLSX, "xlsx", (const uint8_t*)"\x50\x4B\x03\x04", 4, 0, "MS Excel Spreadsheet (XLSX)"},
    {FILE_TYPE_PPT, "ppt", (const uint8_t*)"\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1", 8, 0, "MS PowerPoint Presentation"},
    {FILE_TYPE_PPTX, "pptx", (const uint8_t*)"\x50\x4B\x03\x04", 4, 0, "MS PowerPoint Presentation (PPTX)"},
    
    // 压缩文件
    {FILE_TYPE_ZIP, "zip", (const uint8_t*)"\x50\x4B\x03\x04", 4, 0, "ZIP Archive"},
    {FILE_TYPE_ZIP, "zip", (const uint8_t*)"\x50\x4B\x05\x06", 4, 0, "ZIP Archive (empty)"},
    {FILE_TYPE_RAR, "rar", (const uint8_t*)"Rar!\x1A\x07\x00", 7, 0, "RAR Archive (v1.5+)"},
    {FILE_TYPE_RAR, "rar", (const uint8_t*)"Rar!\x1A\x07\x01\x00", 8, 0, "RAR Archive (v5.0+)"},
    {FILE_TYPE_7Z, "7z", (const uint8_t*)"7z\xBC\xAF\x27\x1C", 6, 0, "7-Zip Archive"},
    
    // 音视频文件
    {FILE_TYPE_MP3, "mp3", (const uint8_t*)"\xFF\xFB", 2, 0, "MP3 Audio (MPEG-1 Layer 3)"},
    {FILE_TYPE_MP3, "mp3", (const uint8_t*)"\x49\x44\x33", 3, 0, "MP3 Audio (ID3v2)"},
    {FILE_TYPE_MP4, "mp4", (const uint8_t*)"ftyp", 4, 4, "MP4 Video"},
    {FILE_TYPE_AVI, "avi", (const uint8_t*)"RIFF", 4, 0, "AVI Video"},
    {FILE_TYPE_MOV, "mov", (const uint8_t*)"moov", 4, 4, "QuickTime Movie"},
    
    // 文本文件
    {FILE_TYPE_HTML, "html", (const uint8_t*)"<!DOCTYPE html", 14, 0, "HTML Document"},
    {FILE_TYPE_HTML, "html", (const uint8_t*)"<html", 5, 0, "HTML Document"},
    {FILE_TYPE_XML, "xml", (const uint8_t*)"<?xml", 5, 0, "XML Document"},
    
    // 可执行文件
    {FILE_TYPE_EXE, "exe", (const uint8_t*)"MZ", 2, 0, "Windows Executable"},
    {FILE_TYPE_DLL, "dll", (const uint8_t*)"MZ", 2, 0, "Windows DLL"},
};

static const size_t signature_count = sizeof(signatures) / sizeof(signatures[0]);

void signature_init(void) {
    printf("Initialized signature database with %zu file signatures\n", signature_count);
}

file_type_t signature_identify(const uint8_t* data, size_t size) {
    if (!data || size == 0) {
        return FILE_TYPE_UNKNOWN;
    }

    // 遍历所有签名进行匹配
    for (size_t i = 0; i < signature_count; i++) {
        const file_signature_t* sig = &signatures[i];
        
        // 检查数据是否足够长
        if (size < sig->offset + sig->magic_len) {
            continue;
        }

        // 比较魔数
        if (memcmp(data + sig->offset, sig->magic, sig->magic_len) == 0) {
            return sig->type;
        }
    }

    // 尝试检测纯文本文件
    int is_text = 1;
    size_t check_len = size < 1024 ? size : 1024;
    for (size_t i = 0; i < check_len; i++) {
        uint8_t c = data[i];
        // 检查是否为可打印字符或常见空白字符
        if (c != '\n' && c != '\r' && c != '\t' && 
            (c < 0x20 || c > 0x7E) && c < 0x80) {
            is_text = 0;
            break;
        }
    }

    if (is_text) {
        return FILE_TYPE_TXT;
    }

    return FILE_TYPE_UNKNOWN;
}

const char* signature_get_extension(file_type_t type) {
    for (size_t i = 0; i < signature_count; i++) {
        if (signatures[i].type == type) {
            return signatures[i].extension;
        }
    }
    
    if (type == FILE_TYPE_TXT) {
        return "txt";
    }
    
    return "bin";
}

const char* signature_get_description(file_type_t type) {
    for (size_t i = 0; i < signature_count; i++) {
        if (signatures[i].type == type) {
            return signatures[i].description;
        }
    }
    
    if (type == FILE_TYPE_TXT) {
        return "Text File";
    }
    
    return "Unknown File";
}

const file_signature_t* signature_get_all(size_t* count) {
    if (count) {
        *count = signature_count;
    }
    return signatures;
}

