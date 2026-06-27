# ============================================
# PrinterSim - 打印机任务调度模拟器 Makefile
# ============================================

# 编译器
CC = gcc

# 编译参数
# -Wall   : 显示所有警告
# -g      : 生成调试信息（GDB 调试需要）
# -std=c99: 使用 C99 标准
# -O2     : 优化等级（调试时可以改成 -O0）
CFLAGS = -Wall -g -std=c99

# 输出目录
BIN_DIR = bin

# ============================================
# 家用版（普通打印机）
# ============================================
ORDINARY_DIR = print_task_ordinary
ORDINARY_SRCS = $(ORDINARY_DIR)/main.c \
                $(ORDINARY_DIR)/print.c
ORDINARY_TARGET = $(BIN_DIR)/printer_ordinary

# ============================================
# 商用版（企业级打印机）
# ============================================
BUSINESS_DIR = print_task_business
BUSINESS_SRCS = $(BUSINESS_DIR)/main.c \
                $(BUSINESS_DIR)/server.c \
                $(BUSINESS_DIR)/local_print.c
BUSINESS_TARGET = $(BIN_DIR)/printer_business

# ============================================
# 目标
# ============================================

# 默认目标：编译全部
all: ordinary business
	@echo ""
	@echo "✅  编译完成！"
	@echo "   家用版: ./$(ORDINARY_TARGET)"
	@echo "   商用版: ./$(BUSINESS_TARGET)"

# 编译家用版
ordinary: $(ORDINARY_TARGET)

$(ORDINARY_TARGET): $(ORDINARY_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(ORDINARY_SRCS) -o $(ORDINARY_TARGET) -I$(ORDINARY_DIR)
	@echo "✅  家用版编译完成"

# 编译商用版
business: $(BUSINESS_TARGET)

$(BUSINESS_TARGET): $(BUSINESS_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(BUSINESS_SRCS) -o $(BUSINESS_TARGET) -I$(BUSINESS_DIR)
	@echo "✅  商用版编译完成"

# 清理编译产物
clean:
	rm -rf $(BIN_DIR)
	rm -f *.o
	rm -f $(ORDINARY_DIR)/*.o
	rm -f $(BUSINESS_DIR)/*.o
	@echo "🧹  清理完成"

# 调试版本（额外打开调试宏）
debug: CFLAGS += -DDEBUG -O0
debug: clean all
	@echo "🔍  调试版本编译完成"

# 声明伪目标（这些不是文件名）
.PHONY: all ordinary business clean debug
