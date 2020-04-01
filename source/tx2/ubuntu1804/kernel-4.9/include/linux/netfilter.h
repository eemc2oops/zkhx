#ifndef __LINUX_NETFILTER_H
#define __LINUX_NETFILTER_H

#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/net.h>
#include <linux/if.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/static_key.h>
#include <linux/netfilter_defs.h>
#include <linux/netdevice.h>
#include <net/net_namespace.h>

#ifdef CONFIG_NETFILTER
static inline int NF_DROP_GETERR(int verdict)
{
	return -(verdict >> NF_VERDICT_QBITS);
}

static inline int nf_inet_addr_cmp(const union nf_inet_addr *a1,
				   const union nf_inet_addr *a2)
{
	return a1->all[0] == a2->all[0] &&
	       a1->all[1] == a2->all[1] &&
	       a1->all[2] == a2->all[2] &&
	       a1->all[3] == a2->all[3];
}

static inline void nf_inet_addr_mask(const union nf_inet_addr *a1,
				     union nf_inet_addr *result,
				     const union nf_inet_addr *mask)
{
	result->all[0] = a1->all[0] & mask->all[0];
	result->all[1] = a1->all[1] & mask->all[1];
	result->all[2] = a1->all[2] & mask->all[2];
	result->all[3] = a1->all[3] & mask->all[3];
}

int netfilter_init(void);

struct sk_buff;

struct nf_hook_ops;

struct sock;
// nf_hook_state_init 里进行初始化赋值
// netfilter  hook 时，临时生成保存调用参数的结构
// nf_queue_entry.state   调用 queue 流程的数据结构 __nf_queue
struct nf_hook_state {
	unsigned int hook;  // 以太网 : NF_INET_PRE_ROUTING   表示数据包的处理步骤
	int thresh;  // INT_MIN
	u_int8_t pf;   // 以太网 :     NFPROTO_IPV4    表示协议
	struct net_device *in;
	struct net_device *out;
	struct sock *sk;
	struct net *net;
	struct nf_hook_entry __rcu *hook_entries;  // 指向　netns_nf.hooks[NFPROTO_NUMPROTO][NF_MAX_HOOKS]　中的某一条。
											// hook_entries　的值在 filter 流程里，会随时变化，但是变化的值，一定是
										   // netns_nf.hooks[NFPROTO_NUMPROTO][NF_MAX_HOOKS] 中的某一条
										   // 赋值流程 nf_queue
	int (*okfn)(struct net *, struct sock *, struct sk_buff *);  //  ip包收包流程  ip_rcv_finish  对应 NF_INET_PRE_ROUTING 链
};

typedef unsigned int nf_hookfn(void *priv,
			       struct sk_buff *skb,
			       const struct nf_hook_state *state);
// nf_hook_entry.ops
// xt_hook_ops_alloc 里创建本结构
// netfilter 的 hook 信息 所有的 hook 都挂在 nf_hook_list 里
// 同时，nf_hook_ops也会以nf_hook_entry.ops 的形式，挂在 netns_nf.hooks 里面
// ipv4 iptable_filter_init　里注册
struct nf_hook_ops {
	struct list_head	list;  // 挂在 nf_hook_list 里    _nf_register_hook 函数负责挂

	/* User fills in from here down. */
	nf_hookfn		*hook;   // 在 nf_iterate 里调用
						     // ipv4 : iptable_filter_hook
						     // ipv4 分片 : ipv4_conntrack_defrag   ( NF_INET_PRE_ROUTING )
						     // ipv4 分片 : ipv4_conntrack_defrag ( NF_INET_LOCAL_OUT   )
	struct net_device	*dev;
	void			*priv;
	u_int8_t		pf;  // ipv4 : NFPROTO_IPV4
	unsigned int		hooknum;  // ipv4 : NF_INET_LOCAL_IN   NF_INET_FORWARD   NF_INET_LOCAL_OUT
								// ipv4 分片 : NF_INET_PRE_ROUTING      NF_INET_LOCAL_OUT
	/* Hooks are ordered in ascending priority. */
	int			priority;  // ipv4  : NF_IP_PRI_FILTER
	                       // ipv4 分片 : NF_IP_PRI_CONNTRACK_DEFRAG
							// 注册时，使用本优先级比较插入位置                 nf_register_net_hook
};
// netns_nf.hooks
struct nf_hook_entry {
	struct nf_hook_entry __rcu	*next;   // 指向下一个 nf_hook_entry ，
										// nf_register_net_hook 注册时通过 nf_hook_ops.priority 判断插入位置
	struct nf_hook_ops		ops;
	const struct nf_hook_ops	*orig_ops;
};
// nf_hook_thresh -> nf_hook_state_init
static inline void nf_hook_state_init(struct nf_hook_state *p,
				      struct nf_hook_entry *hook_entry,
				      unsigned int hook,
				      int thresh, u_int8_t pf,
				      struct net_device *indev,
				      struct net_device *outdev,
				      struct sock *sk,
				      struct net *net,
				      int (*okfn)(struct net *, struct sock *, struct sk_buff *))
{
	p->hook = hook;
	p->thresh = thresh;
	p->pf = pf;
	p->in = indev;
	p->out = outdev;
	p->sk = sk;
	p->net = net;
	RCU_INIT_POINTER(p->hook_entries, hook_entry);
	p->okfn = okfn;
}



