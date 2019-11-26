/*
 * Based on arch/arm/include/asm/tlbflush.h
 *
 * Copyright (C) 1999-2003 Russell King
 * Copyright (C) 2012 ARM Ltd.
 * Copyright (c) 2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ASM_TLBFLUSH_H
#define __ASM_TLBFLUSH_H

#ifndef __ASSEMBLY__

#include <linux/sched.h>
#include <asm/cputype.h>
#include <asm/mmu.h>

/*
 * Raw TLBI operations.
 *
 * Where necessary, use the __tlbi() macro to avoid asm()
 * boilerplate. Drivers and most kernel code should use the TLB
 * management routines in preference to the macro below.
 *
 * The macro can be used as __tlbi(op) or __tlbi(op, arg), depending
 * on whether a particular TLBI operation takes an argument or
 * not. The macros handles invoking the asm with or without the
 * register argument as appropriate.
 */
#define __TLBI_0(op, arg)		asm ("tlbi " #op)
#define __TLBI_1(op, arg)		asm ("tlbi " #op ", %0" : : "r" (arg))
#define __TLBI_N(op, arg, n, ...)	__TLBI_##n(op, arg)

#define __tlbi(op, ...)		__TLBI_N(op, ##__VA_ARGS__, 1, 0)

#define __tlbi_user(op, arg) do {						\
	if (arm64_kernel_unmapped_at_el0())					\
		__tlbi(op, (arg) | USER_ASID_FLAG);				\
} while (0)

/*
 *	TLB Management
 *	==============
 *
 *	The TLB specific code is expected to perform whatever tests it needs
 *	to determine if it should invalidate the TLB for each call.  Start
 *	addresses are inclusive and end addresses are exclusive; it is safe to
 *	round these addresses down.
 *
 *	flush_tlb_all()
 *
 *		Invalidate the entire TLB.
 *
 *	flush_tlb_mm(mm)
 *
 *		Invalidate all TLB entries in a particular address space.
 *		- mm	- mm_struct describing address space
 *
 *	flush_tlb_range(mm,start,end)
 *
 *		Invalidate a range of TLB entries in the specified address
 *		space.
 *		- mm	- mm_struct describing address space
 *		- start - start address (may not be aligned)
 *		- end	- end address (exclusive, may not be aligned)
 *
 *	flush_tlb_page(vaddr,vma)
 *
 *		Invalidate the specified page in the specified address range.
 *		- vaddr - virtual address (may not be aligned)
 *		- vma	- vma_struct describing address range
 *
 *	flush_kern_tlb_page(kaddr)
 *
 *		Invalidate the TLB entry for the specified page.  The address
 *		will be in the kernels virtual memory space.  Current uses
 *		only require the D-TLB to be invalidated.
 *		- kaddr - Kernel virtual memory address
 */

/*
 * Non shared TLB invalidation is only valid here if there are no other PEs in
 * the sharability domain
 */
#ifdef CONFIG_ARM64_NON_SHARED_TLBI
#define __DSB_FOR_TLBI(...)		dsb(nsh##__VA_ARGS__)
#define __TLBI(TYPE, ...)		__tlbi(TYPE, ##__VA_ARGS__)
#define __TLBI_USER(TYPE, ...)		__tlbi_user(TYPE, ##__VA_ARGS__)
#else
#define __DSB_FOR_TLBI(...)		dsb(ish##__VA_ARGS__)
#define __TLBI(TYPE, ...)		__tlbi(TYPE##is, ##__VA_ARGS__)
#define __TLBI_USER(TYPE, ...)		__tlbi_user(TYPE##is, ##__VA_ARGS__)
#endif

static inline void local_flush_tlb_all(void)
{
	dsb(nshst);
	__tlbi(vmalle1);
	dsb(nsh);
	isb();
}

