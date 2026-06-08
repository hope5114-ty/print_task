//商用打印机 打印模块
#include "print.h"
#include "server.h"

// 打印机基础操作
// 初始化打印机所有队列与状态
void Printer_Init(ComPrinter *prt)
{
    // 遍历4组优先级队列，逐个初始化循环队列
    for (int i = 0; i < PRI_LEVEL_NUM; i++)
    {
        prt->priQueue[i].front = 0;
        prt->priQueue[i].rear = 0;
    }

    // 初始状态：空闲、未暂停
    prt->isBusy = 0;
    prt->isPause = 0;

    // 断点缓存初始化
    prt->suspendTask.hasTask = false;
}

// 判断指定优先级队列是否已满
bool Printer_IsFull(ComPrinter *prt, TaskPriority pri)
{
    return (prt->priQueue[pri].rear + 1) % LOCAL_MAX_TASK == prt->priQueue[pri].front;
}

// 判断指定优先级队列是否为空
bool Printer_IsEmpty(ComPrinter *prt, TaskPriority pri)
{
    return prt->priQueue[pri].rear == prt->priQueue[pri].front;
}

// 判断打印机所有队列是否全部为空（整机空闲）
bool Printer_AllEmpty(ComPrinter *prt)
{
    return Printer_IsEmpty(prt, PRI_BATCH) && Printer_IsEmpty(prt, PRI_NORMAL) && Printer_IsEmpty(prt, PRI_BUSINESS) && Printer_IsEmpty(prt, PRI_EMERGENCY);
}

// 任务存入对应优先级队列
int Printer_EnQueue(ComPrinter *prt, PrintTask *task)
{
    TaskPriority pri = task->priority;
    // 入队前先判断相应标志位队列是否有空位
    if (Printer_IsFull(prt, pri))
    {
        printf("入队失败！\n");
        return -1;
    }
    prt->priQueue[pri].taskQueue[prt->priQueue[pri].rear] = *task;
    prt->priQueue[pri].rear = (prt->priQueue[pri].rear + 1) % LOCAL_MAX_TASK;
    return 0;
}

// 优先级调度出队：按 紧急→业务→普通→批量 顺序取任务
bool Printer_DeQueue(ComPrinter *prt, PrintTask *outTask)
{
    if (Printer_AllEmpty(prt)) return false;

    int pri = -1;
    if (!Printer_IsEmpty(prt, PRI_EMERGENCY))      pri = PRI_EMERGENCY;
    else if (!Printer_IsEmpty(prt, PRI_BUSINESS))   pri = PRI_BUSINESS;
    else if (!Printer_IsEmpty(prt, PRI_NORMAL))     pri = PRI_NORMAL;
    else if (!Printer_IsEmpty(prt, PRI_BATCH))      pri = PRI_BATCH;

    if (pri == -1) return false;

    // 先取任务，再移动 front
    *outTask = prt->priQueue[pri].taskQueue[prt->priQueue[pri].front];
    prt->priQueue[pri].front = (prt->priQueue[pri].front + 1) % LOCAL_MAX_TASK;
    return true;
}

// 按优先级队列+位置撤销任务
int Printer_CancelByPos(ComPrinter *prt, TaskPriority pri, int pos, UserRole role)
{
    if (role == USER_NORMAL)
    {
        printf("您无权限执行此操作！\n");
        return -1;
    }

    if (Printer_IsEmpty(prt, pri))
    {
        printf("此位置无数据，撤销任务失败!\n");
        return -1;
    }

    // 计算队列当前总任务数
    int total = (prt->priQueue[pri].rear - prt->priQueue[pri].front + LOCAL_MAX_TASK) % LOCAL_MAX_TASK;
    if (pos < 1 || pos > total)
    {
        printf("位置 %d 超出范围！当前有效任务数：%d\n", pos, total);
        return -1;
    }

    LocalPriQueue *p = &prt->priQueue[pri];
    // 换算为数组真实下标 (pos从1转下标从0)
    int del_idx = (p->front + pos - 1) % LOCAL_MAX_TASK;
    total = (p->rear - p->front + LOCAL_MAX_TASK) % LOCAL_MAX_TASK;
    int cur = del_idx;
    for (int i = 0; i < total - pos; i++)
    {
        int nextIdx = (cur + 1) % LOCAL_MAX_TASK;
        p->taskQueue[cur] = p->taskQueue[nextIdx];
        cur = nextIdx;
    }

    // 队尾指针回退一格
    p->rear = (p->rear - 1 + LOCAL_MAX_TASK) % LOCAL_MAX_TASK;
    printf("成功撤销第 %d 个任务\n", pos);
    return 0;
}