struct nf_sockopt_ops {
	struct list_head list;

	u_int8_t pf;

	/* Non-inclusive ranges: use 0/0/NULL to never get called. */
	int set_optmin;
	int set_optmax;
	int (*set)(struct sock *sk, int optval, void __user *user, unsigned int len);
#ifdef CONFIG_COMPAT
	int (*compat_set)(struct sock *sk, int optval,
			void __user *user, unsigned int len);
#endif
	int get_optmin;
	int get_optmax;
	int (*get)(struct sock *sk, int optval, void __user *user, int *len);
#ifdef CONFIG_COMPAT
	int (*compat_get)(struct sock *sk, int optval,
			void __user *user, int *len);
#endif
	/* Use the module struct to lock set/get code in place */
	struct module *owner;
};

/* Function to register/unregister hook points. */
int nf_register_net_hook(struct net *net, const struct nf_hook_ops *ops);
void nf_unregister_net_hook(struct net *net, const struct nf_hook_ops *ops);
int nf_register_net_hooks(struct net *net, const struct nf_hook_ops *reg,
			  unsigned int n);
void nf_unregister_net_hooks(struct net *net, const struct nf_hook_ops *reg,
			     unsigned int n);

int nf_register_hook(struct nf_hook_ops *reg);
void nf_unregister_hook(struct nf_hook_ops *reg);
int nf_register_hooks(struct nf_hook_ops *reg, unsigned int n);
void nf_unregister_hooks(struct nf_hook_ops *reg, unsigned int n);
int _nf_register_hooks(struct nf_hook_ops *reg, unsigned int n);
void _nf_unregister_hooks(struct nf_hook_ops *reg, unsigned int n);

/* Functions to register get/setsockopt ranges (non-inclusive).  You
   need to check permissions yourself! */
int nf_register_sockopt(struct nf_sockopt_ops *reg);
void nf_unregister_sockopt(struct nf_sockopt_ops *reg);

#ifdef HAVE_JUMP_LABEL
extern struct static_key nf_hooks_needed[NFPROTO_NUMPROTO][NF_MAX_HOOKS];
#endif

int nf_hook_slow(struct sk_buff *skb, struct nf_hook_state *state);

/**
 *	nf_hook_thresh - call a netfilter hook
 *
 *	Returns 1 if the hook has allowed the packet to pass.  The function
 *	okfn must be invoked by the caller in this case.  Any other return
 *	value indicates the packet has been consumed by the hook.
 */
// NF_HOOK_THRESH -> nf_hook_thresh(NFPROTO_IPV4, NF_INET_PRE_ROUTING,net, NULL, skb, dev, NULL, ip_rcv_finish, INT_MIN)
static inline int nf_hook_thresh(u_int8_t pf, unsigned int hook,
				 struct net *net,
				 struct sock *sk,
				 struct sk_buff *skb,
				 struct net_device *indev,
				 struct net_device *outdev,
				 int (*okfn)(struct net *, struct sock *, struct sk_buff *),
				 int thresh)
{
	struct nf_hook_entry *hook_head;
	int ret = 1;  // return 1 表示在函数退出时调用 okfn

#ifdef HAVE_JUMP_LABEL
	if (__builtin_constant_p(pf) &&
	    __builtin_constant_p(hook) &&
	    !static_key_false(&nf_hooks_needed[pf][hook]))
		return 1;
#endif

	rcu_read_lock();
	hook_head = rcu_dereference(net->nf.hooks[pf][hook]);
	if (hook_head) {
		struct nf_hook_state state;

		nf_hook_state_init(&state, hook_head, hook, thresh,
				   pf, indev, outdev, sk, net, okfn);

		ret = nf_hook_slow(skb, &state);
	}
	rcu_read_unlock();

	return ret;
}

static inline int nf_hook(u_int8_t pf, unsigned int hook, struct net *net,
			  struct sock *sk, struct sk_buff *skb,
			  struct net_device *indev, struct net_device *outdev,
			  int (*okfn)(struct net *, struct sock *, struct sk_buff *))
{
	return nf_hook_thresh(pf, hook, net, sk, skb, indev, outdev, okfn, INT_MIN);
}
                   
