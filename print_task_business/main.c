#include "print.h"
#include "server.h"

int main(void)
{
    PrintServer srv;
    ComPrinter prt;

    Server_Init(&srv);
    Printer_Init(&prt);

    printf("========================================\n");
    printf("  企业打印机任务队列系统 - 功能测试\n");
    printf("========================================\n\n");

    // ==================== 1. 服务器提交任务 ====================
    printf("▶ 测试1：服务器任务提交与权限校验\n");
    Server_EnQueue(&srv, "总经理合同", 20, PRI_EMERGENCY, USER_ADMIN);
    Server_EnQueue(&srv, "财务年报", 50, PRI_BUSINESS, USER_ADMIN);
    Server_EnQueue(&srv, "员工周报", 3, PRI_NORMAL, USER_NORMAL);
    Server_EnQueue(&srv, "宣传手册", 100, PRI_BATCH, USER_ADMIN);
    // 普通员工尝试提交紧急任务（应被拒绝）
    Server_EnQueue(&srv, "秘密文件", 10, PRI_EMERGENCY, USER_NORMAL);

    printf("\n当前服务器队列：\n");
    Server_Show(&srv);

    // ==================== 2. 下发任务到打印机 ====================
    printf("\n▶ 测试2：下发任务到打印机\n");
    Server_Dispatch(&srv, &prt);  // 下发紧急任务
    Server_Dispatch(&srv, &prt);  // 下发业务任务
    Server_Dispatch(&srv, &prt);  // 下发普通任务
    Server_Dispatch(&srv, &prt);  // 下发批量任务

    printf("\n下发后打印机各队列：\n");
    Printer_Show(&prt);

    // ==================== 3. 模拟打印并触发紧急抢占 ====================
    printf("\n▶ 测试3：紧急任务抢占与断点恢复\n");

    // 先让打印机开始打印一个普通任务（模拟正在打印中）
    // 因为 Printer_Work 一次调用就完成，为展示抢占，我们手动设置打印机状态为“正在打印”
    // 假设当前正在打印“员工周报”
    PrintTask currentTask;
    if (Printer_DeQueue(&prt, &currentTask)) {
        prt.isBusy = 1;
        prt.runTask.hasTask = true;
        prt.runTask.data = currentTask;
        printf("打印机开始打印任务：%s\n", currentTask.taskName);
    }

    // 此时向打印机紧急队列中添加一个紧急任务（模拟领导临时插队）
    PrintTask urgent;
    strcpy(urgent.taskName, "紧急通知");
    urgent.pageCount = 2;
    urgent.priority = PRI_EMERGENCY;
    urgent.submitRole = USER_ADMIN;
    urgent.createTime = time(NULL);
    Printer_EnQueue(&prt, &urgent);
    printf("紧急任务【紧急通知】已直接加入打印机紧急队列！\n");

    // 调用 Printer_Work，它检测到正在打印且紧急队列非空，触发抢占
    printf("\n--- Printer_Work 第1次调用：检测到紧急任务，应触发抢占 ---\n");
    // 调用Printer_Work模拟打印机工作
    Printer_Work(&prt);

    // 此时普通任务被中断，断点已保存
    printf("\n被中断后打印机状态：\n");
    printf("忙碌？%d  暂停？%d  断点任务：%s\n",
           prt.isBusy, prt.isPause,
           prt.suspendTask.hasTask ? prt.suspendTask.task.taskName : "无");

    // 再次调用 Printer_Work：它会先恢复被中断的任务并完成
    printf("\n--- Printer_Work 第2次调用：恢复被中断的【员工周报】并打印完成 ---\n");
    Printer_Work(&prt);

    // 此时紧急任务还未打印，继续调用 Printer_Work 取出紧急任务
    printf("\n--- Printer_Work 第3次调用：打印紧急任务【紧急通知】---\n");
    Printer_Work(&prt);

    // 剩下的任务正常按优先级打印
    printf("\n--- Printer_Work 第4次调用：打印剩余任务（业务/批量等）---\n");
    Printer_Work(&prt);
    Printer_Work(&prt);  // 可能队列已空

    // ==================== 4. 按名称/位置取消任务 ====================
    printf("\n▶ 测试4：任务撤销\n");
    // 重新添加一些任务
    Server_EnQueue(&srv, "旧报告", 30, PRI_NORMAL, USER_NORMAL);
    Server_EnQueue(&srv, "旧报表", 15, PRI_BUSINESS, USER_ADMIN);
    Server_Dispatch(&srv, &prt);
    Server_Dispatch(&srv, &prt);

    printf("当前打印机队列：\n");
    Printer_Show(&prt);

    // 普通用户删除自己的任务
    Printer_CancelByName(&prt, "旧报告", USER_NORMAL);
    // 普通用户无法删除他人任务
    Printer_CancelByName(&prt, "旧报表", USER_NORMAL);
    // 管理员按位置删除（删除位置1）
    Printer_CancelByPos(&prt, PRI_BUSINESS, 1, USER_ADMIN);

    printf("\n撤销后打印机队列：\n");
    Printer_Show(&prt);

    // // ==================== 5. 暂停/恢复/终止 ====================
    // printf("\n▶ 测试5：暂停、恢复、终止打印\n");
    // // 再下发一个任务并开始打印
    // Server_EnQueue(&srv, "长文档", 500, PRI_BATCH, USER_ADMIN);
    // Server_Dispatch(&srv, &prt);
    // Printer_Work(&prt);  // 开始打印“长文档”（瞬间完成，此处仅演示）
    // // 手动设置忙碌状态以测试暂停
    // prt.isBusy = 1;
    // prt.runTask.hasTask = true;
    // strcpy(prt.runTask.data.taskName, "长文档");
    // Printer_Pause(&prt);
    // Printer_Resume(&prt);
    // Printer_StopCurrent(&prt);

    // // ==================== 6. 清空与收尾 ====================
    // printf("\n▶ 测试6：一键清空\n");
    // Printer_ClearAll(&prt);
    // Server_Clear(&srv);

    // printf("\n========================================\n");
    // printf("  所有测试完成，系统运行正常！\n");
    // printf("========================================\n");

    return 0;
}