// 按任务名称撤销任务
int Printer_CancelByName(ComPrinter *prt, const char *taskName, UserRole role)
{
    int hasDelete = 0; // 标记是否删除过任务
    // 优先判断正在打印的任务
    if (prt->isBusy == 1 && prt->runTask.hasTask)
    {
        if (strcmp(prt->runTask.data.taskName, taskName) == 0)
        {
            if (role == USER_NORMAL && prt->runTask.data.submitRole != USER_NORMAL)
            {
                printf("普通用户无权终止他人打印任务\n");
            }
            else
            {
                printf("目标任务正在打印，已强制终止打印！\n");
                Printer_StopCurrent(prt);
                hasDelete = 1;
            }
        }
    }

    // 遍历4个优先级队列：紧急、业务、普通、批量
    TaskPriority priList[] = {PRI_EMERGENCY, PRI_BUSINESS, PRI_NORMAL, PRI_BATCH};
    for (int i = 0; i < PRI_LEVEL_NUM; i++)
    {
        TaskPriority curPri = priList[i];
        LocalPriQueue *q = &prt->priQueue[curPri];

        // 当前队列为空，直接跳过
        if (Printer_IsEmpty(prt, curPri))
        {
            continue;
        }

        // 计算当前队列总任务数
        int total = (q->rear - q->front + LOCAL_MAX_TASK) % LOCAL_MAX_TASK;

        // 从队头开始逐个遍历任务
        for (int pos = 1; pos <= total; pos++)
        {
            // 算出当前遍历位置对应的数组下标
            int idx = (q->front + pos - 1) % LOCAL_MAX_TASK;

            // 1. 任务名称不匹配，跳过
            if (strcmp(q->taskQueue[idx].taskName, taskName) != 0)
            {
                continue;
            }

            // 2. 权限校验
            if (role == USER_NORMAL)
            {
                // 普通用户：只能删自己提交的任务
                if (q->taskQueue[idx].submitRole != USER_NORMAL)
                {
                    printf("普通用户无权删除他人任务！\n");
                    continue;
                }
            }

            // 3. 判断：该任务是否正在打印，正在打印则立即停止
            if (prt->isBusy == 1)
            {
                printf("目标任务正在打印，已强制终止打印！\n");
                prt->isBusy = 0;
            }

            // 4. 循环队列删除当前位置元素：后续元素依次前移
            total = (q->rear - q->front + LOCAL_MAX_TASK) % LOCAL_MAX_TASK;
            int cur = idx;
            for (int i = 0; i < total - pos; i++)
            {
                int nextIdx = (cur + 1) % LOCAL_MAX_TASK;
                q->taskQueue[cur] = q->taskQueue[nextIdx];
                cur = nextIdx;
            }

            // 队尾指针回退一格
            q->rear = (q->rear - 1 + LOCAL_MAX_TASK) % LOCAL_MAX_TASK;

            printf("成功撤销名称为【%s】的任务\n", taskName);
            hasDelete = 1;

            // 删除一个后，队列总数变少，跳出本轮内层循环，重新遍历当前队列
            break;
        }
    }

    // 最终判断返回结果
    if (hasDelete == 1)
    {
        return 0; // 有任务被删除，执行成功
    }
    else
    {
        printf("未找到名称为【%s】的任务，撤销失败\n", taskName);
        return -1; // 无匹配任务，失败
    }
}

void Printer_Pause(ComPrinter *prt) // 暂停打印
{
    if (prt->isBusy == 0)
    {
        printf("当前无正在打印的任务，无需暂停\n");
        return;
    }
    if (prt->isPause == 1)
    {
        printf("打印已处于暂停状态\n");
        return;
    }
    prt->isPause = 1;
    printf("打印已暂停\n");
}

void Printer_Resume(ComPrinter *prt) // 恢复打印
{
    // 无打印任务，无法恢复
    if (prt->isBusy == 0)
    {
        printf("当前无正在打印的任务，无法恢复\n");
        return;
    }
    // 未暂停，无需恢复
    if (prt->isPause == 0)
    {
        printf("打印未暂停，无需恢复\n");
        return;
    }
    prt->isPause = 0;
    printf("打印已恢复\n");
}

// 终止当前正在执行的任务
void Printer_StopCurrent(ComPrinter *prt) 
{
    if (prt->isBusy == 0)
    {
        printf("当前无正在打印的任务，无需停止\n");
        return;
    }

    // 正在打印，执行停止操作
    prt->isBusy = 0;
    prt->isPause = 0;
    printf("已成功终止当前打印任务\n");
}

// 展示本机所有队列任务
void Printer_Show(ComPrinter *prt)
{
    if (Printer_AllEmpty(prt))
    {
        printf("当前无打印任务！\n");
        return;
    }

    // 优先级队列 + 对应中文名称
    TaskPriority priList[] = {PRI_EMERGENCY, PRI_BUSINESS, PRI_NORMAL, PRI_BATCH};
    const char *priName[] = {"紧急队列", "业务队列", "普通队列", "批量队列"};

    for (int i = 0; i < PRI_LEVEL_NUM; i++)
    {
        TaskPriority curPri = priList[i];
        LocalPriQueue *q = &prt->priQueue[curPri];

        if (Printer_IsEmpty(prt, curPri))
        {
            continue;
        }

        printf("========== %s ==========\n", priName[i]);
        int total = (q->rear - q->front + LOCAL_MAX_TASK) % LOCAL_MAX_TASK;

        for (int pos = 1; pos <= total; pos++)
        {
            int idx = (q->front + pos - 1) % LOCAL_MAX_TASK;
            PrintTask *task = &q->taskQueue[idx];
            printf("第%d个任务 | 名称：%s | 页数：%d\n",
                   pos, task->taskName, task->pageCount);
        }
        printf("\n");
    }
}

