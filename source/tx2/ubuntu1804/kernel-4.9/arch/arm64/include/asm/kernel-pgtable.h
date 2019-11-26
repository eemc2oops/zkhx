/*
 * Kernel page table mapping
 *
 * Copyright (C) 2015 ARM Ltd.
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

#ifndef __ASM_KERNEL_PGTABLE_H
#define __ASM_KERNEL_PGTABLE_H

#include <asm/pgtable.h>
#include <asm/sparsemem.h>

/*
 * The linear mapping and the start of memory are both 2M aligned (per
 * the arm64 booting.txt requirements). Hence we can use section mapping
 * with 4K (section size = 2M) but not with 16K (section size = 32M) or
 * 64K (section size = 512M).
 */
#ifdef CONFIG_ARM64_4K_PAGES            // tx2 : 定义了这个宏
#define ARM64_SWAPPER_USES_SECTION_MAPS 1      // 页大小是4K，则可以启用section map, 
                                               // 系统启动MMU前，需要先给内核image建立好地址映射表．
                                               // 内核映像的大小是确定的．考虑采用尽量少的表项进行映射，以节约内存
                                               // PTE 4K page 有512个表项，可以映射512*4K 共2M的内存
                                               // PTE 16K page 有 2K个表项，可以映射 2K * 16K　共   32M的内存
                                               // PTE 64K page 有 4K个表项，可以映射 8K * 64K　共   512M的内存
                                               // 32M,512M基本可以满足要求，但是2M满足不了kernel imagme的运行要求
                                               // 所以，在4K page时，PTE的上一级PMD需指向mem block，可以映射 512 * 2M                     共1G内存
                                               // tx2 走这个分支
#else
#define ARM64_SWAPPER_USES_SECTION_MAPS 0
#endif

/*
 * The idmap and swapper page tables need some space reserved in the kernel
 * image. Both require pgd, pud (4 levels only) and pmd tables to (section)
 * map the kernel. With the 64K page configuration, swapper and idmap need to
 * map to pte level. The swapper also maps the FDT (see __create_page_tables
 * for more information). Note that the number of ID map translation levels
 * could be increased on the fly if system RAM is out of reach for the default
 * VA range, so pages required to map highest possible PA are reserved in all
 * cases.
 */
#if ARM64_SWAPPER_USES_SECTION_MAPS                            // 这两个值是从 vmlinux.lds 的 idmap_pg_dir 里查出来的
#define SWAPPER_PGTABLE_LEVELS	(CONFIG_PGTABLE_LEVELS - 1)    // create_table_entry 里判断建立几级页表  /arch/arm64/kernel/head.s 
                                                                // tx2 : 2        (3 - 1)
#define IDMAP_PGTABLE_LEVELS	(ARM64_HW_PGTABLE_LEVELS(PHYS_MASK_SHIFT) - 1)   // tx2 : 3   (((((48)) - 4) / (12 - 3)) - 1)
#else
#define SWAPPER_PGTABLE_LEVELS	(CONFIG_PGTABLE_LEVELS)
#define IDMAP_PGTABLE_LEVELS	(ARM64_HW_PGTABLE_LEVELS(PHYS_MASK_SHIFT))
#endif

                                                                 // 这两个值是从 vmlinux.lds 的 idmap_pg_dir 里查出来的
#define SWAPPER_DIR_SIZE	(SWAPPER_PGTABLE_LEVELS * PAGE_SIZE) // tx2 SWAPPER_DIR_SIZE = 0x2000  :  8K       ((3 - 1) * (1 << 12))
#define IDMAP_DIR_SIZE		(IDMAP_PGTABLE_LEVELS * PAGE_SIZE)   // tx2 IDMAP_DIR_SIZE = 0x3000  :  12K  ((((((48)) - 4) / (12 - 3)) - 1) * (1 << 12)) 
                                                                 

