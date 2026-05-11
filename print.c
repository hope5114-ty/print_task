/*********************************************************
 * 文件名：print.c
 * 作  者：tangy
 * 日  期：2026-05-11
 * 功  能：实现打印机核心功能函数的编写
 * 说  明：队列初始化、入队、出队、判空、判满、显示队列、执行打印
 *********************************************************/

#include <stdio.h>
#include "print.h"

//函数定义
// 队列初始化
void initQueue(Queue *q);

// 判断队列是否为空
int isEmpty(Queue *q);

// 判断队列是否已满
int isFull(Queue *q);

// 任务入队：添加打印任务
int enqueue(Queue *q, PrintTask task);

// 任务出队：移除队头任务
int dequeue(Queue *q);

// 展示队列中所有等待的任务
void showQueue(Queue *q);

// 执行打印队头任务
void doPrintTask(Queue *q);