// 一键清空本机所有队列
void Printer_ClearAll(ComPrinter *prt)
{
    // 先判断是否已经全部为空
    if (Printer_AllEmpty(prt))
    {
        printf("所有队列已为空，无需清空\n");
        return;
    }

    // 遍历4个优先级队列，逐个清空
    TaskPriority priList[] = {PRI_EMERGENCY, PRI_BUSINESS, PRI_NORMAL, PRI_BATCH};
    for (int i = 0; i < PRI_LEVEL_NUM; i++)
    {
        LocalPriQueue *q = &prt->priQueue[priList[i]];
        q->front = 0;
        q->rear = 0;
    }

    // 清空后终止打印、解除暂停，整机置为空闲
    prt->isBusy = 0;
    prt->isPause = 0;

    printf("已一键清空所有打印队列，打印停止\n");
}

// 抢占判断：仅检测紧急任务队列是否有任务（主流商用机规则）
bool Printer_EmergencyTaskExist(ComPrinter *prt)
{
    // 紧急队列为空 → 无抢占任务
    if (Printer_IsEmpty(prt, PRI_EMERGENCY))
    {
        return false;
    }
    // 紧急队列有任务 → 可抢占
    return true;
}

// 任务下发：从服务器取任务，下发到打印机
int Server_Dispatch(PrintServer *srv, ComPrinter *prt)
{
    // 服务器空，无法下发
    if (Server_IsEmpty(srv))
    {
        printf("服务器无任务，下发失败\n");
        return -1;
    }
    PrintTask task;
    // 服务器出队
    if (!Server_DeQueue(srv, &task))
    {
        return -1;
    }
    // 打印机对应队列入队
    if (Printer_EnQueue(prt, &task) != 0)
    {
        printf("打印机队列已满，任务下发失败\n");
        // 可拓展：任务回退到服务器
        return -1;
    }
    printf("任务【%s】成功下发至打印机\n", task.taskName);
    return 0;
}

// 模拟打印机工作主逻辑：支持紧急任务强抢占、中断+自动恢复断点任务
void Printer_Work(ComPrinter *prt)
{
    // 正在打印且未暂停，实时检测紧急任务，触发抢占
    if (prt->isBusy == 1 && prt->isPause == 0)
    {
        // 检测紧急队列是否有新任务，需要抢占
        if (Printer_EmergencyTaskExist(prt))
        {
            printf("\n===== 检测到紧急任务，触发抢占！中断当前打印 =====\n");

            // 暂停当前打印
            Printer_Pause(prt);

            // 把正在打印的任务 存入断点缓存 suspendTask
            prt->suspendTask.hasTask = true;
            prt->suspendTask.task = prt->runTask.data;

            printf("已保存被中断任务：%s，待紧急任务执行完毕后恢复\n",
                   prt->suspendTask.task.taskName);
        }
        // 无紧急任务，继续原有打印
        return;
    }

    // 打印机空闲 或 已被抢占暂停，开始调度任务
    if (prt->isBusy == 0 || prt->isPause == 1)
    {
        // 优先级最高：先恢复被中断保存的断点任务
        if (prt->suspendTask.hasTask == true)
        {
            // 取出断点任务
            PrintTask resumeTask = prt->suspendTask.task;

            // 恢复打印状态
            prt->isBusy = 1;
            prt->isPause = 0;
            prt->runTask.hasTask = true;
            prt->runTask.data = resumeTask;

            printf("===== 恢复被中断任务：%s 页数：%d =====\n",
                   resumeTask.taskName, resumeTask.pageCount);

            // 模拟打印执行
            prt->isBusy = 0;
            prt->runTask.hasTask = false;
 
            // 恢复完成，清空断点缓存
            prt->suspendTask.hasTask = false;
            printf("===== 被中断任务【%s】打印完成，恢复流程结束 =====\n\n", resumeTask.taskName);
            return;
        }

        // 无断点任务，按四级优先级正常取任务打印
        PrintTask curTask;
        if (Printer_DeQueue(prt, &curTask))
        {
            prt->isBusy = 1;
            prt->isPause = 0;
            prt->runTask.hasTask = true;
            prt->runTask.data = curTask;

            printf("===== 开始打印任务：%s 页数：%d =====\n",
                   curTask.taskName, curTask.pageCount);

            // 模拟打印耗时
            prt->isBusy = 0;
            prt->runTask.hasTask = false;
            printf("===== 任务【%s】打印完成 =====\n\n", curTask.taskName);
        }
        else
        {
            // 无任何任务，打印机空闲
            prt->isPause = 0;
            printf("打印机空闲，等待新任务...\n");
        }
    }
}
