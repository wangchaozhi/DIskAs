# Makefile for DiskAS

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude -O2
LDFLAGS = 

# 目录
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# 源文件
SOURCES = $(SRC_DIR)/disk_io.c \
          $(SRC_DIR)/signature.c \
          $(SRC_DIR)/file_system.c \
          $(SRC_DIR)/scanner.c \
          $(SRC_DIR)/recovery.c \
          $(SRC_DIR)/utils.c \
          main.c

# 目标文件
OBJECTS = $(SOURCES:.c=.o)

# 可执行文件
TARGET = DiskAS

# 默认目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful!"

# 编译
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	@echo "Cleaning..."
	rm -f $(OBJECTS) $(TARGET)
	rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

# 安装
install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin..."
	install -m 755 $(TARGET) /usr/local/bin/
	@echo "Installation complete!"

# 卸载
uninstall:
	@echo "Uninstalling $(TARGET)..."
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstallation complete!"

# 运行
run: $(TARGET)
	./$(TARGET)

# 帮助
help:
	@echo "Available targets:"
	@echo "  all        - Build the project (default)"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install to /usr/local/bin"
	@echo "  uninstall  - Remove from /usr/local/bin"
	@echo "  run        - Build and run the program"
	@echo "  help       - Show this help message"

.PHONY: all clean install uninstall run help

