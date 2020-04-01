#ifndef _UAPI__LINUX_NETFILTER_H
#define _UAPI__LINUX_NETFILTER_H

#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/sysctl.h>
#include <linux/in.h>
#include <linux/in6.h>

/* Responses from hook functions. */
// 返回值判断流程参见           nf_hook_slow -> nf_iterate   
#define NF_DROP 0  // 直接drop掉这个数据包
#define NF_ACCEPT 1    // nf_iterate   数据包通过了挂载点的所有规则
#define NF_STOLEN 2   // 表示数据包保存到netfilter里，暂时不转到后续流程。  ip包分片包的处理会采用这种方式 ipv4_conntrack_defrag
#define NF_QUEUE 3   // 将数据包enque到用户空间的enque handler
#define NF_REPEAT 4  // 为netfilter的一个内部判定结果，需要重复该条规则的判定，直至不为NF_REPEAT
#define NF_STOP 5    // 数据包通过了挂载点的所有规则。
					// 但与NF_ACCEPT不同的一点时，当某条规则的判定结果为NF_STOP，那么可以直接返回结果NF_STOP，
					// 无需进行后面的判定了。而NF_ACCEPT需要所以的规则都为ACCEPT，才能返回NF_ACCEPT
#define NF_MAX_VERDICT NF_STOP

/* we overload the higher bits for encoding auxiliary data such as the queue
 * number or errno values. Not nice, but better than additional function
 * arguments. */
#define NF_VERDICT_MASK 0x000000ff

/* extra verdict flags have mask 0x0000ff00 */
#define NF_VERDICT_FLAG_QUEUE_BYPASS	0x00008000

/* queue number (NF_QUEUE) or errno (NF_DROP) */
#define NF_VERDICT_QMASK 0xffff0000
#define NF_VERDICT_QBITS 16

#define NF_QUEUE_NR(x) ((((x) << 16) & NF_VERDICT_QMASK) | NF_QUEUE)

#define NF_DROP_ERR(x) (((-x) << 16) | NF_DROP)

/* only for userspace compatibility */
#ifndef __KERNEL__
/* Generic cache responses from hook functions.
   <= 0x2000 is used for protocol-flags. */
#define NFC_UNKNOWN 0x4000
#define NFC_ALTERED 0x8000

/* NF_VERDICT_BITS should be 8 now, but userspace might break if this changes */
#define NF_VERDICT_BITS 16
#endif
// xt_table.valid_hooks    表示位表操作
// nf_hook_ops.hooknum
enum nf_inet_hooks {
	NF_INET_PRE_ROUTING,
	NF_INET_LOCAL_IN,
	NF_INET_FORWARD,
	NF_INET_LOCAL_OUT,
	NF_INET_POST_ROUTING,
	NF_INET_NUMHOOKS
};

enum nf_dev_hooks {
	NF_NETDEV_INGRESS,
	NF_NETDEV_NUMHOOKS
};
// xt_table.af
// nf_hook_ops.pf
enum {
	NFPROTO_UNSPEC =  0,
	NFPROTO_INET   =  1,
	NFPROTO_IPV4   =  2,
	NFPROTO_ARP    =  3,
	NFPROTO_NETDEV =  5,
	NFPROTO_BRIDGE =  7,
	NFPROTO_IPV6   = 10,
	NFPROTO_DECNET = 12,
	NFPROTO_NUMPROTO,
};

union nf_inet_addr {
	__u32		all[4];
	__be32		ip;
	__be32		ip6[4];
	struct in_addr	in;
	struct in6_addr	in6;
};

#endif /* _UAPI__LINUX_NETFILTER_H */
