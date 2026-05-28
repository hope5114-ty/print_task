/*********************************************************
 * 项目名称：嵌入式简易家用打印机任务队列系统
 * 功能描述：基于顺序循环队列实现无优先级打印任务调度
 *            遵循FIFO先进先出规则，模拟家用打印机工作逻辑
 * 实现结构：数组顺序循环队列
 * 作者：hope5114_ty
 * 开发时间：2026年05月28日
 * 版本：V1.0
 * 补充说明：纯裸机C语言实现，无动态内存分配，适配嵌入式MCU
 *********************************************************/

#include "print.h"

void init(Printer *p)
{
    p->front = 0;
    p->rear = 0;
    p->isBusy = 0;
    memset(p->taskQueue, 0, sizeof(p->taskQueue));
}

bool is_full(Printer *p)
{
    return (p->rear + 1) % MAX_TASK == p->front;
}

bool is_empty(Printer *p)
{
    return p->front == p->rear;
}

int enQueue(Printer *p, const char *name, int pages)
{
    if (is_full(p))
    {
        printf("打印机任务已满，无法添加任务！\n");
        return 0;
    }

    strncpy(p->taskQueue[p->rear].taskName, name, TASK_NAME_LEN - 1);
    p->taskQueue[p->rear].pageCount = pages;
    p->rear = (p->rear + 1) % MAX_TASK;
    printf("任务添加成功，任务名称:%s，页数:%d\n", name, pages);
    return 1;
}

PrintTask *deQueue(Printer *p)
{
    if (is_empty(p))
    {
        printf("无打印任务可执行！\n");
        return NULL;
    }
    PrintTask *outTask = &p->taskQueue[p->front];
    p->front = (p->front + 1) % MAX_TASK;
    return outTask;
}

void Printer_Work(Printer *p)
{
    if (is_empty(p))
    {
        printf("无打印任务可执行！\n");
        return;
    }

    p->isBusy = 1;
    printf("\n========== 打印机开始工作 ==========\n");

    PrintTask *task = NULL;
    while ((task = deQueue(p)) != NULL)
    {
        printf("正在打印：%s  共 %d 页\n", task->taskName, task->pageCount);
    }

    p->isBusy = 0;
    printf("========== 所有任务打印完成 ==========\n\n");
}

void show(Printer *p)
{
    if (is_empty(p))
    {
        printf("无打印任务可查看！\n");
        return;
    }

    printf("\n");
    printf("*********查看打印机当下所有任务*********\n");
    int index = p->front;
    while (index != p->rear)
    {
        printf("打印任务名称：%s  共 %d 页\n", p->taskQueue[index].taskName, p->taskQueue[index].pageCount);
        index = (index + 1) % MAX_TASK;
    }
    printf("***************************************\n");
}

void clear(Printer *p)
{
    p->front = 0;
    p->rear = 0;
    printf("已清空所有待打印任务!\n");
}
