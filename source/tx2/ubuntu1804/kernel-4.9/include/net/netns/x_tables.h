#ifndef __NETNS_X_TABLES_H
#define __NETNS_X_TABLES_H

#include <linux/list.h>
#include <linux/netfilter_defs.h>

struct ebt_table;

struct netns_xt {
	struct list_head tables[NFPROTO_NUMPROTO];   // 下标是协议类型 NFPROTO_IPV4 
												// xt_register_table 里往这个链表里挂数据
												// xt_table.list 挂在 tables[x] 链表里
	bool notrack_deprecated_warning;
	bool clusterip_deprecated_warning;
#if defined(CONFIG_BRIDGE_NF_EBTABLES) || \
    defined(CONFIG_BRIDGE_NF_EBTABLES_MODULE)
	struct ebt_table *broute_table;
	struct ebt_table *frame_filter;
	struct ebt_table *frame_nat;
#endif
};
#endif
