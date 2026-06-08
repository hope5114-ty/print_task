//商用打印机服务器模块

#ifndef SERVER_H
#define SERVER_H

#include "common.h"

// 服务器接口声明
// 初始化打印服务器链表
void Server_Init(PrintServer *srv);
// 判断服务器队列是否为空
bool Server_IsEmpty(PrintServer *srv);
// 任务入队：内置权限校验，普通员工无法提交紧急任务
int Server_EnQueue(PrintServer *srv, const char *name, int pages, TaskPriority pri, UserRole role);
// 取出队首任务（用于下发到打印机）
bool Server_DeQueue(PrintServer *srv, PrintTask *outTask);

// 普通用户：按名称删除，无身份限制
int Server_DelByName(PrintServer *srv, const char *taskName, UserRole role);
// 管理员专用：按位置删除，必须传入管理员身份才允许执行
int Server_DelByPos(PrintServer *srv, int pos, UserRole role);

// 遍历展示服务器所有排队任务
void Server_Show(PrintServer *srv);
// 清空链表并释放所有动态内存，防止内存泄漏
void Server_Clear(PrintServer *srv);

// 优先级动态调整：排队超时的任务自动提升优先级，解决任务饥饿
void Server_PriorityDecay(PrintServer *srv);

// 服务器全局排序
void Server_Sort(PrintServer *srv);

#endif
