/*********************************************************
 * 文件名：print.c
 * 作  者：tangy
 * 日  期：2026-05-11
 * 功  能：实现打印机核心功能函数的编写
 * 说  明：队列初始化、入队、出队、判空、判满、显示队列、执行打印
 *********************************************************/

#include <stdio.h>
#include <stdbool.h>
#include "print.h"

// 函数定义
//  队列初始化
void initQueue(Queue *q)
{
    q->front = 0;
    q->rear = 0;
    q->count = 0;
}

// 判断队列是否为空
int isEmpty(Queue *q)
{
    return q->count==0;
}

// 判断队列是否已满
int isFull(Queue *q)
{
    return q->count == MAX_SIZE;
}

// 任务入队：添加打印任务
bool enqueue(Queue *q, PrintTask task)
{
    // 先判断是否为满
    if (isFull(q))
        return false;
    q->data[q->rear] = task;
    q->rear = (q->rear + 1) % MAX_SIZE;
    q->count++;
    return true;
}

// 任务出队：移除队头任务
int dequeue(Queue *q)
{
    if (isEmpty(q))
        return -1; // 为空时不执行
    q->front = (q->front + 1) % MAX_SIZE;
    q->count--;

    return 0;
}

// 展示队列中所有等待的任务
void showQueue(Queue *q)
{
    // 判断队列是否为空
    if (isEmpty(q))
    {
        printf("队列为空\n");
        return;
    }

    int index=q->front;
    for (int i = 0; i < q->count; i++)
    {
        printf("任务id:%d",q->data[index].id);
        printf("文件名:%s",q->data[index].filename);
        switch (q->data[index].status)
        {
        case 0:
            printf("任务状态：waiting");
            break;
        case 1:
            printf("任务状态：printing");
            break;
        case 2:
            printf("任务状态：finished");
            break;
        }
        printf("\n");
    }
    printf("\n");
}

// 执行打印队头任务
void doPrintTask(Queue *q)
{
    // 判断队列是否为空
    if (isEmpty(q))
    {
        printf("队列为空,无打印任务。\n");
        return;
    }

    //打印对头id
    printf("id:%d",q->data[q->front].id);
    printf("文件名:%s",q->data[q->front].filename);
    switch (q->data[q->front].status)
        {
        case 0:
            printf("任务状态：waiting");
            break;
        case 1:
            printf("任务状态：printing");
            break;
        case 2:
            printf("任务状态：finished");
            break;
        }

    //打印完成，将任务移出队列
    dequeue(q);
}