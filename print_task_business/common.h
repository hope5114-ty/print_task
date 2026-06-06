/*********************************************************
 * 项目名称：嵌入式商用企业打印机任务队列系统
 * 功能描述：基于多优先级队列+双层队列架构实现打印任务调度
 *            采用4级优先级调度，同优先级遵循FIFO先进先出规则
 *            模拟企业级商用复合机工作逻辑，支持任务暂停、精准撤销
 * 实现结构：数组循环队列（分优先级独立队列 + 上层服务队列）
 * 队列容量：服务层队列无固定上限；本机硬件队列 MAX_COM_TASK = 64
 * 优先级等级：0-紧急管理任务(最高) 1-核心业务任务 2-普通员工任务(默认) 3-批量低优任务(最低)
 * 作者：hope5114_ty
 * 开发时间：2026年05月30日
 * 规则说明：不强制中断当前打印任务，作业完成后高优先级任务优先执行；
 *           支持指定位置/指定名称撤销排队任务，支持暂停、终止当前任务，保留剩余队列
 *********************************************************/

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define TASK_NAME_LEN 32    // 任务名称最大字符长度
#define LOCAL_MAX_TASK 64   // 打印机单条优先级队列最大任务数(硬件固定容量)
#define PRI_LEVEL_NUM 4     // 优先级总等级
#define WAIT_TIME_LIMIT 180 // 排队超时阈值(模拟节拍)，超时触发优先级衰减

// 用户身份枚举（权限管控）
typedef enum
{
    USER_NORMAL, // 普通员工：禁止提交最高紧急优先级任务
    USER_ADMIN   // 管理员：拥有全部权限，可提交所有优先级任务
} UserRole;

// 打印优先级枚举
// 优先级顺序：紧急(0) > 核心业务(1) > 普通(2) > 批量(3)
typedef enum
{
    PRI_EMERGENCY = 0, // 紧急任务（最高优先级，唯一可抢占的等级） meergency
    PRI_BUSINESS,      // 核心业务任务 business
    PRI_NORMAL,        // 普通日常任务 normal
    PRI_BATCH          // 批量打印任务（最低优先级） batch
} TaskPriority;

// 单个打印任务结构体
typedef struct
{
    char taskName[TASK_NAME_LEN]; // 任务名称
    int pageCount;                // 打印总页数
    TaskPriority priority;        // 当前任务优先级
    UserRole submitRole;          // 任务提交者身份
    time_t createTime;            // 任务创建时间，用于优先级动态衰减
} PrintTask;

// 断点任务缓存（实现抢占+断点续打）
typedef struct
{
    bool hasTask;   // 标记：是否存在被中断的任务
    PrintTask task; // 保存被中断任务的完整信息
    int curPage;    // 记录断点：当前已打印页码
} SuspendTask;

// 第一部分：上层 打印服务器（单向链表）
// 职责：接收任务、缓存排队、优先级衰减、任务下发

// 链表节点结构
typedef struct Node
{
    PrintTask data;
    struct Node *next;
} Node;

// 打印服务器管理结构体
typedef struct
{
    Node *head;  // 链表头结点
    Node *tail;  // 链表尾指针，优化尾插效率
    int taskCnt; // 当前链表内任务总数
} PrintServer;

// 单条优先级循环队列
typedef struct
{
    PrintTask taskQueue[LOCAL_MAX_TASK];
    int front; // 队头指针
    int rear;  // 队尾指针
} LocalPriQueue;

typedef struct
{
    bool hasTask;
    PrintTask data;
} RunTask;

// 商用打印机整机结构体
typedef struct
{
    LocalPriQueue priQueue[PRI_LEVEL_NUM]; // 4组独立优先级队列
    int isBusy;                            // 忙碌标记：0-空闲  1-忙碌
    int isPause;                           // 暂停标记：0-正常  1-暂停
    SuspendTask suspendTask;               // 断点缓存：存放被紧急任务中断的任务
    RunTask runTask;  // 当前正在打印的任务缓存
} ComPrinter;

#endif
