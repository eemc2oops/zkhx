/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		INET protocol dispatch tables.
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 * Fixes:
 *		Alan Cox	: Ahah! udp icmp errors don't work because
 *				  udp_err is never called!
 *		Alan Cox	: Added new fields for init and ready for
 *				  proper fragmentation (_NO_ 4K limits!)
 *		Richard Colella	: Hang on hash collision
 *		Vince Laviano	: Modified inet_del_protocol() to correctly
 *				  maintain copy bit.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#include <linux/cache.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>
#include <net/protocol.h>

// const struct net_protocol __rcu *inet_protos[MAX_INET_PROTOS] __read_mostly;  // 源码定义在这一行。
const struct net_protocol *inet_protos[MAX_INET_PROTOS];  // inet_add_protocol 里添加支持的协议
											// inet_protos[IPPROTO_ICMP] = icmp_protocol
											// inet_protos[IPPROTO_UDP] = udp_protocol
											// inet_protos[IPPROTO_TCP] = tcp_protocol
											// inet_protos[IPPROTO_IGMP] = igmp_protocol

// const struct net_offload __rcu *inet_offloads[MAX_INET_PROTOS] __read_mostly; // 源码定义在这一行
const struct net_offload *inet_offloads[MAX_INET_PROTOS];

EXPORT_SYMBOL(inet_offloads);
// inet_init -> inet_add_protocol(&icmp_protocol, IPPROTO_ICMP)
// inet_init -> inet_add_protocol(&udp_protocol, IPPROTO_UDP)
// inet_init -> inet_add_protocol(&tcp_protocol, IPPROTO_TCP)
// inet_init -> inet_add_protocol(&igmp_protocol, IPPROTO_IGMP)
int inet_add_protocol(const struct net_protocol *prot, unsigned char protocol)
{
	if (!prot->netns_ok) {
		pr_err("Protocol %u is not namespace aware, cannot register.\n",
			protocol);
		return -EINVAL;
	}

	return !cmpxchg((const struct net_protocol **)&inet_protos[protocol],
			NULL, prot) ? 0 : -1;
}
EXPORT_SYMBOL(inet_add_protocol);

int inet_add_offload(const struct net_offload *prot, unsigned char protocol)
{
	return !cmpxchg((const struct net_offload **)&inet_offloads[protocol],
			NULL, prot) ? 0 : -1;
}
EXPORT_SYMBOL(inet_add_offload);

int inet_del_protocol(const struct net_protocol *prot, unsigned char protocol)
{
	int ret;

	ret = (cmpxchg((const struct net_protocol **)&inet_protos[protocol],
		       prot, NULL) == prot) ? 0 : -1;

	synchronize_net();

	return ret;
}
EXPORT_SYMBOL(inet_del_protocol);

int inet_del_offload(const struct net_offload *prot, unsigned char protocol)
{
	int ret;

	ret = (cmpxchg((const struct net_offload **)&inet_offloads[protocol],
		       prot, NULL) == prot) ? 0 : -1;

	synchronize_net();

	return ret;
}
EXPORT_SYMBOL(inet_del_offload);
