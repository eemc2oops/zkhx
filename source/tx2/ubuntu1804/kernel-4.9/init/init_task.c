#include <linux/init_task.h>
#include <linux/export.h>
#include <linux/mqueue.h>
#include <linux/sched.h>
#include <linux/sched/sysctl.h>
#include <linux/sched/rt.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>

#include <asm/pgtable.h>
#include <asm/uaccess.h>

static struct signal_struct init_signals = INIT_SIGNALS(init_signals);
static struct sighand_struct init_sighand = INIT_SIGHAND(init_sighand);

/* Initial task structure */
// arch/arm64/kernel/head.s 在内核启动时，打开mmu后，把sp_el0指针关联到这个结构上
struct task_struct init_task = INIT_TASK(init_task);
EXPORT_SYMBOL(init_task);

/*
 * Initial thread structure. Alignment of this is handled by a special
 * linker map entry.
 */
// __init_task_data 是宏定义 在 include/linux/init_task.h 
// #define __init_task_data __attribute__((__section__(".data..init_task")))
// arch/arm64/kernel/head.s 在内核启动时，打开mmu后，把sp指针关联到这个结构上，此处做为启动阶段内核的调用栈使用
// tx2 下 init_thread_union 结构体里，定义的时候是空，此处纯粹分配了一个结构的空间
union thread_union init_thread_union __init_task_data = {
#ifndef CONFIG_THREAD_INFO_IN_TASK   // tx2 定义了 CONFIG_THREAD_INFO_IN_TASK
	INIT_THREAD_INFO(init_task)
#endif
};