/*
TLB的全称是Translation Lookaside Buffer，
我们知道，处理器在取指或者执行访问memory指令的时候都需要进行地址翻译，即把虚拟地址翻译成物理地址。
而地址翻译是一个漫长的过程，需要遍历几个level的Translation table，从而产生严重的开销。
为了提高性能，我们会在MMU中增加一个TLB的单元，把地址翻译关系保存在这个高速缓存中，从而省略了对内存中页表的访问。

当需要转换VA到PA的时候，首先在TLB中找是否有匹配的条目，如果有，那么我们称之TLB hit，这时候不需要再去访问页表来完成地址翻译。
不过TLB始终是全部页表的一个子集，因此也有可能在TLB中找不到。
如果没有在TLB中找到对应的item，那么称之TLB miss，那么就需要去访问memory中的page table来完成地址翻译，同时将翻译结果放入TLB，
如果TLB已经满了，那么还要设计替换算法来决定让哪一个TLB entry失效，从而加载新的页表项

这个接口用来invalidate TLB cache中的所有的条目。
执行完毕了该接口之后，由于TLB cache中没有缓存任何的VA到PA的转换信息，
因此，调用该接口API之前的所有的对page table的修改都可以被CPU感知到。注：该接口是大杀器，不要随便使用。
对于ARM64，flush_tlb_all接口使用的底层命令是：tlbi vmalle1is。
Tlbi是TLB Invalidate指令，
vmalle1is是参数，指明要invalidate那些TLB。
  vm表示本次invalidate操作对象是当前VMID，
  all表示要invalidate所有的TLB entry，
  e1是表示要flush的TLB entry的memory space identifier是EL0和EL1，regime stage 1的TLB entry。
  is是inner shareable的意思，表示要invalidate所有inner shareable内的所有PEs的TLB。
  如果没有is，则表示要flush的是local TLB，其他processor core的TLB则不受影响。
flush_tlb_all接口有一个变种：local_flush_tlb_all。
flush_tlb_all是invalidate系统中所有的TLB（各个PEs上的TLB），
而local_flush_tlb_all仅仅是invalidate本CPU core上的TLB。local_flush_tlb_all对应的底层接口是：tlbi vmalle1，没有is参数。
*/
static inline void flush_tlb_all(void)
{
	__DSB_FOR_TLBI(st);
	__TLBI(vmalle1);
	__DSB_FOR_TLBI();
	isb();
}

static inline void flush_tlb_mm(struct mm_struct *mm)
{
	unsigned long asid = ASID(mm) << 48;

	__DSB_FOR_TLBI(st);
	__TLBI(aside1, asid);
	__TLBI_USER(aside1, asid);
	__DSB_FOR_TLBI();
}

static inline void flush_tlb_page(struct vm_area_struct *vma,
				  unsigned long uaddr)
{
	unsigned long addr = uaddr >> 12 | (ASID(vma->vm_mm) << 48);

	__DSB_FOR_TLBI(st);
	__TLBI(vale1, addr);
	__TLBI_USER(vale1, addr);
	__DSB_FOR_TLBI();
}

/*
 * This is meant to avoid soft lock-ups on large TLB flushing ranges and not
 * necessarily a performance improvement.
 */
#define MAX_TLB_RANGE	(1024UL << PAGE_SHIFT)

static inline void __flush_tlb_range(struct vm_area_struct *vma,
				     unsigned long start, unsigned long end,
				     bool last_level)
{
	unsigned long asid = ASID(vma->vm_mm) << 48;
	unsigned long addr;

	if ((end - start) > MAX_TLB_RANGE) {
		flush_tlb_mm(vma->vm_mm);
		return;
	}

	start = asid | (start >> 12);
	end = asid | (end >> 12);

	__DSB_FOR_TLBI(st);
	for (addr = start; addr < end; addr += 1 << (PAGE_SHIFT - 12)) {
		if (last_level) {
			__TLBI(vale1, addr);
			__TLBI_USER(vale1, addr);
		} else {
			__TLBI(vae1, addr);
			__TLBI_USER(vae1, addr);
		}
	}
	__DSB_FOR_TLBI();
}

static inline void flush_tlb_range(struct vm_area_struct *vma,
				   unsigned long start, unsigned long end)
{
	__flush_tlb_range(vma, start, end, false);
}

static inline void flush_tlb_kernel_range(unsigned long start, unsigned long end)
{
	unsigned long addr;

	if ((end - start) > MAX_TLB_RANGE) {
		flush_tlb_all();
		return;
	}

	start >>= 12;
	end >>= 12;

	__DSB_FOR_TLBI(st);
	for (addr = start; addr < end; addr += 1 << (PAGE_SHIFT - 12))
		__TLBI(vaae1, addr);
	__DSB_FOR_TLBI();
	isb();
}

/*
 * Used to invalidate the TLB (walk caches) corresponding to intermediate page
 * table levels (pgd/pud/pmd).
 */
static inline void __flush_tlb_pgtable(struct mm_struct *mm,
				       unsigned long uaddr)
{
	unsigned long addr = uaddr >> 12 | (ASID(mm) << 48);

	__TLBI(vae1, addr);
	__TLBI_USER(vae1, addr);
	__DSB_FOR_TLBI();
}

#endif

#endif
