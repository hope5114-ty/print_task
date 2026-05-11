/*********************************************************
 * 文件名：main.c
 * 作  者：tangy
 * 日  期：2026-05-11
 * 功  能：显示菜单和交互界面
 * 说  明：响应用户操作，调用底层队列功能，实现完整打印机控制系统
 *********************************************************/

#include <stdio.h>
#include "print.h"

int main(int argc, char const *argv[])
{
    // 新建一个打印机(队列)变量
    Queue xiaomi;
    initQueue(&xiaomi);

    int option;
    while (1)
    {
        // 打印用户界面
        printf("******打印机任务队列******\n");
        printf("1.添加打印机任务\n");
        printf("2.查看打印机当下的所有任务\n");
        printf("3.执行队头打印\n");
        printf("4.退出程序\n");
        printf("***********************");
        printf("请选择操作选项：");
        scanf("%d", &option);

        switch (option)
        {
        // 添加打印机任务
        case 1:
            // 创建打印任务
            PrintTask task;
            printf("请输入任务编号:");
            scanf("%d", &task.id);
            printf("请输入文件名:");
            scanf("%s", task.filename);
            // 新进入的任务默认为waiting状态
            task.status = 0;
            if (enqueue(&xiaomi, task))
            {
                printf("打印任务添加成功！\n");
            }
            else
            {
                printf("打印任务添加失败，xioami打印机任务已满！\n");
            }
            break;
        // 遍历打印机中的所有任务
        case 2:
            showQueue(&xiaomi);
            break;
        // 执行队头的打印任务
        case 3:
            doPrintTask(&xiaomi);
            break;
        // 退出程序
        case 4:
            printf("已退出程序！\n");
            break;
        default:
            printf("没有此选项！\n");
            break;
        }
    }

    return 0;
}
