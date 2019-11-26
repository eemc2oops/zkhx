/*
 * Based on arch/arm/include/asm/memory.h
 *
 * Copyright (C) 2000-2002 Russell King
 * Copyright (C) 2012 ARM Ltd.
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
 *
 * Note: this file should not be included by non-asm/.h files
 */
#ifndef __ASM_MEMORY_H
#define __ASM_MEMORY_H

#include <linux/compiler.h>
#include <linux/const.h>
#include <linux/types.h>
#include <asm/bug.h>
#include <asm/sizes.h>

/*
 * Allow for constants defined here to be used from assembly code
 * by prepending the UL suffix only with actual C code compilation.
 */
#define UL(x) _AC(x, UL)

/*
 * Size of the PCI I/O space. This must remain a power of two so that
 * IO_SPACE_LIMIT acts as a mask for the low bits of I/O addresses.
 */
#define PCI_IO_SIZE		SZ_16M

/*
 * Log2 of the upper bound of the size of a struct page. Used for sizing
 * the vmemmap region only, does not affect actual memory footprint.
 * We don't use sizeof(struct page) directly since taking its size here
 * requires its definition to be available at this point in the inclusion
 * chain, and it may not be a power of 2 in the first place.
 */
#define STRUCT_PAGE_MAX_SHIFT	6

/*
 * VMEMMAP_SIZE - allows the whole linear region to be covered by
 *                a struct page array
 */
#define VMEMMAP_SIZE (UL(1) << (VA_BITS - PAGE_SHIFT - 1 + STRUCT_PAGE_MAX_SHIFT))

/*
 * PAGE_OFFSET - the virtual address of the start of the linear map (top
 *		 (VA_BITS - 1))
 * KIMAGE_VADDR - the virtual address of the start of the kernel image
 * VA_BITS - the maximum number of bits for virtual addresses.
 * VA_START - the first kernel virtual address.
 */

/*
48 bit ,4k page, 采用4级映射
Translation table lookup with 4KB pages:
+--------+--------+--------+--------+--------+--------+--------+--------+
|63    56|55    48|47    40|39    32|31    24|23    16|15     8|7      0|
+--------+--------+--------+--------+--------+--------+--------+--------+
|                 |         |         |         |         |
|                 |         |         |         |         v
|                 |         |         |         |   [11:0]  in-page offset
 
|                 |         |         |         +-> [20:12] L3 index
 
|                 |         |         +-----------> [29:21] L2 index
 
|                 |         +---------------------> [38:30] L1 index
 
|                 +-------------------------------> [47:39] L0 index
+-------------------------------------------------> [63] TTBR0/1
*/

/*
TX2 使用39bit地址空间，4K page，3级映射
Translation table lookup with 4KB pages:
+--------+--------+--------+--------+--------+--------+--------+--------+
|63    56|55    48|47    40|39    32|31    24|23    16|15     8|7      0|
+--------+--------+--------+--------+--------+--------+--------+--------+
|                           |         |         |         |
|                           |         |         |         v
|                           |         |         |   [11:0]  in-page offset 
|                           |         |         +-> [20:12] L2 index 
|                           |         +-----------> [29:21] L1 index 
|                           +---------------------> [38:30] L0 index
+-------------------------------------------------> [63] TTBR0/1

如果 L1 映射表里，保存的是block mem的话，则L1表项可以指向 2MB (0-20共21bit,表示2MB)
*/

/*
TX2   kernel 4.9
Memory: 7976748K/8220672K available (14846K kernel code, 2394K rwdata, 6456K rodata, 7488K init, 605K bss, 178388K reserved, 65536K cma-reserved)
Virtual kernel memory layout:
modules : 0xffffff8000000000 - 0xffffff8008000000   (   128 MB)
vmalloc : 0xffffff8008000000 - 0xffffffbebfff0000   (   250 GB)
.text : 0xffffff8008080000 - 0xffffff8008f00000   ( 14848 KB)
.rodata : 0xffffff8008f00000 - 0xffffff8009560000   (  6528 KB)
.init : 0xffffff8009560000 - 0xffffff8009cb0000   (  7488 KB)
.data : 0xffffff8009cb0000 - 0xffffff8009f06808   (  2395 KB)
.bss : 0xffffff8009f06808 - 0xffffff8009f9deb4   (   606 KB)
fixed   : 0xffffffbefe7fd000 - 0xffffffbefec00000   (  4108 KB)
PCI I/O : 0xffffffbefee00000 - 0xffffffbeffe00000   (    16 MB)
vmemmap : 0xffffffbf00000000 - 0xffffffc000000000   (     4 GB maximum)
          0xffffffbf00000000 - 0xffffffbf07dc8000   (   125 MB actual)
memory  : 0xffffffc000000000 - 0xffffffc1f7200000   (  8050 MB)
*/