/* Activate hook; either okfn or kfree_skb called, unless a hook
   returns NF_STOLEN (in which case, it's up to the hook to deal with
   the consequences).

   Returns -ERRNO if packet dropped.  Zero means queued, stolen or
   accepted.
*/

/* RR:
   > I don't want nf_hook to return anything because people might forget
   > about async and trust the return value to mean "packet was ok".

   AK:
   Just document it clearly, then you can expect some sense from kernel
   coders :)
*/
// NF_HOOK -> NF_HOOK_THRESH(NFPROTO_IPV4, NF_INET_PRE_ROUTING,net, NULL, skb, dev, NULL, ip_rcv_finish, INT_MIN)
static inline int
NF_HOOK_THRESH(uint8_t pf, unsigned int hook, struct net *net, struct sock *sk,
	       struct sk_buff *skb, struct net_device *in,
	       struct net_device *out,
	       int (*okfn)(struct net *, struct sock *, struct sk_buff *),
	       int thresh)
{
	int ret = nf_hook_thresh(pf, hook, net, sk, skb, in, out, okfn, thresh);
	if (ret == 1)
		ret = okfn(net, sk, skb);
	return ret;
}

static inline int
NF_HOOK_COND(uint8_t pf, unsigned int hook, struct net *net, struct sock *sk,
	     struct sk_buff *skb, struct net_device *in, struct net_device *out,
	     int (*okfn)(struct net *, struct sock *, struct sk_buff *),
	     bool cond)
{
	int ret;

	if (!cond ||
	    ((ret = nf_hook_thresh(pf, hook, net, sk, skb, in, out, okfn, INT_MIN)) == 1))
		ret = okfn(net, sk, skb);
	return ret;
}

// ip_rcv -> NF_HOOK(NFPROTO_IPV4, NF_INET_PRE_ROUTING,net, NULL, skb, dev, NULL, ip_rcv_finish)
static inline int
NF_HOOK(uint8_t pf, unsigned int hook, struct net *net, struct sock *sk, struct sk_buff *skb,
	struct net_device *in, struct net_device *out,
	int (*okfn)(struct net *, struct sock *, struct sk_buff *))
{
	return NF_HOOK_THRESH(pf, hook, net, sk, skb, in, out, okfn, INT_MIN);
}

/* Call setsockopt() */
int nf_setsockopt(struct sock *sk, u_int8_t pf, int optval, char __user *opt,
		  unsigned int len);
int nf_getsockopt(struct sock *sk, u_int8_t pf, int optval, char __user *opt,
		  int *len);
#ifdef CONFIG_COMPAT
int compat_nf_setsockopt(struct sock *sk, u_int8_t pf, int optval,
		char __user *opt, unsigned int len);
int compat_nf_getsockopt(struct sock *sk, u_int8_t pf, int optval,
		char __user *opt, int *len);
#endif

/* Call this before modifying an existing packet: ensures it is
   modifiable and linear to the point you care about (writable_len).
   Returns true or false. */
int skb_make_writable(struct sk_buff *skb, unsigned int writable_len);

struct flowi;
struct nf_queue_entry;

// ipv4_netfilter_init 里注册 nf_ip_afinfo
// 所有的本结构都  通过         nf_register_afinfo  注册在 nf_afinfo[NFPROTO_NUMPROTO] 里
struct nf_afinfo {
	unsigned short	family;  // ipv4 : AF_INET
	__sum16		(*checksum)(struct sk_buff *skb, unsigned int hook,
				    unsigned int dataoff, u_int8_t protocol);  // ipv4 : nf_ip_checksum
	__sum16		(*checksum_partial)(struct sk_buff *skb,
					    unsigned int hook,
					    unsigned int dataoff,
					    unsigned int len,
					    u_int8_t protocol);  // ipv4 : nf_ip_checksum_partial
	int		(*route)(struct net *net, struct dst_entry **dst,
				 struct flowi *fl, bool strict); // ipv4 : nf_ip_route
	void		(*saveroute)(const struct sk_buff *skb,
				     struct nf_queue_entry *entry); // __nf_queue 里调用
				     								// ipv4 : nf_ip_saveroute
	int		(*reroute)(struct net *net, struct sk_buff *skb,
				   const struct nf_queue_entry *entry); // ipv4 : nf_ip_reroute
	int		route_key_size;  // ipv4 : sizeof(struct ip_rt_info)
};

// extern const struct nf_afinfo __rcu *nf_afinfo[NFPROTO_NUMPROTO]; // 源码定义这一行
// nf_register_afinfo 是对应的注册函数
// __nf_queue -> nf_get_afinfo
static inline const struct nf_afinfo *nf_get_afinfo(unsigned short family)
{
	return rcu_dereference(nf_afinfo[family]);
}

