/*********************************************************
 * 项目名称：嵌入式简易家用打印机任务队列系统
 * 功能描述：基于顺序循环队列实现无优先级打印任务调度
 *            遵循FIFO先进先出规则，模拟家用打印机工作逻辑
 * 实现结构：数组顺序循环队列
 * 队列容量：MAX_TASK = 8 (贴合真实家用打印机任务上限)
 * 作者：hope5114_ty
 * 开发时间：2026年05月28日
 * 版本：V1.0
 * 补充说明：纯裸机C语言实现，无动态内存分配，适配嵌入式MCU
 *********************************************************/

#ifndef PRINT_H
#define PRINT_H
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_TASK 8       // 打印机最大任务容量
#define TASK_NAME_LEN 32 // 任务名称最大长度

/* 单个打印任务结构体：存储任务基础信息 */
typedef struct
{
    char taskName[TASK_NAME_LEN]; // 文档/任务名称
    int pageCount;                // 打印页数
} PrintTask;

/* 打印机循环队列结构体 */
typedef struct
{
    PrintTask taskQueue[MAX_TASK]; // 任务数组（循环队列主体）
    int front;                     // 队头指针：取出任务位置
    int rear;                      // 队尾指针：存入任务位置
    int isBusy;                    // 打印机状态 0-空闲 1-忙碌
} Printer;

// 基本功能（函数声明）
// 队列初始化
void init(Printer *p);
// 判断队列是否已满
bool is_full(Printer *p);
// 判断队列是否为空
bool is_empty(Printer *p);
// 任务入队
int enQueue(Printer *p,const char *name, int pages);
// 任务出队
PrintTask* deQueue(Printer *p);
// 模拟打印机工作
void Printer_Work(Printer *p);
// 遍历查看任务
void show(Printer *p);
// 清空队列
void clear(Printer *p);

#endif
