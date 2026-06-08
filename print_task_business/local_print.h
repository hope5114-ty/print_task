//商用打印机 打印模块

#ifndef PRINT_H
#define PRINT_H

#include "common.h"

// 打印机基础操作
// 初始化打印机所有队列与状态
void Printer_Init(ComPrinter *prt);
// 判断指定优先级队列是否已满
bool Printer_IsFull(ComPrinter *prt, TaskPriority pri);
// 判断指定优先级队列是否为空
bool Printer_IsEmpty(ComPrinter *prt, TaskPriority pri);
// 判断打印机所有队列是否全部为空（整机空闲）
bool Printer_AllEmpty(ComPrinter *prt);
// 任务存入对应优先级队列
int Printer_EnQueue(ComPrinter *prt, PrintTask *task);

// 优先级调度出队：按 紧急→业务→普通→批量 顺序取任务
bool Printer_DeQueue(ComPrinter *prt, PrintTask *outTask);

// 按优先级队列+位置撤销任务
int Printer_CancelByPos(ComPrinter *prt, TaskPriority pri, int pos, UserRole role);
// 按任务名称撤销任务
int Printer_CancelByName(ComPrinter *prt, const char *taskName, UserRole role);

// 打印状态控制
void Printer_Pause(ComPrinter *prt);       // 暂停打印
void Printer_Resume(ComPrinter *prt);      // 恢复打印
void Printer_StopCurrent(ComPrinter *prt); // 终止当前正在执行的任务

// 展示本机所有队列任务
void Printer_Show(ComPrinter *prt);
// 一键清空本机所有队列
void Printer_ClearAll(ComPrinter *prt);

// 抢占判断：仅检测【紧急任务队列】是否有任务（主流商用机规则）
bool Printer_EmergencyTaskExist(ComPrinter *prt);

// 任务下发：从服务器取任务，下发到打印机
int Server_Dispatch(PrintServer *srv, ComPrinter *prt);
// 模拟打印机工作主逻辑
void Printer_Work(ComPrinter *prt);

#endif
