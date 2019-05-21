/* 
   bsl_netdev.c: bsl network driver from a dummy net driver
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/rtnetlink.h>
#include <net/rtnetlink.h>
#include <linux/u64_stats_sync.h>

extern int bsl_max_devcount(void);
extern unsigned long long bsl_read_reg(int devid, int addr);
extern void bsl_write_reg(int devid, int addr, uint64_t val);


/* fake multicast ability */
static void set_multicast_list(struct net_device *dev)
{
}

struct pcpu_dstats {
	u64			tx_packets;
	u64			tx_bytes;
	struct u64_stats_sync	syncp;
};

static struct rtnl_link_stats64 *bslnet_get_stats64(struct net_device *dev,
						   struct rtnl_link_stats64 *stats)
{
	int i;

	for_each_possible_cpu(i) {
		const struct pcpu_dstats *dstats;
		u64 tbytes, tpackets;
		unsigned int start;

		dstats = per_cpu_ptr(dev->dstats, i);
		do {
			start = u64_stats_fetch_begin_irq(&dstats->syncp);
			tbytes = dstats->tx_bytes;
			tpackets = dstats->tx_packets;
		} while (u64_stats_fetch_retry_irq(&dstats->syncp, start));
		stats->tx_bytes += tbytes;
		stats->tx_packets += tpackets;
	}
	return stats;
}

static netdev_tx_t bslnet_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct pcpu_dstats *dstats = this_cpu_ptr(dev->dstats);

	u64_stats_update_begin(&dstats->syncp);
	dstats->tx_packets++;
	dstats->tx_bytes += skb->len;
	u64_stats_update_end(&dstats->syncp);

	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

static int bslnet_dev_init(struct net_device *dev)
{
	dev->dstats = netdev_alloc_pcpu_stats(struct pcpu_dstats);
	if (!dev->dstats)
		return -ENOMEM;

	return 0;
}

static void bslnet_dev_uninit(struct net_device *dev)
{
	free_percpu(dev->dstats);
}

static int bslnet_change_carrier(struct net_device *dev, bool new_carrier)
{
	if (new_carrier)
		netif_carrier_on(dev);
	else
		netif_carrier_off(dev);
	return 0;
}

static const struct net_device_ops bslnet_netdev_ops = {
	.ndo_init		= bslnet_dev_init,
	.ndo_uninit		= bslnet_dev_uninit,
	.ndo_start_xmit		= bslnet_xmit,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_rx_mode	= set_multicast_list,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_get_stats64	= bslnet_get_stats64,
	.ndo_change_carrier	= bslnet_change_carrier,
};

static void bslnet_setup(struct net_device *dev)
{
	ether_setup(dev);

	/* Initialize the device structure. */
	dev->netdev_ops = &bslnet_netdev_ops;
	dev->destructor = free_netdev;

	/* Fill in device structure with ethernet-generic values. */
	dev->tx_queue_len = 0;
	dev->flags |= IFF_NOARP;
	dev->flags &= ~IFF_MULTICAST;
	dev->priv_flags |= IFF_LIVE_ADDR_CHANGE;
	dev->features	|= NETIF_F_SG | NETIF_F_FRAGLIST | NETIF_F_TSO;
	dev->features	|= NETIF_F_HW_CSUM | NETIF_F_HIGHDMA | NETIF_F_LLTX;
	eth_hw_addr_random(dev);
}

static int bslnet_validate(struct nlattr *tb[], struct nlattr *data[])
{
	if (tb[IFLA_ADDRESS]) {
		if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN)
			return -EINVAL;
		if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS])))
			return -EADDRNOTAVAIL;
	}
	return 0;
}

static struct rtnl_link_ops bslnet_link_ops __read_mostly = {
	.kind		= "bslnet",
	.setup		= bslnet_setup,
	.validate	= bslnet_validate,
};

static int __init bslnet_init_one(void)
{
	struct net_device *dev_bslnet;
	int err;

	dev_bslnet = alloc_netdev(0, "bslnet%d", NET_NAME_UNKNOWN, bslnet_setup);
	if (!dev_bslnet)
		return -ENOMEM;

	dev_bslnet->rtnl_link_ops = &bslnet_link_ops;
	err = register_netdevice(dev_bslnet);
	if (err < 0)
		goto err;
	return 0;

err:
	free_netdev(dev_bslnet);
	return err;
}

static int __init bslnet_init_module(void)
{
	int i, err = 0;
	int devcount, portcount;

	rtnl_lock();
	err = __rtnl_link_register(&bslnet_link_ops);
	if (err < 0)
		goto out;

	devcount = bsl_max_devcount();
	portcount = devcount * 2;
	for (i = 0; i < portcount && !err; i++) {
		err = bslnet_init_one();
		cond_resched();
	}
	if (err < 0)
		__rtnl_link_unregister(&bslnet_link_ops);

out:
	rtnl_unlock();

	return err;
}

static void __exit bslnet_cleanup_module(void)
{
	rtnl_link_unregister(&bslnet_link_ops);
}

module_init(bslnet_init_module);
module_exit(bslnet_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_ALIAS_RTNL_LINK("bslnet");
