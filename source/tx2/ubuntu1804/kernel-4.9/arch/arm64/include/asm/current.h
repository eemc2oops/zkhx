#ifndef __ASM_CURRENT_H
#define __ASM_CURRENT_H

#include <linux/compiler.h>

#include <asm/sysreg.h>

#ifndef __ASSEMBLY__

struct task_struct;

/*
 * We don't use read_sysreg() as we want the compiler to cache the value where
 * possible.
 */
static __always_inline struct task_struct *get_current(void)
{
    /*
        启动阶段  　arch/arm64/kernel/head.S　 __primary_switched 里把 init_task 结构赋给 sp_el0
        系统运行阶段 arch/arm64/kernel/entry.S  　   kernel_entry 里，当从用户态进入内核态时，用 sp_el0 保存当前 task_struct .
    */
	unsigned long sp_el0;

	asm ("mrs %0, sp_el0" : "=r" (sp_el0));

	return (struct task_struct *)sp_el0;
}

#define current get_current()

#endif /* __ASSEMBLY__ */

#endif /* __ASM_CURRENT_H */

