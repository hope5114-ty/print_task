//商用打印机服务器模块

#include "server.h"


// 初始化打印服务器链表
void Server_Init(PrintServer *srv)
{
    srv->head = malloc(sizeof(Node));
    if (srv->head == NULL)
    {
        perror("内存分配失败");
        return;
    }
    srv->head->next = NULL;
    srv->tail = srv->head;
    srv->taskCnt = 0;
}

// 判断服务器队列是否为空
bool Server_IsEmpty(PrintServer *srv)
{
    return srv->tail == srv->head;
}

// 任务入队：内置权限校验，普通员工无法提交紧急任务
int Server_EnQueue(PrintServer *srv, const char *name, int pages, TaskPriority pri, UserRole role)
{
    // 当普通员工提交紧急任务时
    if (role == USER_NORMAL && pri == PRI_EMERGENCY)
    {
        printf("你的权限无法提交紧急任务！\n");
        return -1;
    }
    Node *new_task = malloc(sizeof(Node));
    if (new_task == NULL)
    {
        printf("内存分配失败\n");
        return -1;
    }
    new_task->data.createTime = time(NULL);
    new_task->data.pageCount = pages;
    new_task->data.priority = pri;
    new_task->data.submitRole = role;
    strncpy(new_task->data.taskName, name, TASK_NAME_LEN - 1);
    new_task->data.taskName[TASK_NAME_LEN - 1] = '\0';
    srv->tail->next = new_task;
    new_task->next = NULL;
    srv->tail = new_task;
    srv->taskCnt++;

    // 每一次入队后都调用排序函数对服务器进行排序
    Server_Sort(srv);
    return 0;
}

// 取出队首任务（用于下发到打印机）
bool Server_DeQueue(PrintServer *srv, PrintTask *outTask)
{
    if (Server_IsEmpty(srv))
    {
        return false;
    }

    Node *delNode = srv->head->next;
    *outTask = delNode->data;
    srv->head->next = delNode->next;
    if (srv->head->next == NULL)
    {
        srv->tail = srv->head;
    }

    free(delNode);
    srv->taskCnt--;
    return true;
}

// 普通用户：按名称删除
int Server_DelByName(PrintServer *srv, const char *taskName, UserRole role)
{
    if (Server_IsEmpty(srv))
    {
        printf("队列为空，无需撤销任务！\n");
        return -1;
    }

    Node *temp = srv->head;
    Node *del_task = NULL;
    int find = 0;
    while (temp->next != NULL)
    {
        if (!strcmp(temp->next->data.taskName, taskName))
        {
            if (role == USER_NORMAL)
            {
                if (temp->next->data.submitRole != USER_NORMAL)
                {
                    temp = temp->next;
                    continue;
                }
            }
            find = 1;
            del_task = temp->next;
            if (del_task->next == NULL)
            {
                srv->tail = temp;
            }
            temp->next = temp->next->next;
            srv->taskCnt--;
            free(del_task);
            printf("服务器端成功撤销任务\n");
            // 不break，继续遍历下一个同名任务
            continue;
        }
        temp = temp->next;
    }
    if (!find)
    {
        printf("没有找到该任务！\n");
        return -1;
    }

    del_task = temp->next;
    if (del_task->next == NULL)
    {
        srv->tail = temp;
    }
    temp->next = temp->next->next;
    srv->taskCnt--;
    free(del_task);
    printf("服务器端成功撤销任务\n");
    return 0;
}

// 管理员专用：按位置删除，必须传入管理员身份才允许执行
int Server_DelByPos(PrintServer *srv, int pos, UserRole role)
{
    if (role != USER_ADMIN)
    {
        printf("您无权进行此操作！\n");
        return -1;
    }

    if (Server_IsEmpty(srv))
    {
        printf("打印任务为空，无可删除的任务！\n");
        return -1;
    }

    if (pos < 1 || pos > srv->taskCnt)
    {
        printf("输入位置非法！\n");
        return -1;
    }

    int index = pos;
    Node *temp = srv->head;
    for (int i = 1; i < index; i++)
    {
        temp = temp->next;
    }
    Node *del_task = temp->next;
    if (del_task->next == NULL)
    {
        srv->tail = temp;
    }
    temp->next = temp->next->next;
    srv->taskCnt--;
    free(del_task);
    return 0;
}

// 遍历展示服务器所有排队任务
void Server_Show(PrintServer *srv)
{
    if (Server_IsEmpty(srv))
    {
        printf("服务器无正在排队项目！\n");
        return;
    }

    Node *temp = srv->head->next;
    while (temp != NULL)
    {
        printf("任务名称：%s\t打印页数：%d\n", temp->data.taskName, temp->data.pageCount);
        temp = temp->next;
    }
}

// 清空服务区所有排队等待的任务
void Server_Clear(PrintServer *srv)
{
    if (Server_IsEmpty(srv))
        return;

    Node *temp = srv->head->next;
    Node *del;
    while (temp != NULL)
    {
        del = temp;
        temp = temp->next;
        free(del);
        del = NULL;
    }
    srv->taskCnt = 0;
    srv->tail = srv->head;
}

// 优先级动态调整：排队超时的任务自动提升优先级，解决任务饥饿
void Server_PriorityDecay(PrintServer *srv)
{
    if (Server_IsEmpty(srv))
    {
        return;
    }

    // 获取当前系统时间戳（单位：秒）
    time_t now = time(NULL);
    Node *cur = srv->head->next;

    while (cur != NULL)
    {
        // 计算当前任务已经排队的秒数
        long wait_sec = now - cur->data.createTime;

        // 排队超时 并且 不是最高优先级(0)，则提升优先级
        if (wait_sec >= WAIT_TIME_LIMIT && cur->data.priority > PRI_EMERGENCY)
        {
            cur->data.priority -= 1;
        }

        cur = cur->next;
    }

    // 优先级改变，重新全局排序
    Server_Sort(srv);
}

// 服务器全局排序
void Server_Sort(PrintServer *srv)
{
    if (Server_IsEmpty(srv) || srv->head->next->next == NULL)
    {
        return;
    }

    Node *p, *q;
    PrintTask temp;
    for (p = srv->head->next; p != NULL; p = p->next)
    {
        for (q = p->next; q != NULL; q = q->next)
        {
            // 优先级更低，需要交换
            if (p->data.priority > q->data.priority)
            {
                temp = p->data;
                p->data = q->data;
                q->data = temp;
            }
            // 优先级相同，创建时间更晚，需要交换
            else if (p->data.priority == q->data.priority)
            {
                if (p->data.createTime > q->data.createTime)
                {
                    temp = p->data;
                    p->data = q->data;
                    q->data = temp;
                }
            }
        }
    }
}
