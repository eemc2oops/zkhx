#ifndef _LINUX_STACKPROTECTOR_H
#define _LINUX_STACKPROTECTOR_H 1

#include <linux/compiler.h>
#include <linux/sched.h>
#include <linux/random.h>

#ifdef CONFIG_CC_STACKPROTECTOR  // tx2 没有定义 CONFIG_CC_STACKPROTECTOR
# include <asm/stackprotector.h>
#else
// start_kernel -> boot_init_stack_canary
static inline void boot_init_stack_canary(void)
{
}
#endif

#endif