static inline __sum16
nf_checksum(struct sk_buff *skb, unsigned int hook, unsigned int dataoff,
	    u_int8_t protocol, unsigned short family)
{
	const struct nf_afinfo *afinfo;
	__sum16 csum = 0;

	rcu_read_lock();
	afinfo = nf_get_afinfo(family);
	if (afinfo)
		csum = afinfo->checksum(skb, hook, dataoff, protocol);
	rcu_read_unlock();
	return csum;
}

static inline __sum16
nf_checksum_partial(struct sk_buff *skb, unsigned int hook,
		    unsigned int dataoff, unsigned int len,
		    u_int8_t protocol, unsigned short family)
{
	const struct nf_afinfo *afinfo;
	__sum16 csum = 0;

	rcu_read_lock();
	afinfo = nf_get_afinfo(family);
	if (afinfo)
		csum = afinfo->checksum_partial(skb, hook, dataoff, len,
						protocol);
	rcu_read_unlock();
	return csum;
}

int nf_register_afinfo(const struct nf_afinfo *afinfo);
void nf_unregister_afinfo(const struct nf_afinfo *afinfo);

#include <net/flow.h>
extern void (*nf_nat_decode_session_hook)(struct sk_buff *, struct flowi *);

static inline void
nf_nat_decode_session(struct sk_buff *skb, struct flowi *fl, u_int8_t family)
{
#ifdef CONFIG_NF_NAT_NEEDED
	void (*decodefn)(struct sk_buff *, struct flowi *);

	rcu_read_lock();
	decodefn = rcu_dereference(nf_nat_decode_session_hook);
	if (decodefn)
		decodefn(skb, fl);
	rcu_read_unlock();
#endif
}

#else /* !CONFIG_NETFILTER */
static inline int
NF_HOOK_COND(uint8_t pf, unsigned int hook, struct net *net, struct sock *sk,
	     struct sk_buff *skb, struct net_device *in, struct net_device *out,
	     int (*okfn)(struct net *, struct sock *, struct sk_buff *),
	     bool cond)
{
	return okfn(net, sk, skb);
}

static inline int
NF_HOOK(uint8_t pf, unsigned int hook, struct net *net, struct sock *sk,
	struct sk_buff *skb, struct net_device *in, struct net_device *out,
	int (*okfn)(struct net *, struct sock *, struct sk_buff *))
{
	return okfn(net, sk, skb);
}

static inline int nf_hook(u_int8_t pf, unsigned int hook, struct net *net,
			  struct sock *sk, struct sk_buff *skb,
			  struct net_device *indev, struct net_device *outdev,
			  int (*okfn)(struct net *, struct sock *, struct sk_buff *))
{
	return 1;
}
struct flowi;
static inline void
nf_nat_decode_session(struct sk_buff *skb, struct flowi *fl, u_int8_t family)
{
}
#endif /*CONFIG_NETFILTER*/

#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
#include <linux/netfilter/nf_conntrack_zones_common.h>

extern void (*ip_ct_attach)(struct sk_buff *, const struct sk_buff *) __rcu;
void nf_ct_attach(struct sk_buff *, const struct sk_buff *);
extern void (*nf_ct_destroy)(struct nf_conntrack *) __rcu;
#else
static inline void nf_ct_attach(struct sk_buff *new, struct sk_buff *skb) {}
#endif

struct nf_conn;
enum ip_conntrack_info;
struct nlattr;

struct nfnl_ct_hook {
	struct nf_conn *(*get_ct)(const struct sk_buff *skb,
				  enum ip_conntrack_info *ctinfo);
	size_t (*build_size)(const struct nf_conn *ct);
	int (*build)(struct sk_buff *skb, struct nf_conn *ct,
		     enum ip_conntrack_info ctinfo,
		     u_int16_t ct_attr, u_int16_t ct_info_attr);
	int (*parse)(const struct nlattr *attr, struct nf_conn *ct);
	int (*attach_expect)(const struct nlattr *attr, struct nf_conn *ct,
			     u32 portid, u32 report);
	void (*seq_adjust)(struct sk_buff *skb, struct nf_conn *ct,
			   enum ip_conntrack_info ctinfo, s32 off);
};
extern struct nfnl_ct_hook __rcu *nfnl_ct_hook;

/**
 * nf_skb_duplicated - TEE target has sent a packet
 *
 * When a xtables target sends a packet, the OUTPUT and POSTROUTING
 * hooks are traversed again, i.e. nft and xtables are invoked recursively.
 *
 * This is used by xtables TEE target to prevent the duplicated skb from
 * being duplicated again.
 */
DECLARE_PER_CPU(bool, nf_skb_duplicated);

#endif /*__LINUX_NETFILTER_H*/