/*
为了查找代码方便把TEXT_OFFSET的宏定义搬到这里    　　这个定义是 tx2 里的定义                          512K
正常状态下这个宏是编译时传入的，在makefile里定义                arch/arm64/Makefile 里定义
*/
#define TEXT_OFFSET   0x00080000     /* bootloader会把kernel image从外设copy到RAM中，那么具体copy到什么位置呢？从RAM的起始地址开始吗？
                                        实际上是从TEXT_OFFSET开始的，偏移这么一小段内存估计是为了bootloader和kernel之间传递一些信息。
                                        所以，这里TEXT是指kernel text segment，而OFFSET是相对于RAM的首地址而言的。
                                        TEXT_OFFSET必须要4K对齐并且TEXT_OFFSET的size不能大于2M。
                                     */


#define VA_BITS			(CONFIG_ARM64_VA_BITS)    //  虚拟地址宽度,最大值为48,
                                                  //  比如配置为39，       空间大于是      512G
                                                  //  用户地址 0x00000000_00000000 - 0x0000007f_ffffffff
                                                  //  内核空间地址为 0xffffff80_00000000 - 0xffffffff_ffffffff ,分别为 512G
                                                  //  tx2  : 39
#define VA_START		(UL(0xffffffffffffffff) - (UL(1) << VA_BITS) + 1)       // 内核虚拟地址的起始地址     
                                                                                // 地址宽度是  　39 时      VA_START = 0xffffff80_00000000
#define PAGE_OFFSET		(UL(0xffffffffffffffff) - (UL(1) << (VA_BITS - 1)) + 1)   //  内核里映像看到的物理内存映射起始地址
                                                                                  //  地址宽度是 39 时 PAGE_OFFSET = 0xffffffC0_00000000
#define KIMAGE_VADDR		(MODULES_END)                      // kernel image start address
                                                               // tx2 : 0xffffff8008000000
#define MODULES_END		(MODULES_VADDR + MODULES_VSIZE)        // 内核模块的结束地址
                                                               // tx2 MODULES_END = VA_START + SZ_128M         0xffffff80_08000000
#define MODULES_VADDR		(VA_START + KASAN_SHADOW_SIZE)     // 内核模块的起始地址
                                                               // tx2  MODULES_VADDR = VA_START    0xffffff80_00000000
#define MODULES_VSIZE		(SZ_128M)
#define VMEMMAP_START		(PAGE_OFFSET - VMEMMAP_SIZE)
#define PCI_IO_END		(VMEMMAP_START - SZ_2M)
#define PCI_IO_START		(PCI_IO_END - PCI_IO_SIZE)
#define FIXADDR_TOP		(PCI_IO_START - SZ_2M)

/* The physical address of the end of vmemmap */
#define VMEMMAP_END_PHYS	((VMEMMAP_SIZE >> STRUCT_PAGE_MAX_SHIFT) << PAGE_SHIFT)

#define KERNEL_START      _text   // _text表示内核映像段的起始地址　(虚拟地址)
                                // _text 在 arch/arm64/kernel/vmlinux.lds.s 里定义成 KIMAGE_VADDR + TEXT_OFFSET
                                // tx2 : 0xFFFFFF8008080000
#define KERNEL_END        _end   // _end表示映射结束的地址　(虚拟地址)

/*
 * The size of the KASAN shadow region. This should be 1/8th of the
 * size of the entire kernel virtual address space.
 */
#ifdef CONFIG_KASAN    // tx2 没定义这个
#define KASAN_SHADOW_SIZE	(UL(1) << (VA_BITS - 3))
#else
#define KASAN_SHADOW_SIZE	(0)    //  tx2 采用这个定义
#endif

/*
 * Physical vs virtual RAM address space conversion.  These are
 * private definitions which should NOT be used outside memory.h
 * files.  Use virt_to_phys/phys_to_virt/__pa/__va instead.
 */
#define __virt_to_phys(x) ({						\
	phys_addr_t __x = (phys_addr_t)(x);				\
	__x & BIT(VA_BITS - 1) ? (__x & ~PAGE_OFFSET) + PHYS_OFFSET :	\
				 (__x - kimage_voffset); })

#define __phys_to_virt(x)	((unsigned long)((x) - PHYS_OFFSET) | PAGE_OFFSET)
#define __phys_to_kimg(x)	((unsigned long)((x) + kimage_voffset))

/*
 * Convert a page to/from a physical address
 */
#define page_to_phys(page)	(__pfn_to_phys(page_to_pfn(page)))
#define phys_to_page(phys)	(pfn_to_page(__phys_to_pfn(phys)))

/*
 * Memory types available.
 */
#define MT_DEVICE_nGnRnE	0
#define MT_DEVICE_nGnRE		1
#define MT_DEVICE_GRE		2
#define MT_NORMAL_NC		3
#define MT_NORMAL		4
#define MT_NORMAL_WT		5

/*
 * Memory types for Stage-2 translation
 */
#define MT_S2_NORMAL		0xf
#define MT_S2_DEVICE_nGnRE	0x1

