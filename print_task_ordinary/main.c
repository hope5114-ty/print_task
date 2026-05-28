/*********************************************************
 * 项目名称：嵌入式简易家用打印机任务队列系统
 * 功能描述：基于顺序循环队列实现无优先级打印任务调度
 *            遵循FIFO先进先出规则，模拟家用打印机工作逻辑
 * 实现结构：数组顺序循环队列
 * 队列容量：MAX_TASK = 8 (贴合真实家用打印机任务上限)
 * 作者：hope5114_ty
 * 开发时间：2026年05月28日
 * 版本：V1.0
 * 补充说明：纯裸机C语言实现，无动态内存分配，适配嵌入式MCU
 *********************************************************/

#include "print.h"

int main(int argc, char const *argv[])
{
    Printer myPrinter;
    init(&myPrinter);

    enQueue(&myPrinter,"xxx课程实验报告",34);
    enQueue(&myPrinter,"旅行照片",78);
    enQueue(&myPrinter,"毕业论文",234);
    show(&myPrinter);

    Printer_Work(&myPrinter);
    show(&myPrinter);
    return 0;
}
