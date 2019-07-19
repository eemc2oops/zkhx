/*
 * Copyright (C) 2015, Intel Corporation
 * Author: Jiang Liu <jiang.liu@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */
#ifndef _LINUX_RESOURCE_EXT_H
#define _LINUX_RESOURCE_EXT_H
#include <linux/types.h>
#include <linux/list.h>
#include <linux/ioport.h>
#include <linux/slab.h>

/* Represent resource window for bridge devices */
struct resource_win {
	struct resource res;		/* In master (CPU) address space */
	resource_size_t offset;		/* Translation offset for bridge */
};

/*
 * Common resource list management data structure and interfaces to support
 * ACPI, PNP and PCI host bridge etc.
 */
struct resource_entry {
	struct list_head	node;
	struct resource		*res;	/* In master (CPU) address space */
	resource_size_t		offset;	/* Translation offset for bridge */
	struct resource		__res;	/* Default storage for res */
};

extern struct resource_entry *
resource_list_create_entry(struct resource *res, size_t extra_size);
extern void resource_list_free(struct list_head *head);

static inline void resource_list_add(struct resource_entry *entry,
				     struct list_head *head)
{
	list_add(&entry->node, head);
}
// pci_add_resource_offset -> resource_list_add_tail
static inline void resource_list_add_tail(struct resource_entry *entry,
					  struct list_head *head)
{
	list_add_tail(&entry->node, head);
}

static inline void resource_list_del(struct resource_entry *entry)
{
	list_del(&entry->node);
}

static inline void resource_list_free_entry(struct resource_entry *entry)
{
	kfree(entry);
}

static inline void
resource_list_destroy_entry(struct resource_entry *entry)
{
	resource_list_del(entry);
	resource_list_free_entry(entry);
}

#define resource_list_for_each_entry(entry, list)	\
	list_for_each_entry((entry), (list), node)

#define resource_list_for_each_entry_safe(entry, tmp, list)	\
	list_for_each_entry_safe((entry), (tmp), (list), node)

#endif /* _LINUX_RESOURCE_EXT_H */
