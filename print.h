/*********************************************************
 * 文件名：print.h
 * 作  者：tangy
 * 日  期：2026-05-11
 * 功  能：打印机任务队列结构体、函数声明
 * 说  明：提供循环队列与打印任务相关定义
 *********************************************************/

#ifndef __PRINT_H
#define __PRINT_H

#define MAX_SIZE 30

// 打印任务的三种状态
typedef enum
{
    waiting,  // 0 等待
    printing, // 1 打印中
    finished  // 2 完成
} TaskStatus;

// 打印任务的信息
typedef struct
{
    int id;            // 任务ID
    char filename[50]; // 文件名
    TaskStatus status; // 三种状态
} PrintTask;

// 本打印机的所有打印任务
typedef struct
{
    // 存放一堆打印任务的数组
    PrintTask data[MAX_SIZE];
    int front; // 队头
    int rear;  // 队尾
    int count; // 当前任务个数
} Queue;

// 队列初始化
void initQueue(Queue *q);

// 判断队列是否为空
int isEmpty(Queue *q);

// 判断队列是否已满
int isFull(Queue *q);

// 任务入队：添加打印任务
bool enqueue(Queue *q, PrintTask task);

// 任务出队：移除队头任务
int dequeue(Queue *q);

// 展示队列中所有等待的任务
void showQueue(Queue *q);

// 执行打印队头任务
void doPrintTask(Queue *q);

#endif