#ifdef CONFIG_ARM64_4K_PAGES
#define IOREMAP_MAX_ORDER	(PUD_SHIFT)
#else
#define IOREMAP_MAX_ORDER	(PMD_SHIFT)
#endif

#ifdef CONFIG_BLK_DEV_INITRD
#define __early_init_dt_declare_initrd(__start, __end)			\
	do {								\
		initrd_start = (__start);				\
		initrd_end = (__end);					\
	} while (0)
#endif

#ifndef __ASSEMBLY__

#include <linux/bitops.h>
#include <linux/mmdebug.h>

extern s64			memstart_addr;
/* PHYS_OFFSET - the physical address of the start of memory. */
#define PHYS_OFFSET		({ VM_BUG_ON(memstart_addr & 1); memstart_addr; })  /*系统内存的起始物理地址。
                                                                             在系统初始化的过程中，
                                                                          会把PHYS_OFFSET开始的物理内存映射到PAGE_OFFSET的虚拟内存上去。
                                                                           */
                                                                           // tx2 : 

/* the virtual base of the kernel image (minus TEXT_OFFSET) */
extern u64			kimage_vaddr;    // arch/arm64/kernel/head.s 里定义　__primary_switched　里赋值
                                    // 赋值为内核的起始地址，　虚拟地址
                                    // tx2 : kimage_vaddr : 0xFFFFFF8008000000

/* the offset between the kernel virtual and physical mappings */
extern u64			kimage_voffset;  // arch/arm64/kernel/head.s 的函数    __primary_switched 里赋值
                                    // 表示内核起始虚拟地址和内核起始物理地址的差值
                                    // 变量定义在 arch/arm64/mm/mmu.c 里

static inline unsigned long kaslr_offset(void)
{
	return kimage_vaddr - KIMAGE_VADDR;
}

/*
 * Allow all memory at the discovery stage. We will clip it later.
 */
#define MIN_MEMBLOCK_ADDR	0
#define MAX_MEMBLOCK_ADDR	U64_MAX

/*
 * PFNs are used to describe any physical page; this means
 * PFN 0 == physical address 0.
 *
 * This is the PFN of the first RAM page in the kernel
 * direct-mapped view.  We assume this is the first page
 * of RAM in the mem_map as well.
 */
#define PHYS_PFN_OFFSET	(PHYS_OFFSET >> PAGE_SHIFT)

/*
 * Note: Drivers should NOT use these.  They are the wrong
 * translation for translating DMA addresses.  Use the driver
 * DMA support - see dma-mapping.h.
 */
#define virt_to_phys virt_to_phys
static inline phys_addr_t virt_to_phys(const volatile void *x)
{
	return __virt_to_phys((unsigned long)(x));
}

#define phys_to_virt phys_to_virt
static inline void *phys_to_virt(phys_addr_t x)
{
	return (void *)(__phys_to_virt(x));
}

/*
 * Drivers should NOT use these either.
 */
#define __pa(x)			__virt_to_phys((unsigned long)(x))
#define __va(x)			((void *)__phys_to_virt((phys_addr_t)(x)))
#define pfn_to_kaddr(pfn)	__va((pfn) << PAGE_SHIFT)
#define virt_to_pfn(x)      __phys_to_pfn(__virt_to_phys(x))
#define sym_to_pfn(x)	    __phys_to_pfn(__pa_symbol(x))

/*
 *  virt_to_page(k)	convert a _valid_ virtual address to struct page *
 *  virt_addr_valid(k)	indicates whether a virtual address is valid
 */
#define ARCH_PFN_OFFSET		((unsigned long)PHYS_PFN_OFFSET)

#ifndef CONFIG_SPARSEMEM_VMEMMAP
#define virt_to_page(kaddr)	pfn_to_page(__pa(kaddr) >> PAGE_SHIFT)
#define _virt_addr_valid(kaddr)	pfn_valid(__pa(kaddr) >> PAGE_SHIFT)
#else
#define __virt_to_pgoff(kaddr)	(((u64)(kaddr) & ~PAGE_OFFSET) / PAGE_SIZE * sizeof(struct page))
#define __page_to_voff(kaddr)	(((u64)(kaddr) & ~VMEMMAP_START) * PAGE_SIZE / sizeof(struct page))

#define page_to_virt(page)	((void *)((__page_to_voff(page)) | PAGE_OFFSET))
#define virt_to_page(vaddr)	((struct page *)((__virt_to_pgoff(vaddr)) | VMEMMAP_START))

#define _virt_addr_valid(kaddr)	pfn_valid((((u64)(kaddr) & ~PAGE_OFFSET) \
					   + PHYS_OFFSET) >> PAGE_SHIFT)
#endif
#endif

#define _virt_addr_is_linear(kaddr)	(((u64)(kaddr)) >= PAGE_OFFSET)
#define virt_addr_valid(kaddr)		(_virt_addr_is_linear(kaddr) && \
					 _virt_addr_valid(kaddr))

#include <asm-generic/memory_model.h>

#endif