#ifdef CONFIG_ARM64_SW_TTBR0_PAN   // tx2 定义了这个宏
#define RESERVED_TTBR0_SIZE	(PAGE_SIZE)    //  tx2 : PAGE_SIZE = 4K  4096
#else
#define RESERVED_TTBR0_SIZE	(0)
#endif

/* Initial memory map size */
#if ARM64_SWAPPER_USES_SECTION_MAPS   // tx2   ARM64_SWAPPER_USES_SECTION_MAPS = 1
#define SWAPPER_BLOCK_SHIFT	SECTION_SHIFT   // tx2 : 21
#define SWAPPER_BLOCK_SIZE	SECTION_SIZE    // tx2 : 2M   0x200000
#define SWAPPER_TABLE_SHIFT	PUD_SHIFT       // tx2 : 30
#else
#define SWAPPER_BLOCK_SHIFT	PAGE_SHIFT
#define SWAPPER_BLOCK_SIZE	PAGE_SIZE
#define SWAPPER_TABLE_SHIFT	PMD_SHIFT
#endif

/* The size of the initial kernel direct mapping */
#define SWAPPER_INIT_MAP_SIZE	(_AC(1, UL) << SWAPPER_TABLE_SHIFT)  // tx2 : 0x40000000  1G

/*
 * Initial memory map attributes.
 */
#define _SWAPPER_PTE_FLAGS	(PTE_TYPE_PAGE | PTE_AF | PTE_SHARED)
#define _SWAPPER_PMD_FLAGS	(PMD_TYPE_SECT | PMD_SECT_AF | PMD_SECT_S)

#ifdef CONFIG_UNMAP_KERNEL_AT_EL0   // tx2没有定义这个宏
#define SWAPPER_PTE_FLAGS	(_SWAPPER_PTE_FLAGS | PTE_NG)
#define SWAPPER_PMD_FLAGS	(_SWAPPER_PMD_FLAGS | PMD_SECT_NG)
#else
#define SWAPPER_PTE_FLAGS	_SWAPPER_PTE_FLAGS  // tx2走这里
#define SWAPPER_PMD_FLAGS	_SWAPPER_PMD_FLAGS  // tx2走这里
#endif

#if ARM64_SWAPPER_USES_SECTION_MAPS
#define SWAPPER_MM_MMUFLAGS	(PMD_ATTRINDX(MT_NORMAL) | SWAPPER_PMD_FLAGS)  //
                                                                           // tx2 : 0x711
#else
#define SWAPPER_MM_MMUFLAGS	(PTE_ATTRINDX(MT_NORMAL) | SWAPPER_PTE_FLAGS)
#endif

/*
 * To make optimal use of block mappings when laying out the linear
 * mapping, round down the base of physical memory to a size that can
 * be mapped efficiently, i.e., either PUD_SIZE (4k granule) or PMD_SIZE
 * (64k granule), or a multiple that can be mapped using contiguous bits
 * in the page tables: 32 * PMD_SIZE (16k granule)
 */
#if defined(CONFIG_ARM64_4K_PAGES)
#define ARM64_MEMSTART_SHIFT		PUD_SHIFT
#elif defined(CONFIG_ARM64_16K_PAGES)
#define ARM64_MEMSTART_SHIFT		(PMD_SHIFT + 5)
#else
#define ARM64_MEMSTART_SHIFT		PMD_SHIFT
#endif

/*
 * sparsemem vmemmap imposes an additional requirement on the alignment of
 * memstart_addr, due to the fact that the base of the vmemmap region
 * has a direct correspondence, and needs to appear sufficiently aligned
 * in the virtual address space.
 */
#if defined(CONFIG_SPARSEMEM_VMEMMAP) && ARM64_MEMSTART_SHIFT < SECTION_SIZE_BITS
#define ARM64_MEMSTART_ALIGN	(1UL << SECTION_SIZE_BITS)
#else
#define ARM64_MEMSTART_ALIGN	(1UL << ARM64_MEMSTART_SHIFT)
#endif

#endif	/* __ASM_KERNEL_PGTABLE_H */
