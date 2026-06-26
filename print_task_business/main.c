/*********************************************************
 * main.c - 商用打印机队列系统基本测试
 * 功能：演示四级优先级调度 + 抢占功能
 *********************************************************/

#include "common.h"
#include "server.h"
#include "print.h"

static ComPrinter g_printer;
static PrintServer g_server;

// 显示菜单
void ShowMenu(void)
{
    printf("\n");
    printf("\n========== 商用打印机测试 ==========\n");
    printf("1. 提交任务\n");
    printf("2. 下发任务\n");
    printf("3. 执行打印\n");
    printf("4. 查看服务器\n");
    printf("5. 查看打印机\n");
    printf("6. 撤销任务\n");
    printf("0. 退出\n");
    printf("====================================\n");
    printf("选择: ");
}

// 初始化测试任务
void InitTestTasks(void)
{
    printf("\n初始化测试任务...\n");
    
    // 提交4个不同优先级的任务
    Server_EnQueue(&g_server, "紧急报告", 10, PRI_EMERGENCY, USER_ADMIN);
    Server_EnQueue(&g_server, "财务报表", 30, PRI_BUSINESS, USER_ADMIN);
    Server_EnQueue(&g_server, "普通文档", 5, PRI_NORMAL, USER_NORMAL);
    Server_EnQueue(&g_server, "批量打印", 100, PRI_BATCH, USER_NORMAL);
    
    printf("✅ 已提交4个测试任务 (优先级: 0,1,2,3)\n\n");
}

int main(void)
{
    // 初始化
    Server_Init(&g_server);
    Printer_Init(&g_printer);
    InitTestTasks();
    
    char choice;
    char name[32];
    int pages, pri, role;
    
    while (1)
    {
        ShowMenu();
        scanf(" %c", &choice);
        getchar();
        
        switch (choice)
        {
        case '1': // 提交任务
            printf("名称 页数 优先级(0-3) 身份(0普通/1管理员): ");
            scanf("%s %d %d %d", name, &pages, &pri, &role);
            Server_EnQueue(&g_server, name, pages, pri, role);
            break;
            
        case '2': // 下发任务到打印机
            Server_Dispatch(&g_server, &g_printer);
            break;
            
        case '3': // 执行打印（自动处理抢占）
            Printer_Work(&g_printer);
            break;
            
        case '4': // 查看服务器队列
            printf("\n服务器任务数: %d\n", g_server.taskCnt);
            Server_Show(&g_server);
            break;
            
        case '5': // 查看打印机队列
            printf("\n打印机状态: %s\n", g_printer.isBusy ? "忙碌" : "空闲");
            if (g_printer.suspendTask.hasTask)
                printf("断点任务: %s\n", g_printer.suspendTask.task.taskName);
            Printer_Show(&g_printer);
            break;
            
        case '6': // 撤销任务
            printf("输入任务名称: ");
            scanf("%s", name);
            Printer_CancelByName(&g_printer, name, USER_ADMIN);
            Server_DelByName(&g_server, name, USER_ADMIN);
            break;
            
        case '0':
            printf("退出系统\n");
            Server_Clear(&g_server);
            return 0;
            
        default:
            printf("无效选择\n");
        }
    }
    return 0;
}
